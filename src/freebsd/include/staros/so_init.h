/**********************************************************************************
 * Copyright (C) 2013-2015, Nanjing WFNEX Technology Co., Ltd. All rights reserved.
***********************************************************************************/
#ifndef __SO_INIT_H__
#define __SO_INIT_H__
#include "staros/so_export.h"
#ifndef _KERNEL
	#include <stdlib.h>
	#include <stdint.h>
	typedef unsigned int in_addr_t;
#ifdef _WIN32
#define SO_CDECL __cdecl
#define SO_STDCALL __stdcall
#ifndef NOMINMAX
#define NOMINMAX
#endif // NOMINMAX

	// supports Windows NT 4.0 and later, not support Windows 95.
	// mainly for using winsock2 functions
#ifndef _WIN32_WINNT
  //#define _WIN32_WINNT 0x0400
#define _WIN32_WINNT 0x0600
#endif // _WIN32_WINNT
#define _WINSOCKAPI_    // stops windows.h including winsock.h
#define _WINSOCKAPI_    // stops windows.h including winsock.h

#include <windows.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <mmsystem.h>
#include <sys/types.h>
/* as in <windows.h> */
	typedef long long ssize_t;
	typedef	unsigned int	nfds_t;

#else
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <poll.h>
#include <pthread.h>
#include <sys/mman.h>
	#include <unistd.h>
	typedef	unsigned char	u_char;
	typedef	unsigned short	u_short;
	typedef	unsigned int	u_int;
#define SO_CDECL 
#define SO_STDCALL 
#endif
#else
	#include <sys/ctype.h>
	#include <sys/types.h>
	#include <sys/param.h>
	#include <sys/time.h>
	#include <sys/socket.h>
	#include <sys/socketvar.h>
	#include <sys/module.h>
	#include <sys/kernel.h>
	#include <sys/proc.h>
	#include <sys/kthread.h>
	#include <sys/sched.h>
	#include <sys/sockio.h>
	#include <sys/ck.h>
	#include <sys/poll.h>
	#include <net/if.h>
	#include <net/if_var.h>
	#include <net/if_types.h>
	#include <net/ethernet.h>
	#include <net/if_arp.h>
	#include <net/if_tap.h>
	#include <net/if_dl.h>
	#include <net/route.h>
	#include <net/route/route_ctl.h>

	#include <netinet/in.h>
	#include <netinet/in_var.h>
	#include <netinet6/nd6.h>
