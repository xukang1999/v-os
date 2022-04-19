#ifndef _SO_CONTEXT_H_
#define _SO_CONTEXT_H_
#include "staros/so_init.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct internal_hooks
{
	int (*putchar)(int c);
	int (*puts)(const char *str);
	void* (*allocate)(size_t size);
	void (*deallocate)(void* pointer);
	void* (*calloc)(size_t num, size_t size);
	void* (* reallocate)(void* pointer, size_t size);
	void* (*mmap)(void* addr, uint64_t len, int prot, int flags, int fd, uint64_t offset);
	int (*munmap)(void* addr, uint64_t len);

	void (*initialize_sx_lock)(char** lock);
	void (*uninitialize_sx_lock)(char* lock);
	void (*acquire_lock_exclusive)(void* lock);
	void (*release_lock_exclusive)(void* lock);
	void (*acquire_lock_shared)(void* lock);
	void (*release_lock_shared)(void* lock);

	/*wrlock*/
	void (*initialize_rw_lock)(void** lock);
	void (*uninitialize_rw_lock)(void* lock);
	void (*rw_rlock_cb)(void* lock);
	void (*rw_runlock_cb)(void* lock);
	void (*rw_wlock_cb)(void* lock);
	void (*rw_wunlock_cb)(void* lock);
	/*thread*/
	void (*thread_create)(void (*func)(void*),
		void (*init)(void*),
		void* arg,
		void** threadid,
		void* newtd);
	void (*thread_destroy)(void *threadid);

	/*time*/
	void (*mssleep)(int mssec);
	void (*ussleep)(uint64_t usec);
	uint64_t (*cpu_ticks)(void);

	void (*critical_enter)(void);
	void (*critical_exit)(void);

	/*mb*/
	void (*mem_barrier)(void);

	void (*yield)(void);

	uint64_t (*rdtsc)(void);
	uint64_t (*rdtscp)(void);
	uint64_t (*rdtscp_aux)(uint32_t* aux);
	uint32_t (*rdtsc32)(void);
	uint32_t (*rdtscp32)(void);
	/*mem*/
	void (*mfence)(void);
	void (*sfence)(void);
	void (*lfence)(void);

	int (*breakpoint)(void);

	/*rand*/
	void (*rand_bytes)(void* ptr, unsigned int len);

	void (*set_errno)(int errno);
} internal_hooks;


char* context_malloc(size_t size);
void context_free(const char* p);
char* context_realloc(void* p, size_t size);
void* context_mmap(void* addr, uint64_t len, int prot, int flags, int fd, uint64_t offset);
int context_munmap(void* addr, uint64_t len);

typedef void (* initialize_sx_lock)(char **lock);

typedef void (* acquire_lock_exclusive)(void *lock);
typedef void (* release_lock_exclusive)(void* lock);

typedef void (* acquire_lock_shared)(void* lock);
typedef void (* release_lock_shared)(void* lock);


int context_putchar(int c);
int context_puts(const char *str);
uintptr_t context_thread_create(void (*func)(void*), void (*init)(void*), void* arg, void* newtd);
void context_thread_destroy(uintptr_t threadid);

void context_mssleep(int msecs);
void context_usleep(uint64_t usecs);
void context_critical_enter(void);
void context_critical_exit(void);
void context_mb(void);
void context_yield(void);
uint64_t context_cpu_ticks(void);
uint64_t context_rdtsc(void);
uint32_t context_rdtsc32(void);
uint64_t context_rdtscp(void);
uint32_t context_rdtscp32(void);
uint64_t context_rdtscp_aux(uint32_t *aux);
void context_mfence(void);
void context_sfence(void);
void context_lfence(void);
void context_rand_bytes(void* ptr, unsigned int len);
int context_breakpoint(void);
#ifdef __cplusplus
}
#endif
#endif/*_SO_CONTEXT_H_*/
