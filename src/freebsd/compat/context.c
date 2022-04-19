#include <sys/cdefs.h>
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/epoch.h>
#include <sys/eventhandler.h>
#include <sys/exec.h>
#include <sys/file.h>
#include <sys/filedesc.h>
#include <sys/imgact.h>
#include <sys/jail.h>
#include <sys/ktr.h>
#include <sys/lock.h>
#include <sys/loginclass.h>
#include <sys/mount.h>
#include <sys/mutex.h>
#include <sys/dtrace_bsd.h>
#include <sys/syscallsubr.h>
#include <sys/sysctl.h>
#include <sys/proc.h>
#include <sys/racct.h>
#include <sys/resourcevar.h>
#include <sys/systm.h>
#include <sys/signalvar.h>
#include <sys/vnode.h>
#include <sys/sysent.h>
#include <sys/reboot.h>
#include <sys/sched.h>
#include <sys/sx.h>
#include <sys/sysproto.h>
#include <sys/vmmeter.h>
#include <sys/unistd.h>
#include <sys/malloc.h>
#include <sys/conf.h>
#include <sys/cpuset.h>

#include <machine/cpu.h>

#include <security/audit/audit.h>
#include <security/mac/mac_framework.h>

#include <vm/vm.h>
#include <vm/vm_param.h>
#include <vm/vm_extern.h>
#include <vm/pmap.h>
#include <vm/vm_map.h>

#include <ddb/ddb.h>
#include <ddb/db_sym.h>
#include <sys/stdint.h>
#include <sys/_null.h>
#include "context.h"

internal_hooks global_hooks = { NULL };

so_atomic_hooks global_atomic_hooks = { NULL };

void InitMemHooks(so_mem_hooks* hooks)
{
    if (hooks == NULL)
    {
        /* Reset hooks */
        return;
    }

    global_hooks.allocate = hooks->malloc_fn;
    global_hooks.deallocate = hooks->free_fn;
    global_hooks.reallocate = hooks->reallocat_fn;
    global_hooks.calloc = hooks->calloc;
}

void InitMMapHooks(so_mmap_hooks* hooks)
{
    global_hooks.mmap = hooks->mmap;
    global_hooks.munmap = hooks->munmap;
}

