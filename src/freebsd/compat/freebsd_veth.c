/*
 * Copyright (c) 2010 Kip Macy. All rights reserved.
 * Copyright (C) 2017-2021 THL A29 Limited, a Tencent company.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Derived in part from libplebnet's pn_veth.c.
 *
 */

#include <sys/ctype.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/module.h>
#include <sys/kernel.h>
#include <sys/proc.h>
#include <sys/kthread.h>
#include <sys/sched.h>
#include <sys/sockio.h>
#include <sys/ck.h>

#include <net/if.h>
#include <net/if_var.h>
#include <net/if_types.h>
#include <net/ethernet.h>
#include <net/if_arp.h>
#include <net/if_tap.h>
#include <net/if_dl.h>
#include <net/route.h>
#include <net/route/route_ctl.h>

#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet6/nd6.h>

#include <machine/atomic.h>

#include "freebsd_veth.h"


struct vos_veth {
    struct ifnet *ifp;
    uint8_t mac[ETHER_ADDR_LEN];
    char host_ifname[IF_NAMESIZE];

    in_addr_t ip;
    in_addr_t netmask;
    in_addr_t broadcast;
    in_addr_t gateway;

    uint8_t nb_vip;
    in_addr_t vip[VIP_MAX_NUM];

#ifdef INET6
    struct in6_addr ip6;
    struct in6_addr gateway6;
    uint8_t prefix_length;

    uint8_t nb_vip6;
    uint8_t vip_prefix_length;
    struct in6_addr vip6[VIP_MAX_NUM];
#endif /* INET6 */
    int (*eth_output)(struct vos_veth* veth, void *m);
    void *host_ctx;
};

static int
vos_veth_transmit(struct ifnet* ifp, struct mbuf* m)
{
    struct vos_veth* sc = (struct vos_veth*)ifp->if_softc;
    (sc->eth_output)(sc, m);
    return 0;
}
static void
vos_veth_init(void* arg)
{
    struct vos_veth* sc = arg;
    struct ifnet* ifp = sc->ifp;

    ifp->if_drv_flags |= IFF_DRV_RUNNING;
    ifp->if_drv_flags &= ~IFF_DRV_OACTIVE;
}
static void
vos_veth_stop(struct vos_veth* sc)
{
    struct ifnet* ifp = sc->ifp;

    ifp->if_drv_flags &= ~(IFF_DRV_RUNNING | IFF_DRV_OACTIVE);
}
static int
vos_veth_ioctl(struct ifnet* ifp, u_long cmd, caddr_t data)
{
    int error = 0;
    struct vos_veth* sc = ifp->if_softc;

    switch (cmd) {
    case SIOCSIFFLAGS:
        if (ifp->if_flags & IFF_UP) {
            vos_veth_init(sc);
        }
        else if (ifp->if_drv_flags & IFF_DRV_RUNNING)
            vos_veth_stop(sc);
        break;
    default:
        error = ether_ioctl(ifp, cmd, data);
        break;
    }

    return (error);
}
static void
vos_veth_start(struct ifnet* ifp)
{
    /* nothing to do */
}
static void
vos_veth_qflush(struct ifnet* ifp)
{

}

