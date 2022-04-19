/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2019 Intel Corporation
 */

#ifndef _PTHREAD_H_
#define _PTHREAD_H_

#include <stdint.h>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

 /* Must come first. */
#include <windows.h>

#include <basetsd.h>
#include <psapi.h>
#include <setupapi.h>
#include <winioctl.h>

/**
 * This file is required to support the common code in eal_common_proc.c,
 * eal_common_thread.c and common\include\rte_per_lcore.h as Microsoft libc
 * does not contain pthread.h. This may be removed in future releases.
 */
#ifdef __cplusplus
extern "C" {
#endif

#define PTHREAD_BARRIER_SERIAL_THREAD TRUE

/* defining pthread_t type on Windows since there is no in Microsoft libc*/
typedef uintptr_t pthread_t;

/* defining pthread_attr_t type on Windows since there is no in Microsoft libc*/
typedef void *pthread_attr_t;

typedef void *pthread_mutexattr_t;

typedef CRITICAL_SECTION pthread_mutex_t;

typedef SYNCHRONIZATION_BARRIER pthread_barrier_t;

#define pthread_barrier_init(barrier, attr, count) \
	InitializeSynchronizationBarrier(barrier, count, -1)
#define pthread_barrier_wait(barrier) EnterSynchronizationBarrier(barrier, \
	SYNCHRONIZATION_BARRIER_FLAGS_BLOCK_ONLY)
#define pthread_barrier_destroy(barrier) \
	DeleteSynchronizationBarrier(barrier)
#define pthread_cancel(thread) TerminateThread((HANDLE) thread, 0)

/* pthread function overrides */
#define pthread_self() \
	((pthread_t)GetCurrentThreadId())


static inline int
pthread_equal(pthread_t t1, pthread_t t2)
{
	return t1 == t2;
}
static inline void pthread_exit(void* retval)
{

}

static inline int
pthread_create(void *threadid, const void *threadattr, void *threadfunc,
		void *args)
{

	HANDLE hThread;
	hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadfunc,
		args, 0, (LPDWORD)threadid);
	if (hThread) {
		SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
		SetThreadPriority(hThread, THREAD_PRIORITY_TIME_CRITICAL);
	}
	return ((hThread != NULL) ? 0 : E_FAIL);
}

static inline int
pthread_join(pthread_t thread,void **value_ptr)
{
	return 0;
}

static inline int
pthread_mutex_init(pthread_mutex_t *mutex,pthread_mutexattr_t *attr)
{
	InitializeCriticalSection(mutex);
	return 0;
}

static inline int
pthread_mutex_lock(pthread_mutex_t *mutex)
{
	EnterCriticalSection(mutex);
	return 0;
}

static inline int
pthread_mutex_unlock(pthread_mutex_t *mutex)
{
	LeaveCriticalSection(mutex);
	return 0;
}

static inline int
pthread_mutex_destroy(pthread_mutex_t *mutex)
{
	DeleteCriticalSection(mutex);
	return 0;
}

#ifdef __cplusplus
}
#endif

#endif /* _PTHREAD_H_ */