#endif
#ifdef __cplusplus
extern "C" {
#endif


#define VOS_MAP_FILE        0
#define VOS_MAP_TYPE        0x0F
#define VOS_MAP_FIXED       0x10
#define VOS_MAP_ANONYMOUS   0x20

#define VOS_MAP_PROT_NONE     0x00
#define VOS_MAP_PROT_READ     0x01
#define VOS_MAP_PROT_WRITE    0x02
#define VOS_MAP_PROT_EXEC     0x04

#define VOS_MAP_SHARED    0x0001
#define VOS_MAP_PRIVATE   0x0002
#define VOS_MAP_ANON      VOS_MAP_ANONYMOUS
#define VOS_MAP_NOCORE    0x00020000

#define VOS_MAP_FAILED    ((void *)-1)


typedef struct so_mem_hooks
{
	/* malloc/free are CDECL on Windows regardless of the default calling convention of the compiler, so ensure the hooks allow passing those functions directly. */
	void* (* malloc_fn)(size_t sz);
	void (* free_fn)(void* ptr);
	void* (* reallocat_fn)(void* pointer, size_t size);
	void* (*calloc)(size_t num, size_t size);
} so_mem_hooks;

typedef struct so_mmap_hooks
{
	void* (* mmap)(void* addr, size_t len, int prot, int flags, int fildes, off_t off);
	int (* munmap)(void* addr, size_t len);
} so_mmap_hooks;

typedef struct so_atomic_hooks
{
	void (*atomic_set_int)(u_int* P, u_int V);
	void (*atomic_clear_int)(u_int* P, u_int V);
	void (*atomic_add_int)(u_int* P, u_int V);
	void (*atomic_subtract_int)(u_int* P, u_int V);
	void (*atomic_add_long)(unsigned long long* P, unsigned long long V);
	void (*atomic_subtract_long)(unsigned long long* P, unsigned long long V);
	char (*atomic_fetchadd_char)(volatile char* p, char v);
	int (*atomic_fetchadd_int)(volatile int* p, int v);
	u_int (*atomic_fetchadd_uint)(volatile u_int* p, u_int v);
	unsigned long long (*atomic_fetchadd_long)(volatile unsigned long long* p, unsigned long long v);
	uint8_t (*atomic_fetchadd_8)(volatile uint8_t* p, uint8_t v);
	uint16_t (*atomic_fetchadd_16)(volatile uint16_t* p, uint16_t v);
	uint32_t (*atomic_fetchadd_32_pfn)(volatile uint32_t* p, uint32_t v);
	uint64_t (*atomic_fetchadd_64_pfn)(volatile uint64_t* p, uint64_t v);
	void* (*atomic_fetchadd_ptr)(volatile void* p, void* v);

	char (*atomic_fetchsub_char)(volatile char* p, char v);
	int (*atomic_fetchsub_int)(volatile int* p, int v);
	u_int(*atomic_fetchsub_uint)(volatile u_int* p, u_int v);
	unsigned long long(*atomic_fetchsub_long)(volatile unsigned long long* p, unsigned long long v);
	uint8_t(*atomic_fetchsub_8)(volatile uint8_t* p, uint8_t v);
	uint16_t(*atomic_fetchsub_16)(volatile uint16_t* p, uint16_t v);
	uint32_t(*atomic_fetchsub_32)(volatile uint32_t* p, uint32_t v);
	uint64_t(*atomic_fetchsub_64)(volatile uint64_t* p, uint64_t v);
	void* (*atomic_fetchsub_ptr)(volatile void* p, void* v);

	char (*atomic_fetchor_char)(volatile char* p, char v);
	int (*atomic_fetchor_int)(volatile int* p, int v);
	u_int(*atomic_fetchor_uint)(volatile u_int* p, u_int v);
	unsigned long long(*atomic_fetchor_long)(volatile unsigned long long* p, unsigned long long v);
	uint8_t(*atomic_fetchor_8)(volatile uint8_t* p, uint8_t v);
	uint16_t(*atomic_fetchor_16)(volatile uint16_t* p, uint16_t v);
	uint32_t(*atomic_fetchor_32)(volatile uint32_t* p, uint32_t v);
	uint64_t(*atomic_fetchor_64)(volatile uint64_t* p, uint64_t v);
	void* (*atomic_fetchor_ptr)(volatile void* p, void* v);

	char (*atomic_fetchand_char)(volatile char* p, char v);
	int (*atomic_fetchand_int)(volatile int* p, int v);
	u_int(*atomic_fetchand_uint)(volatile u_int* p, u_int v);
	unsigned long long(*atomic_fetchand_long)(volatile unsigned long long* p, unsigned long long v);
	uint8_t(*atomic_fetchand_8)(volatile uint8_t* p, uint8_t v);
	uint16_t(*atomic_fetchand_16)(volatile uint16_t* p, uint16_t v);
	uint32_t(*atomic_fetchand_32)(volatile uint32_t* p, uint32_t v);
	uint64_t(*atomic_fetchand_64)(volatile uint64_t* p, uint64_t v);
	void* (*atomic_fetchand_ptr)(volatile void* p, void* v);

	char (*atomic_fetchxor_char)(volatile char* p, char v);
	int (*atomic_fetchxor_int)(volatile int* p, int v);
	u_int(*atomic_fetchxor_uint)(volatile u_int* p, u_int v);
	unsigned long long(*atomic_fetchxor_long)(volatile unsigned long long* p, unsigned long long v);
	uint8_t(*atomic_fetchxor_8)(volatile uint8_t* p, uint8_t v);
	uint16_t(*atomic_fetchxor_16)(volatile uint16_t* p, uint16_t v);
	uint32_t(*atomic_fetchxor_32)(volatile uint32_t* p, uint32_t v);
	uint64_t(*atomic_fetchxor_64)(volatile uint64_t* p, uint64_t v);
	void* (*atomic_fetchxor_ptr)(volatile void* p, void *v);

	int (*sync_bool_compare_and_swap_char)(char* ptr, char compare, char set);
	char (*sync_val_compare_and_swap_char)(char* ptr, char compare, char set);
	int (*sync_bool_compare_and_swap_int)(int* ptr, int compare, int set);
	char (*sync_val_compare_and_swap_int)(int* ptr, int compare, int set);
	int (*sync_bool_compare_and_swap_uint)(unsigned int* ptr, unsigned int compare, unsigned int set);
	unsigned int (*sync_val_compare_and_swap_uint)(unsigned int* ptr, unsigned int compare, unsigned int set);
	int (*sync_bool_compare_and_swap_8)(uint8_t* ptr, uint8_t compare, uint8_t set);
	uint8_t (*sync_val_compare_and_swap_8)(uint8_t* ptr, uint8_t compare, uint8_t set);
	int (*sync_bool_compare_and_swap_16)(uint16_t* ptr, uint16_t compare, uint16_t set);
	uint16_t (*sync_val_compare_and_swap_16)(uint16_t* ptr, uint16_t compare, uint16_t set);
	int (*sync_bool_compare_and_swap_32)(uint32_t* ptr, uint32_t compare, uint32_t set);
	uint32_t (*sync_val_compare_and_swap_32)(uint32_t* ptr, uint32_t compare, uint32_t set);
	int (*sync_bool_compare_and_swap_64)(uint64_t* ptr, uint64_t compare, uint64_t set);
	uint64_t (*sync_val_compare_and_swap_64)(uint64_t* ptr, uint64_t compare, uint64_t set);
	int (*sync_bool_compare_and_swap_ptr)(void* ptr, void* compare, void* set);
	void *(*sync_val_compare_and_swap_ptr)(void* ptr, void* compare, void* set);

	int	(*atomic_cmpset_char)(volatile u_char* object, u_char expect, u_char desired);
	int	(*atomic_cmpset_short)(volatile u_short* dst, u_short expect, u_short desired);
	int	(*atomic_cmpset_int)(volatile u_int* dst, u_int expect, u_int desired);
	int	(*atomic_cmpset_long)(volatile unsigned long long* dst, unsigned long long expect, unsigned long long desired);

	int	(*atomic_fcmpset_char)(volatile u_char* object, u_char* expected, u_char desired);
	int	(*atomic_fcmpset_short)(volatile u_short* object, u_short* expected, u_short desired);
	int	(*atomic_fcmpset_int)(volatile u_int* object, u_int* expected, u_int desired);
	int	(*atomic_fcmpset_long)(volatile unsigned long long* object, unsigned long long* expected, unsigned long long desired);


	void (*atomic_thread_fence_acq)(void);
	void (*atomic_thread_fence_rel)(void);
	void (*atomic_thread_fence_acq_rel)(void);
	void (*atomic_thread_fence_seq_cst)(void);
	void (*atomic_add_barr_int)(u_int* P, u_int V);
	void (*atomic_store_rel_int)(volatile u_int* p, u_int v);
	void (*atomic_store_rel_long)(volatile unsigned long long* p, unsigned long long v);
	unsigned long long (*atomic_load_acq_long)(volatile unsigned long long* p);
	u_int (*atomic_load_acq_int)(volatile u_int* p);
	void (*atomic_subtract_barr_int)(volatile u_int* p, u_int val);
	u_int (*atomic_swap_int)(volatile u_int* p, u_int v);
	unsigned long long (*atomic_swap_long)(volatile unsigned long long* p, unsigned long long v);
	void (*atomic_set_long)(volatile unsigned long long* p, unsigned long long v);
	void (*atomic_clear_long)(unsigned long long* p, unsigned long long v);
	void (*atomic_set_char)(u_char* P, u_char V);
	void (*atomic_clear_char)(u_char* P, u_char V);
	void (*atomic_subtract_barr_long)(volatile unsigned long long* p, unsigned long long val);
	void (*atomic_add_barr_long)(unsigned long long* P, unsigned long long V);
} so_atomic_hooks;

typedef struct so_sx_lock_hooks
{
	void (*initialize_sx_lock)(char** lock);
	void (*uninitialize_sx_lock)(char* lock);
	void (*acquire_lock_exclusive)(void* lock);
	void (*release_lock_exclusive)(void* lock);
	void (*acquire_lock_shared)(void* lock);
	void (*release_lock_shared)(void* lock);
} so_sx_lock_hooks;

typedef struct so_rw_lock_hooks
{
	void (*initialize_rw_lock)(void** lock);
	void (*uninitialize_rw_lock)(void* lock);
	void (*rw_rlock)(void* lock);
	void (*rw_runlock)(void* lock);
	void (*rw_wlock)(void* lock);
	void (*rw_wunlock)(void* lock);
} so_rw_lock_hooks;

typedef int (*fs_loop_func_t)(void *arg);

typedef int (*putchar_callback)(int c);

typedef struct so_thread_hooks
{
	void (*thread_create)(void (*func)(void*),
		void (*init)(void*),
		void* arg,
		void** threadid,
		void* newtd);
	void (*thread_destroy)(void* threadid);
} so_thread_hooks;


SO_EXPORT void InitMemHooks(so_mem_hooks* hooks);
SO_EXPORT void InitMMapHooks(so_mmap_hooks* hooks);
SO_EXPORT void InitAtomicHooks(so_atomic_hooks* hooks);
SO_EXPORT void InitSXLockHooks(so_sx_lock_hooks* hooks);
SO_EXPORT void InitStdoutHooks(putchar_callback putchar);
SO_EXPORT void InitRWLockHooks(so_rw_lock_hooks* hooks);
SO_EXPORT void InitThreadHooks(so_thread_hooks* hooks);
SO_EXPORT void InitTimeHooks(void (*sleepms)(int ms), void (*sleepus)(uint64_t us));
SO_EXPORT void InitCritical(void (*critical_enter)(void),void (*critical_exit)(void));
SO_EXPORT void InitMemBarrier(void (*mem_barrier)(void));
SO_EXPORT void InitYield(void (*yield)(void));
SO_EXPORT void InitCPUTicks(uint64_t (*cpu_ticks)(void));
SO_EXPORT void InitRdtsc(uint64_t(*dtsc)(void));
SO_EXPORT void InitRdtscp(uint64_t(*rdtscp)(void));
SO_EXPORT void InitRdtscpAux(uint64_t(*rdtscp_aux)(uint32_t* aux));
SO_EXPORT void InitRdtsc32(uint32_t(*rdtsc32)(void));
SO_EXPORT void InitRdtscp32(uint32_t(*rdtscp32)(void));
SO_EXPORT void Initsfence(void (*sfence)(void));
SO_EXPORT void Initlfence(void (*lfence)(void));
SO_EXPORT void Initmfence(void (*mfence)(void));
SO_EXPORT void InitBreak(int (*breakpoint)(void));
SO_EXPORT void InitRandBytes(void (*RandBytes)(void* ptr, unsigned int len));
SO_EXPORT void test();
SO_EXPORT void InitSetErrno(void (*set_errno)(int errnor));
#ifdef __cplusplus
}
#endif
#endif