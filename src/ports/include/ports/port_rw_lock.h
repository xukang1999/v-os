#ifndef __PORT_RW_LOCK_H
#define __PORT_RW_LOCK_H
#include "staros/so_init.h"
#ifdef __cplusplus
extern "C" {
#endif
SO_EXPORT void
port_rw_init(void **mutex);

SO_EXPORT void
port_rw_wlock(void* mutex);
SO_EXPORT void
port_rw_wunlock(void* mutex);

SO_EXPORT void
port_rw_rlock(void* mutex);
SO_EXPORT void
port_rw_runlock(void* mutex);

SO_EXPORT void
port_rw_destroy(void* mutex);
#ifdef __cplusplus
}
#endif
#endif//__PORT_MUTEX_H