void vos_veth_set_hw_features(struct vos_veth* sc, struct port_hw_features* cfg)
{
    struct ifnet *ifp = sc->ifp;
    if (cfg->rx_csum) {
        ifp->if_capabilities |= IFCAP_RXCSUM;
    }
    if (cfg->tx_csum_ip) {
        ifp->if_capabilities |= IFCAP_TXCSUM;
        ifp->if_hwassist |= CSUM_IP;
    }
    if (cfg->tx_csum_l4) {
        ifp->if_hwassist |= CSUM_DELAY_DATA;
    }
    if (cfg->tx_tso) {
        ifp->if_capabilities |= IFCAP_TSO;
        ifp->if_hwassist |= CSUM_TSO;
    }

    ifp->if_capenable = ifp->if_capabilities;
}
struct vos_veth*
create_vos_veth(const in_addr_t* ipaddr,
    const in_addr_t* netmask,
    const in_addr_t* broadcast,
    const in_addr_t* gw,
    char macaddr[VOS_MAC_LEN],
    char name[VOS_IF_NAMESIZE],
    eth_output outputfun, void* cookie)
{
    struct vos_veth*sc = NULL;
    int error;
    struct ifnet* ifp;

    sc = malloc(sizeof(struct vos_veth), M_DEVBUF, M_WAITOK);
    if (NULL == sc) {
        printf("ff_veth_softc allocation failed\n");
        goto fail;
    }
    memset(sc, 0, sizeof(struct vos_veth));
    sc->host_ctx = cookie;
    sc->eth_output = outputfun;
    memcpy(sc->mac, macaddr, sizeof(sc->mac));
    memcpy(sc->host_ifname, name, sizeof(sc->mac));
    ifp = sc->ifp = if_alloc(IFT_ETHER);

    ifp->if_init = vos_veth_init;
    ifp->if_softc = sc;

    if_initname(ifp, sc->host_ifname, IF_DUNIT_NONE);
    ifp->if_flags = IFF_BROADCAST | IFF_SIMPLEX | IFF_MULTICAST;
    ifp->if_ioctl = vos_veth_ioctl;
    ifp->if_start = vos_veth_start;
    ifp->if_transmit = vos_veth_transmit;
    ifp->if_qflush = vos_veth_qflush;
    ether_ifattach(ifp, sc->mac);

    if (ipaddr && netmask && broadcast)
    {
        vos_veth_setaddr(sc, ipaddr, netmask, broadcast);
    }
    if (gw)
    {
        vos_veth_set_gateway(sc, gw);
    }
    return sc;

fail:
    if (sc) {
        free(sc, M_DEVBUF);
    }

    return NULL;
}

int
destroy_vos_veth(struct vos_veth* veth)
{
    struct vos_veth*sc = (struct vos_veth*)veth;

    if (sc->ifp)
    {
        if_free(sc->ifp);
    }
    if (sc) {
        free(sc, M_DEVBUF);
    }

    return (0);
}
void* vos_veth_get_cookie(struct vos_veth* pveth)
{
    return pveth->host_ctx;
}
void vos_veth_set_cookie(struct vos_veth* pveth, void* cookie)
{
    pveth->host_ctx = cookie;
}
int vos_veth_input_mbuf(struct vos_veth* eth, void* m)
{
    struct ifnet* ifp = (struct ifnet*)eth->ifp;
    struct mbuf* mb = (struct mbuf*)m;
    int size = mb->m_len;
    mb->m_pkthdr.rcvif = ifp;

    ifp->if_input(ifp, mb);

    return size;
}
int vos_veth_input(struct vos_veth* eth, const char* pkt, size_t pktsize, uint8_t rx_csum)
{
    struct ifnet* ifp = (struct ifnet*)eth->ifp;
    struct mbuf* mb = vos_mbuf_gethdr(pkt, pktsize, NULL, 0, rx_csum,NULL);

    if (mb == NULL) {
        return -1;
    }

    if (ifp == NULL)
    {
        return -1;
    }
    mb->m_pkthdr.rcvif = ifp;

    ifp->if_input(ifp, mb);

    vos_mbuf_free((struct mbuf*)mb);
    return pktsize;
}
int vos_veth_set_addr(const char* if_dname,
    const in_addr_t* paddr,
    const in_addr_t* netmask,
    const in_addr_t* broadcast)
{
    struct in_aliasreq req;
    bzero(&req, sizeof req);
    strcpy(req.ifra_name, if_dname);
    struct sockaddr_in sa;
    bzero(&sa, sizeof(sa));
    sa.sin_len = sizeof(sa);
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = *paddr;
    bcopy(&sa, &req.ifra_addr, sizeof(sa));

    sa.sin_addr.s_addr = *netmask;
    bcopy(&sa, &req.ifra_mask, sizeof(sa));

    sa.sin_addr.s_addr = *broadcast;
    bcopy(&sa, &req.ifra_broadaddr, sizeof(sa));

    struct socket* so = NULL;
    socreate(AF_INET, &so, SOCK_DGRAM, 0, curthread->td_ucred, curthread);
    int ret = ifioctl(so, SIOCAIFADDR, (caddr_t)&req, curthread);

    sofree(so);

    return ret;
}
int
vos_veth_setaddr(struct vos_veth* sc,
    const in_addr_t* paddr,
    const in_addr_t* netmask,
    const in_addr_t* broadcast)
{
    struct in_aliasreq req;
    bzero(&req, sizeof req);
    strcpy(req.ifra_name, sc->ifp->if_dname);
    memcpy(&(sc->ip), paddr, sizeof(in_addr_t));
    memcpy(&(sc->netmask), netmask, sizeof(in_addr_t));
    memcpy(&(sc->broadcast), broadcast, sizeof(in_addr_t));
    struct sockaddr_in sa;
    bzero(&sa, sizeof(sa));
    sa.sin_len = sizeof(sa);
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = sc->ip;
    bcopy(&sa, &req.ifra_addr, sizeof(sa));

    sa.sin_addr.s_addr = sc->netmask;
    bcopy(&sa, &req.ifra_mask, sizeof(sa));

    sa.sin_addr.s_addr = sc->broadcast;
    bcopy(&sa, &req.ifra_broadaddr, sizeof(sa));

    struct socket* so = NULL;
    socreate(AF_INET, &so, SOCK_DGRAM, 0, curthread->td_ucred, curthread);
    int ret = ifioctl(so, SIOCAIFADDR, (caddr_t)&req, curthread);

    sofree(so);

    return ret;
}

