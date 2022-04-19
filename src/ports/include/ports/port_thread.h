#ifndef __PORT_THREAD_H
#define __PORT_THREAD_H
#include "staros/so_init.h"
#ifdef __cplusplus
extern "C" {
#endif
SO_EXPORT void thread_create(void (*func)(void*),
	void (*init)(void*),
	void* arg,
	void** threadid,
	void* newtd);
SO_EXPORT void thread_destroy(void* threadid);
#ifdef __cplusplus
}
#endif
#endif//__PORT_THREAD_H