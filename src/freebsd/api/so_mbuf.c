#include <sys/cdefs.h>
#include <sys/param.h>
#include <sys/pcpu.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/lock.h>
#include <sys/sx.h>
#include <sys/vmmeter.h>
#include <sys/cpuset.h>
#include <sys/sysctl.h>
#include <sys/filedesc.h>
#include <vm/vm.h>
#include <vm/vm_param.h>
#include <vm/vm_domainset.h>
#include <vm/vm_object.h>
#include <vm/vm_page.h>
#include <vm/vm_pageout.h>
#include <vm/vm_phys.h>
#include <vm/vm_pagequeue.h>
#include <vm/vm_map.h>
#include <vm/vm_kern.h>
#include <vm/vm_extern.h>
#include <vm/vm_dumpset.h>
#include <vm/uma.h>
#include <vm/uma_int.h>
#include <vm/uma_dbg.h>
#include <sys/_stdarg.h>
#include <sys/nlist_aout.h>
#include <sys/link_aout.h>
#include "staros/staros.h"


void
vos_mbuf_free(void* m)
{
    m_freem((struct mbuf*)m);
}

void*
vos_mbuf_gethdr(void* pkt, uint16_t total, void* data,
    uint16_t len, uint8_t rx_csum, void (*ext_free)(void* m))
{
    struct mbuf* m = m_gethdr(M_NOWAIT, MT_DATA);
    if (m == NULL) {
        return NULL;
    }

    if (m_pkthdr_init(m, M_NOWAIT) != 0) {
        return NULL;
    }

    if ((ext_free != NULL) && (data != NULL))
    {
        m_extadd(m, data, len, ext_free, pkt, NULL, 0, EXT_DISPOSABLE);
    }
    else
    {
        memcpy(mtod(m, caddr_t), pkt, total);
    }
    m->m_pkthdr.len = total;
    m->m_len = m->m_pkthdr.len;
    m->m_next = NULL;
    m->m_nextpkt = NULL;

    if (rx_csum) {
        m->m_pkthdr.csum_flags = CSUM_IP_CHECKED | CSUM_IP_VALID |
            CSUM_DATA_VALID | CSUM_PSEUDO_HDR;
        m->m_pkthdr.csum_data = 0xffff;
    }


    return (void*)m;
}

void*
vos_mbuf_get(void* p, void* m, void* data, uint16_t len, void (* ext_free)(void* m))
{
    struct mbuf* prev = (struct mbuf*)p;
    struct mbuf* mb = m_get(M_NOWAIT, MT_DATA);

    if (mb == NULL) {
        return NULL;
    }

    m_extadd(mb, data, len, ext_free, m, NULL, 0, EXT_DISPOSABLE);

    mb->m_next = NULL;
    mb->m_nextpkt = NULL;
    mb->m_len = len;

    if (prev != NULL) {
        prev->m_next = mb;
    }

    return (void*)mb;
}

int
vos_mbuf_copydata(void* m, void* data, int off, int len)
{
    int ret;
    struct mbuf* mb = (struct mbuf*)m;

    if (off + len > mb->m_pkthdr.len) {
        return -1;
    }

    m_copydata(mb, off, len, data);

    return 0;
}

void
vos_mbuf_tx_offload(void* m, struct vos_tx_offload* offload)
{
    struct mbuf* mb = (struct mbuf*)m;
    if (mb->m_pkthdr.csum_flags & CSUM_IP) {
        offload->ip_csum = 1;
    }

    if (mb->m_pkthdr.csum_flags & CSUM_TCP) {
        offload->tcp_csum = 1;
    }

    if (mb->m_pkthdr.csum_flags & CSUM_UDP) {
        offload->udp_csum = 1;
    }

    if (mb->m_pkthdr.csum_flags & CSUM_SCTP) {
        offload->sctp_csum = 1;
    }

    if (mb->m_pkthdr.csum_flags & CSUM_TSO) {
        offload->tso_seg_size = mb->m_pkthdr.tso_segsz;
    }
}


int vos_next_mbuf(void** mbuf_bsd, void** data, unsigned* len)
{
    struct mbuf* mb = *(struct mbuf**)mbuf_bsd;

    *len = mb->m_len;
    *data = mb->m_data;

    if (mb->m_next)
        *mbuf_bsd = mb->m_next;
    else
        *mbuf_bsd = NULL;
    return 0;
}

void* vos_mbuf_mtod(void* bsd_mbuf)
{
    if (!bsd_mbuf)
        return NULL;
    return (void*)((struct mbuf*)bsd_mbuf)->m_data;
}

// get source rte_mbuf from ext cluster, which carry rte_mbuf while recving pkt, such as arp.
void* vos_rte_frm_extcl(void* mbuf, void (*ext_free)(void* m))
{
    struct mbuf* bsd_mbuf = mbuf;

    if ((bsd_mbuf->m_flags & M_EXT) &&
        bsd_mbuf->m_ext.ext_type == EXT_DISPOSABLE && bsd_mbuf->m_ext.ext_free == ext_free) {
        return bsd_mbuf->m_ext.ext_arg1;
    }
    else
        return NULL;
}

void
vos_mbuf_set_vlan_info(void* hdr, uint16_t vlan_tci) {
    struct mbuf* m = (struct mbuf*)hdr;
    m->m_pkthdr.ether_vtag = vlan_tci;
    m->m_flags |= M_VLANTAG;
    return;
}
int32_t vos_mbuf_getpktlen(void* mbuf)
{
    struct mbuf* m = (struct mbuf*)mbuf;
    return m->m_pkthdr.len;
}