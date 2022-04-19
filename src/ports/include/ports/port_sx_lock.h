#ifndef SX_LOCK_H
#define SX_LOCK_H
#include "staros/so_init.h"
#ifdef __cplusplus
extern "C" {
#endif

SO_EXPORT void initialize_sx_lock(char** lock);
SO_EXPORT void uninitialize_sx_lock(char* lock);
SO_EXPORT void acquire_lock_exclusive(void* lock);
SO_EXPORT void release_lock_exclusive(void* lock);
SO_EXPORT void acquire_lock_shared(void* lock);
SO_EXPORT void release_lock_shared(void* lock);
#ifdef __cplusplus
}
#endif

#endif/*SX_LOCK_H*/