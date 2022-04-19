#ifndef _SO_KTHREAD_H
#define _SO_KTHREAD_H
#include "staros/so_init.h"

#ifdef __cplusplus
extern "C" {
#endif

	SO_EXPORT int kthread_create(void (*func)(void*), void* arg, const char* name);

#ifdef __cplusplus
}
#endif
#endif /* _SO_KTHREAD_H*/
