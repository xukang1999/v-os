#ifndef _PORT_TIME_H
#define _PORT_TIME_H
#include "staros/so_init.h"

#ifdef __cplusplus
extern "C" {
#endif

SO_EXPORT void port_mssleep(int msecs);
SO_EXPORT void port_ussleep(uint64_t usecs);
SO_EXPORT void port_second_sleep(uint32_t secs);
#ifdef __cplusplus
}
#endif
#endif//_PORT_TIME_H