void InitAtomicHooks(so_atomic_hooks* hooks)
{
    global_atomic_hooks.atomic_set_int = hooks->atomic_set_int;
    global_atomic_hooks.atomic_clear_int = hooks->atomic_clear_int;
    global_atomic_hooks.atomic_add_int = hooks->atomic_add_int;
    global_atomic_hooks.atomic_subtract_int = hooks->atomic_subtract_int;
    global_atomic_hooks.atomic_add_long = hooks->atomic_add_long;
    global_atomic_hooks.atomic_subtract_long = hooks->atomic_subtract_long;
    global_atomic_hooks.atomic_fetchadd_int = hooks->atomic_fetchadd_int;
    global_atomic_hooks.atomic_fetchadd_long = hooks->atomic_fetchadd_long;
    global_atomic_hooks.atomic_cmpset_char = hooks->atomic_cmpset_char;
    global_atomic_hooks.atomic_cmpset_short = hooks->atomic_cmpset_short;
    global_atomic_hooks.atomic_cmpset_int = hooks->atomic_cmpset_int;
    global_atomic_hooks.atomic_cmpset_long = hooks->atomic_cmpset_long;
    global_atomic_hooks.atomic_fcmpset_char = hooks->atomic_fcmpset_char;
    global_atomic_hooks.atomic_fcmpset_short = hooks->atomic_fcmpset_short;
    global_atomic_hooks.atomic_fcmpset_int = hooks->atomic_fcmpset_int;
    global_atomic_hooks.atomic_fcmpset_long = hooks->atomic_fcmpset_long;
    global_atomic_hooks.atomic_thread_fence_acq = hooks->atomic_thread_fence_acq;
    global_atomic_hooks.atomic_thread_fence_rel = hooks->atomic_thread_fence_rel;
    global_atomic_hooks.atomic_thread_fence_acq_rel = hooks->atomic_thread_fence_acq_rel;
    global_atomic_hooks.atomic_thread_fence_seq_cst = hooks->atomic_thread_fence_seq_cst;
    global_atomic_hooks.atomic_add_barr_int = hooks->atomic_add_barr_int;
    global_atomic_hooks.atomic_store_rel_int = hooks->atomic_store_rel_int;
    global_atomic_hooks.atomic_store_rel_long = hooks->atomic_store_rel_long;
    global_atomic_hooks.atomic_load_acq_long = hooks->atomic_load_acq_long;
    global_atomic_hooks.atomic_load_acq_int = hooks->atomic_load_acq_int;
    global_atomic_hooks.atomic_subtract_barr_int = hooks->atomic_subtract_barr_int;

    global_atomic_hooks.atomic_swap_int = hooks->atomic_swap_int;
    global_atomic_hooks.atomic_swap_long = hooks->atomic_swap_long;
    global_atomic_hooks.atomic_set_long = hooks->atomic_set_long;
    global_atomic_hooks.atomic_clear_long = hooks->atomic_clear_long;
    global_atomic_hooks.atomic_set_char = hooks->atomic_set_char;
    global_atomic_hooks.atomic_clear_char = hooks->atomic_clear_char;
    global_atomic_hooks.atomic_subtract_barr_long = hooks->atomic_subtract_barr_long;

    global_atomic_hooks.atomic_add_barr_int = hooks->atomic_add_barr_int;
    global_atomic_hooks.atomic_add_barr_long = hooks->atomic_add_barr_long;

    global_atomic_hooks.atomic_fetchadd_char = hooks->atomic_fetchadd_char;
    global_atomic_hooks.atomic_fetchadd_int = hooks->atomic_fetchadd_int;
    global_atomic_hooks.atomic_fetchadd_uint = hooks->atomic_fetchadd_uint;
    global_atomic_hooks.atomic_fetchadd_long = hooks->atomic_fetchadd_long;
    global_atomic_hooks.atomic_fetchadd_8 = hooks->atomic_fetchadd_8;
    global_atomic_hooks.atomic_fetchadd_16 = hooks->atomic_fetchadd_16;
    global_atomic_hooks.atomic_fetchadd_32_pfn = hooks->atomic_fetchadd_32_pfn;
    global_atomic_hooks.atomic_fetchadd_64_pfn = hooks->atomic_fetchadd_64_pfn;
    global_atomic_hooks.atomic_fetchadd_ptr = hooks->atomic_fetchadd_ptr;

    global_atomic_hooks.atomic_fetchsub_char = hooks->atomic_fetchsub_char;
    global_atomic_hooks.atomic_fetchsub_int = hooks->atomic_fetchsub_int;
    global_atomic_hooks.atomic_fetchsub_uint = hooks->atomic_fetchsub_uint;
    global_atomic_hooks.atomic_fetchsub_long = hooks->atomic_fetchsub_long;
    global_atomic_hooks.atomic_fetchsub_8 = hooks->atomic_fetchsub_8;
    global_atomic_hooks.atomic_fetchsub_16 = hooks->atomic_fetchsub_16;
    global_atomic_hooks.atomic_fetchsub_32 = hooks->atomic_fetchsub_32;
    global_atomic_hooks.atomic_fetchsub_64 = hooks->atomic_fetchsub_64;
    global_atomic_hooks.atomic_fetchsub_ptr = hooks->atomic_fetchsub_ptr;

    global_atomic_hooks.atomic_fetchor_char = hooks->atomic_fetchor_char;
    global_atomic_hooks.atomic_fetchor_int = hooks->atomic_fetchor_int;
    global_atomic_hooks.atomic_fetchor_uint = hooks->atomic_fetchor_uint;
    global_atomic_hooks.atomic_fetchor_long = hooks->atomic_fetchor_long;
    global_atomic_hooks.atomic_fetchor_8 = hooks->atomic_fetchor_8;
    global_atomic_hooks.atomic_fetchor_16 = hooks->atomic_fetchor_16;
    global_atomic_hooks.atomic_fetchor_32 = hooks->atomic_fetchor_32;
    global_atomic_hooks.atomic_fetchor_64 = hooks->atomic_fetchor_64;
    global_atomic_hooks.atomic_fetchor_ptr = hooks->atomic_fetchor_ptr;

    global_atomic_hooks.atomic_fetchand_char = hooks->atomic_fetchand_char;
    global_atomic_hooks.atomic_fetchand_int = hooks->atomic_fetchand_int;
    global_atomic_hooks.atomic_fetchand_uint = hooks->atomic_fetchand_uint;
    global_atomic_hooks.atomic_fetchand_long = hooks->atomic_fetchand_long;
    global_atomic_hooks.atomic_fetchand_8 = hooks->atomic_fetchand_8;
    global_atomic_hooks.atomic_fetchand_16 = hooks->atomic_fetchand_16;
    global_atomic_hooks.atomic_fetchand_32 = hooks->atomic_fetchand_32;
    global_atomic_hooks.atomic_fetchand_64 = hooks->atomic_fetchand_64;
    global_atomic_hooks.atomic_fetchand_ptr = hooks->atomic_fetchand_ptr;

    global_atomic_hooks.atomic_fetchxor_char = hooks->atomic_fetchxor_char;
    global_atomic_hooks.atomic_fetchxor_int = hooks->atomic_fetchxor_int;
    global_atomic_hooks.atomic_fetchxor_uint = hooks->atomic_fetchxor_uint;
    global_atomic_hooks.atomic_fetchxor_long = hooks->atomic_fetchxor_long;
    global_atomic_hooks.atomic_fetchxor_8 = hooks->atomic_fetchxor_8;
    global_atomic_hooks.atomic_fetchxor_16 = hooks->atomic_fetchxor_16;
    global_atomic_hooks.atomic_fetchxor_32 = hooks->atomic_fetchxor_32;
    global_atomic_hooks.atomic_fetchxor_64 = hooks->atomic_fetchxor_64;
    global_atomic_hooks.atomic_fetchxor_ptr = hooks->atomic_fetchxor_ptr;

    global_atomic_hooks.sync_bool_compare_and_swap_char = hooks->sync_bool_compare_and_swap_char;
    global_atomic_hooks.sync_val_compare_and_swap_char = hooks->sync_val_compare_and_swap_char;
    global_atomic_hooks.sync_bool_compare_and_swap_int = hooks->sync_bool_compare_and_swap_int;
    global_atomic_hooks.sync_val_compare_and_swap_int = hooks->sync_val_compare_and_swap_int;
    global_atomic_hooks.sync_bool_compare_and_swap_uint = hooks->sync_bool_compare_and_swap_uint;
    global_atomic_hooks.sync_val_compare_and_swap_uint = hooks->sync_val_compare_and_swap_uint;
    global_atomic_hooks.sync_bool_compare_and_swap_8 = hooks->sync_bool_compare_and_swap_8;
    global_atomic_hooks.sync_val_compare_and_swap_8 = hooks->sync_val_compare_and_swap_8;
    global_atomic_hooks.sync_bool_compare_and_swap_16 = hooks->sync_bool_compare_and_swap_16;
    global_atomic_hooks.sync_val_compare_and_swap_16 = hooks->sync_val_compare_and_swap_16;
    global_atomic_hooks.sync_bool_compare_and_swap_32 = hooks->sync_bool_compare_and_swap_32;
    global_atomic_hooks.sync_val_compare_and_swap_32 = hooks->sync_val_compare_and_swap_32;
    global_atomic_hooks.sync_bool_compare_and_swap_64 = hooks->sync_bool_compare_and_swap_64;
    global_atomic_hooks.sync_val_compare_and_swap_64 = hooks->sync_val_compare_and_swap_64;
    global_atomic_hooks.sync_bool_compare_and_swap_ptr = hooks->sync_bool_compare_and_swap_ptr;
    global_atomic_hooks.sync_val_compare_and_swap_ptr = hooks->sync_val_compare_and_swap_ptr;

}