int
vos_veth_set_gateway(struct vos_veth* sc, in_addr_t *paddr)
{
    struct rt_addrinfo info;
    struct rib_cmd_info rci;
    memcpy(&(sc->gateway), paddr, sizeof(in_addr_t));
    bzero((caddr_t)&info, sizeof(info));
    info.rti_flags = RTF_GATEWAY;

    struct sockaddr_in gw;
    bzero(&gw, sizeof(gw));
    gw.sin_len = sizeof(gw);
    gw.sin_family = AF_INET;
    gw.sin_addr.s_addr = sc->gateway;
    info.rti_info[RTAX_GATEWAY] = (struct sockaddr*)&gw;

    struct sockaddr_in dst;
    bzero(&dst, sizeof(dst));
    dst.sin_len = sizeof(dst);
    dst.sin_family = AF_INET;
    dst.sin_addr.s_addr = 0;
    info.rti_info[RTAX_DST] = (struct sockaddr*)&dst;

    struct sockaddr_in nm;
    bzero(&nm, sizeof(nm));
    nm.sin_len = sizeof(nm);
    nm.sin_family = AF_INET;
    nm.sin_addr.s_addr = 0;
    info.rti_info[RTAX_NETMASK] = (struct sockaddr*)&nm;

    return rib_action(RT_DEFAULT_FIB, RTM_ADD, &info, &rci);
}

