#ifndef _SO_MBUF_H
#define _SO_MBUF_H
#include "staros/so_init.h"
#ifdef __cplusplus
extern "C" {
#endif

struct vos_tx_offload {
	uint8_t ip_csum;
	uint8_t tcp_csum;
	uint8_t udp_csum;
	uint8_t sctp_csum;
	uint16_t tso_seg_size;
};

SO_EXPORT void *vos_mbuf_gethdr(void* pkt, uint16_t total, void* data,
	uint16_t len, uint8_t rx_csum, void (*ext_free)(void* m));
SO_EXPORT void *vos_mbuf_get(void* p, void* m, void* data, uint16_t len, void (*ext_free)(void* m));
SO_EXPORT void vos_mbuf_free(void *m);
SO_EXPORT int vos_mbuf_copydata(void *m, void *data, int off, int len);
SO_EXPORT int vos_next_mbuf(void **mbuf_bsd, void **data, unsigned *len);
SO_EXPORT void* vos_mbuf_mtod(void* bsd_mbuf);
SO_EXPORT void* vos_rte_frm_extcl(void* mbuf, void (*ext_free)(void* m));
SO_EXPORT void vos_mbuf_tx_offload(void *m, struct vos_tx_offload*offload);
SO_EXPORT void vos_mbuf_set_vlan_info(void *hdr, uint16_t vlan_tci);

SO_EXPORT int32_t vos_mbuf_getpktlen(void* mbuf);

#ifdef __cplusplus
}
#endif

#endif