void test()
{

}
void*
context_mmap(void* addr, uint64_t len, int prot, int flags, int fd, uint64_t offset)
{
    if (global_hooks.mmap == NULL)
    {
        return NULL;
    }
    void* ret = (global_hooks.mmap(addr, len, prot, flags, fd, offset));
    return ret;
}

int
context_munmap(void* addr, uint64_t len)
{
    if (global_hooks.munmap == NULL)
    {
        return -1;
    }
    return (global_hooks.munmap(addr, len));
}

char* context_malloc(size_t size)
{
    if (global_hooks.allocate == NULL)
    {
        return NULL;
    }
    char * pmem = global_hooks.allocate(size);
    if (pmem)
    {
        memset(pmem, 0, size);
    }
    return pmem;
}
void context_free(const char* p)
{
    if (global_hooks.deallocate == NULL)
    {
        return;
    }
    global_hooks.deallocate(p);
}

char* context_realloc(void* p, size_t size)
{
    if (global_hooks.reallocate == NULL)
    {
        return NULL;
    }
    return global_hooks.reallocate(p, size);
}

void InitSXLockHooks(so_sx_lock_hooks* hooks)
{
    global_hooks.initialize_sx_lock = hooks->initialize_sx_lock;
    global_hooks.uninitialize_sx_lock = hooks->uninitialize_sx_lock;
    global_hooks.acquire_lock_exclusive = hooks->acquire_lock_exclusive;
    global_hooks.release_lock_exclusive = hooks->release_lock_exclusive;
    global_hooks.acquire_lock_shared = hooks->acquire_lock_shared;
    global_hooks.release_lock_shared = hooks->release_lock_shared;
}

