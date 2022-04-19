#ifndef _SO_VETH_H
#define _SO_VETH_H
#include "staros/so_init.h"

#ifdef __cplusplus
extern "C" {
#endif

#define VIP_MAX_NUM 64
#define VOS_MAC_LEN 6
#define VOS_IF_NAMESIZE 16
struct vos_veth;

struct port_hw_features {
	uint8_t rx_csum;
	uint8_t rx_lro;
	uint8_t tx_csum_ip;
	uint8_t tx_csum_l4;
	uint8_t tx_tso;
};

typedef int (*eth_output)(struct vos_veth* veth, void *m);

SO_EXPORT struct vos_veth* create_vos_veth(const in_addr_t* ipaddr,
	const in_addr_t* netmask,
	const in_addr_t* broadcast,
	const in_addr_t* gw,
	char macaddr[VOS_MAC_LEN],
	char name[VOS_IF_NAMESIZE],
	eth_output outputfun, void* cookie);

SO_EXPORT void vos_veth_set_hw_features(struct vos_veth* sc, struct port_hw_features* cfg);
SO_EXPORT int destroy_vos_veth(struct vos_veth*veth);

SO_EXPORT int vos_veth_input_mbuf(struct vos_veth* eth, void* m);
SO_EXPORT int vos_veth_input(struct vos_veth* eth, const char* pkt, size_t pktsize, uint8_t rx_csum);

SO_EXPORT void* vos_veth_get_cookie(struct vos_veth* pveth);
SO_EXPORT void vos_veth_set_cookie(struct vos_veth* pveth, void *cookie);
SO_EXPORT int vos_veth_setaddr(struct vos_veth* sc,
	const in_addr_t* paddr, 
	const in_addr_t* netmask,
	const in_addr_t* broadcast);

SO_EXPORT int vos_veth_set_addr(const char* if_dname,
	const in_addr_t* paddr,
	const in_addr_t* netmask,
	const in_addr_t* broadcast);

SO_EXPORT int vos_veth_set_gateway(struct vos_veth* sc, in_addr_t* paddr);

SO_EXPORT int vos_veth_setvaddr(struct vos_veth* sc, const char* if_dname,
	uint8_t nb_vip,
	in_addr_t vip[VIP_MAX_NUM]);

SO_EXPORT int vos_veth_set_vaddr( const char* if_dname,
	uint8_t nb_vip,
	in_addr_t vip[VIP_MAX_NUM]);

SO_EXPORT int vos_veth_setaddr6(struct vos_veth* sc, struct in6_addr* ip6);
SO_EXPORT int vos_veth_set_gateway6(struct vos_veth* sc, struct in6_addr* ip6gateway);
SO_EXPORT int vos_veth_setvaddr6(struct vos_veth* sc, const char* if_dname,
	uint8_t nb_vip6,
	struct in6_addr vip6[VIP_MAX_NUM]);

SO_EXPORT int vos_veth_set_macaddr(struct vos_veth* sc, char macaddr[VOS_MAC_LEN]);
SO_EXPORT int vos_veth_get_macaddr(struct vos_veth* sc, char macaddr[VOS_MAC_LEN]);
#ifdef __cplusplus
}
#endif
#endif /* _FREEBSD_VETH_H*/