int vos_veth_set_vaddr(const char* if_dname,
    uint8_t nb_vip,
    in_addr_t vip[VIP_MAX_NUM])
{
    struct in_aliasreq req;
    bzero(&req, sizeof req);

    strlcpy(req.ifra_name, if_dname, IFNAMSIZ);

    struct sockaddr_in sa;
    bzero(&sa, sizeof(sa));
    sa.sin_len = sizeof(sa);
    sa.sin_family = AF_INET;

    int i, ret=-1;
    struct socket* so = NULL;
    socreate(AF_INET, &so, SOCK_DGRAM, 0, curthread->td_ucred, curthread);

    for (i = 0; i < nb_vip; ++i) {
        sa.sin_addr.s_addr = vip[i];
        bcopy(&sa, &req.ifra_addr, sizeof(sa));

        // Only support '255.255.255.255' netmask now
        sa.sin_addr.s_addr = 0xFFFFFFFF;
        bcopy(&sa, &req.ifra_mask, sizeof(sa));

        // Only support 'x.x.x.255' broadaddr now
        sa.sin_addr.s_addr = vip[i] | 0xFF000000;
        bcopy(&sa, &req.ifra_broadaddr, sizeof(sa));

        ret = ifioctl(so, SIOCAIFADDR, (caddr_t)&req, curthread);
        if (ret < 0) {
            printf("ff_veth_setvaddr ifioctl SIOCAIFADDR error\n");
            goto done;
        }
    }

done:
    sofree(so);

    return ret;
}
int
vos_veth_setvaddr(struct vos_veth* sc, const char* if_dname,
    uint8_t nb_vip,
    in_addr_t vip[VIP_MAX_NUM])
{
    struct in_aliasreq req;
    bzero(&req, sizeof req);

    if (if_dname) {
        strlcpy(req.ifra_name, if_dname, IFNAMSIZ);
    }
    else {
        strlcpy(req.ifra_name, sc->ifp->if_dname, IFNAMSIZ);
    }

    sc->nb_vip = nb_vip;
    memcpy((char *)sc->vip, (char *)vip, sizeof(in_addr_t) * VIP_MAX_NUM);

    struct sockaddr_in sa;
    bzero(&sa, sizeof(sa));
    sa.sin_len = sizeof(sa);
    sa.sin_family = AF_INET;

    int i, ret=-1;
    struct socket* so = NULL;
    socreate(AF_INET, &so, SOCK_DGRAM, 0, curthread->td_ucred, curthread);

    for (i = 0; i < sc->nb_vip; ++i) {
        sa.sin_addr.s_addr = sc->vip[i];
        bcopy(&sa, &req.ifra_addr, sizeof(sa));

        // Only support '255.255.255.255' netmask now
        sa.sin_addr.s_addr = 0xFFFFFFFF;
        bcopy(&sa, &req.ifra_mask, sizeof(sa));

        // Only support 'x.x.x.255' broadaddr now
        sa.sin_addr.s_addr = sc->vip[i] | 0xFF000000;
        bcopy(&sa, &req.ifra_broadaddr, sizeof(sa));

        ret = ifioctl(so, SIOCAIFADDR, (caddr_t)&req, curthread);
        if (ret < 0) {
            printf("ff_veth_setvaddr ifioctl SIOCAIFADDR error\n");
            goto done;
        }
    }

done:
    sofree(so);

    return ret;
}

int
vos_veth_setaddr6(struct vos_veth* sc, struct in6_addr *ip6)
{
#ifdef INET6
    struct in6_aliasreq ifr6;
    bzero(&ifr6, sizeof(ifr6));

    memcpy(&(sc->ip6), ip6, sizeof(struct in6_addr));

    strcpy(ifr6.ifra_name, sc->ifp->if_dname);

    ifr6.ifra_addr.sin6_len = sizeof ifr6.ifra_addr;
    ifr6.ifra_addr.sin6_family = AF_INET6;
    ifr6.ifra_addr.sin6_addr = sc->ip6;

    ifr6.ifra_prefixmask.sin6_len = sizeof ifr6.ifra_prefixmask;
    memset(&ifr6.ifra_prefixmask.sin6_addr, 0xff, sc->prefix_length / 8);
    uint8_t mask_size_mod = sc->prefix_length % 8;
    if (mask_size_mod)
    {
        ifr6.ifra_prefixmask.sin6_addr.__u6_addr.__u6_addr8[sc->prefix_length / 8] = \
            ((1 << mask_size_mod) - 1) << (8 - mask_size_mod);
    }

    ifr6.ifra_lifetime.ia6t_pltime = ifr6.ifra_lifetime.ia6t_vltime = ND6_INFINITE_LIFETIME;

    struct socket* so = NULL;
    socreate(AF_INET6, &so, SOCK_DGRAM, 0, curthread->td_ucred, curthread);
    int ret = ifioctl(so, SIOCAIFADDR_IN6, (caddr_t)&ifr6, curthread);

    sofree(so);

    return ret;
#else
    return -1;
#endif
}