void InitStdoutHooks(putchar_callback putchar)
{
    global_hooks.putchar = putchar;
}
void InitRWLockHooks(so_rw_lock_hooks* hooks)
{
    global_hooks.initialize_rw_lock = hooks->initialize_rw_lock;
    global_hooks.uninitialize_rw_lock = hooks->uninitialize_rw_lock;
    global_hooks.rw_rlock_cb = hooks->rw_rlock;
    global_hooks.rw_runlock_cb = hooks->rw_runlock;
    global_hooks.rw_wlock_cb = hooks->rw_wlock;
    global_hooks.rw_wunlock_cb = hooks->rw_wunlock;
}
int context_putchar(int c)
{
    if (global_hooks.putchar == NULL)
    {
        return -1;
    }
    return global_hooks.putchar(c);
}
int context_puts(const char *str)
{
    if (global_hooks.puts == NULL)
    {
        return -1;
    }
    return global_hooks.puts(str);
}
void InitThreadHooks(so_thread_hooks* hooks)
{
    global_hooks.thread_create = hooks->thread_create;
    global_hooks.thread_destroy = hooks->thread_destroy;
}
void context_mssleep(int msecs)
{
    if (global_hooks.mssleep == NULL)
    {
        return ;
    }
    global_hooks.mssleep(msecs);
}
void context_usleep(uint64_t usecs)
{
    if (global_hooks.ussleep == NULL)
    {
        return;
    }
    global_hooks.ussleep(usecs);
}
void InitTimeHooks(void (*sleepms)(int ms), void (*sleepus)(uint64_t us))
{
    global_hooks.mssleep = sleepms;
    global_hooks.ussleep = sleepus;
}
void InitCritical(void (*critical_enter)(void), void (*critical_exit)(void))
{
    global_hooks.critical_enter = critical_enter;
    global_hooks.critical_exit = critical_exit;
}
void InitMemBarrier(void (*mem_barrier)(void))
{
    global_hooks.mem_barrier = mem_barrier;
}
void InitYield(void (*yield)(void))
{
    global_hooks.yield = yield;
}
void InitCPUTicks(uint64_t(*cpu_ticks)(void))
{
    global_hooks.cpu_ticks = cpu_ticks;
}
void InitRdtsc(uint64_t(*dtsc)(void))
{
    global_hooks.rdtsc = dtsc;
}
void InitRdtscp(uint64_t(*rdtscp)(void))
{
    global_hooks.rdtscp = rdtscp;
}
void InitRdtscpAux(uint64_t(*rdtscp_aux)(uint32_t* aux))
{
    global_hooks.rdtscp_aux = rdtscp_aux;
}
void InitRdtsc32(uint32_t(*rdtsc32)(void))
{
    global_hooks.rdtsc32 = rdtsc32;
}
void InitRdtscp32(uint32_t(*rdtscp32)(void))
{
    global_hooks.rdtscp32 = rdtscp32;
}

