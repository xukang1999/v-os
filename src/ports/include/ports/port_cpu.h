#ifndef PORT_CPU_H
#define PORT_CPU_H
#include "staros/so_init.h"
#ifdef __cplusplus
extern "C" {
#endif
SO_EXPORT void port_yield(void);
SO_EXPORT uint64_t port_cpud_ticks(void);
SO_EXPORT uint64_t port_rdtsc();
SO_EXPORT uint64_t port_rdtscp();
SO_EXPORT uint64_t port_rdtscp_aux(uint32_t* aux);
SO_EXPORT uint32_t port_rdtsc32(void);
SO_EXPORT uint32_t port_rdtscp32(void);
SO_EXPORT void port_sfence(void);
SO_EXPORT void port_lfence(void);
SO_EXPORT void port_mfence(void);
SO_EXPORT int port_breakpoint(void);

#ifdef __cplusplus
}
#endif

#endif