int
vos_veth_set_gateway6(struct vos_veth* sc, struct in6_addr* ip6gateway)
{
#ifdef INET6
    struct sockaddr_in6 gw;
    struct rt_addrinfo info;
    struct rib_cmd_info rci;

    bzero((caddr_t)&info, sizeof(info));
    info.rti_flags = RTF_GATEWAY;

    memcpy(&(sc->gateway6), ip6gateway, sizeof(struct in6_addr));

    bzero(&gw, sizeof(gw));



    gw.sin6_len = sizeof(struct sockaddr_in6);
    gw.sin6_family = AF_INET6;

    gw.sin6_addr = sc->gateway6;

    info.rti_info[RTAX_GATEWAY] = (struct sockaddr*)&gw;

    return rib_action(RT_DEFAULT_FIB, RTM_ADD, &info, &rci);
#else
    return -1;
#endif
}
int vos_veth_set_macaddr(struct vos_veth* sc, char macaddr[VOS_MAC_LEN])
{
    memcpy(sc->mac, macaddr, VOS_MAC_LEN);
    return 0;
}
int vos_veth_get_macaddr(struct vos_veth* sc, char macaddr[VOS_MAC_LEN])
{
    memcpy(macaddr, sc->mac, VOS_MAC_LEN);
    return 0;
}
int
vos_veth_setvaddr6(struct vos_veth* sc, const char* if_dname,
    uint8_t nb_vip6,
    struct in6_addr vip6[VIP_MAX_NUM])
{
#ifdef INET6
    struct in6_aliasreq ifr6;
    bzero(&ifr6, sizeof(ifr6));

    sc->nb_vip6 = nb_vip6;
    memcpy((char*)sc->vip6, (char*)vip6, sizeof(struct in6_addr) * VIP_MAX_NUM);


    if (if_dname) {
        strlcpy(ifr6.ifra_name, if_dname, IFNAMSIZ);
    }
    else {
        strlcpy(ifr6.ifra_name, sc->ifp->if_dname, IFNAMSIZ);
    }

    ifr6.ifra_addr.sin6_len = sizeof ifr6.ifra_addr;
    ifr6.ifra_addr.sin6_family = AF_INET6;

    ifr6.ifra_prefixmask.sin6_len = sizeof ifr6.ifra_prefixmask;
    memset(&ifr6.ifra_prefixmask.sin6_addr, 0xff, sc->prefix_length / 8);
    uint8_t mask_size_mod = sc->prefix_length % 8;
    if (mask_size_mod)
    {
        ifr6.ifra_prefixmask.sin6_addr.__u6_addr.__u6_addr8[sc->prefix_length / 8] = \
            ((1 << mask_size_mod) - 1) << (8 - mask_size_mod);
    }

    ifr6.ifra_lifetime.ia6t_pltime = ifr6.ifra_lifetime.ia6t_vltime = ND6_INFINITE_LIFETIME;

    struct socket* so = NULL;
    socreate(AF_INET6, &so, SOCK_DGRAM, 0, curthread->td_ucred, curthread);

    int i, ret;
    for (i = 0; i < sc->nb_vip6; ++i) {
        ifr6.ifra_addr.sin6_addr = sc->vip6[i];

        ret = ifioctl(so, SIOCAIFADDR_IN6, (caddr_t)&ifr6, curthread);
        if (ret < 0) {
            printf("ff_veth_setvaddr6 ifioctl SIOCAIFADDR error\n");
            goto done;
        }
    }

done:
    sofree(so);

    return ret;
#else
    return -1;
#endif
}