void InitBreak(int (*breakpoint)(void))
{
    global_hooks.breakpoint = breakpoint;
}
void InitRandBytes(void (*RandBytes)(void* ptr, unsigned int len))
{
    global_hooks.rand_bytes = RandBytes;
}

void Initsfence(void (*sfence)(void))
{
    global_hooks.sfence = sfence;
}
void Initlfence(void (*lfence)(void))
{
    global_hooks.lfence = lfence;
}
void Initmfence(void (*mfence)(void))
{
    global_hooks.mfence = mfence;
}

void context_critical_enter(void)
{
    if (global_hooks.critical_enter == NULL)
    {
        return;
    }
    global_hooks.critical_enter();
}
void context_critical_exit(void)
{
    if (global_hooks.critical_exit == NULL)
    {
        return;
    }
    global_hooks.critical_exit();
}

void context_mb(void)
{
    if (global_hooks.mem_barrier == NULL)
    {
        return;
    }
    global_hooks.mem_barrier();
}

void context_yield(void)
{
    if (global_hooks.yield == NULL)
    {
        return;
    }
    global_hooks.yield();
}
uint64_t context_rdtsc(void)
{
    if (global_hooks.rdtsc == NULL)
    {
        return 0;
    }
    return global_hooks.rdtsc();
}
uint64_t context_rdtscp(void)
{
    if (global_hooks.rdtscp == NULL)
    {
        return 0;
    }
    return global_hooks.rdtscp();
}
uint64_t context_rdtscp_aux(uint32_t *aux)
{
    if (global_hooks.rdtscp_aux == NULL)
    {
        return 0;
    }
    return global_hooks.rdtscp_aux(aux);
}
uint32_t context_rdtscp32()
{
    if (global_hooks.rdtscp32 == NULL)
    {
        return 0;
    }
    return global_hooks.rdtscp32();
}
uint32_t context_rdtsc32()
{
    if (global_hooks.rdtsc32 == NULL)
    {
        return 0;
    }
    return global_hooks.rdtsc32();
}
uint64_t context_cpu_ticks(void)
{
    if (global_hooks.cpu_ticks == NULL)
    {
        return 0;
    }
    return global_hooks.cpu_ticks();
}

void context_mfence(void)
{
    if (global_hooks.mfence == NULL)
    {
        return;
    }
    global_hooks.mfence();
}
void context_sfence(void)
{
    if (global_hooks.sfence == NULL)
    {
        return;
    }
    global_hooks.sfence();
}
void context_lfence(void)
{
    if (global_hooks.lfence == NULL)
    {
        return;
    }
    global_hooks.lfence();
}

int context_breakpoint(void)
{
    if (global_hooks.breakpoint == NULL)
    {
        return -1;
    }
    return global_hooks.breakpoint();
}

void
context_rand_bytes(void* ptr, unsigned int len)
{
    if (global_hooks.rand_bytes == NULL)
    {
        return;
    }
    global_hooks.rand_bytes(ptr, len);
}


void context_set_errno(int errno)
{
    if (global_hooks.set_errno == NULL)
    {
        return;
    }
    global_hooks.set_errno(errno);
}

void InitSetErrno(void (*set_errno)(int errnor))
{
    global_hooks.set_errno = set_errno;
}