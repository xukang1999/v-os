#include "ports/port_init.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif
void port_freebsd_init()
{
	so_mem_hooks memfun = { port_malloc,port_free,port_realloc,port_calloc };
	so_mmap_hooks mmapfun = { port_mmap,port_munmap };

	so_atomic_hooks hooks;
	hooks.atomic_set_int= atomic_set_int;
	hooks.atomic_clear_int = atomic_clear_int;
	hooks.atomic_add_int = atomic_add_int;
	hooks.atomic_subtract_int = atomic_subtract_int;
	hooks.atomic_add_long = atomic_add_long;
	hooks.atomic_subtract_long = atomic_subtract_long;
	hooks.atomic_cmpset_char = atomic_cmpset_char;
	hooks.atomic_cmpset_short = atomic_cmpset_short;
	hooks.atomic_cmpset_int = atomic_cmpset_int;
	hooks.atomic_cmpset_long = atomic_cmpset_long;
	hooks.atomic_fcmpset_char = atomic_fcmpset_char;
	hooks.atomic_fcmpset_short = atomic_fcmpset_short;
	hooks.atomic_fcmpset_int = atomic_fcmpset_int;
	hooks.atomic_fcmpset_long = atomic_fcmpset_long;
	hooks.atomic_thread_fence_acq = atomic_thread_fence_acq;
	hooks.atomic_thread_fence_rel = atomic_thread_fence_rel;
	hooks.atomic_thread_fence_acq_rel = atomic_thread_fence_acq_rel;
	hooks.atomic_thread_fence_seq_cst = atomic_thread_fence_seq_cst;
	hooks.atomic_add_barr_int = atomic_add_barr_int;
	hooks.atomic_store_rel_int = atomic_store_rel_int;
	hooks.atomic_store_rel_long = atomic_store_rel_long;
	hooks.atomic_load_acq_long = atomic_load_acq_long;
	hooks.atomic_load_acq_int = atomic_load_acq_int;
	hooks.atomic_subtract_barr_int = atomic_subtract_barr_int;


	hooks.atomic_swap_int = atomic_swap_int;
	hooks.atomic_swap_long = atomic_swap_long;
	hooks.atomic_set_long = atomic_set_long;
	hooks.atomic_clear_long = atomic_clear_long;
	hooks.atomic_set_char = atomic_set_char;
	hooks.atomic_clear_char = atomic_clear_char;
	hooks.atomic_subtract_barr_long = atomic_subtract_barr_long;

	hooks.atomic_add_barr_int = atomic_add_barr_int;
	hooks.atomic_add_barr_long = atomic_add_barr_long;

	hooks.atomic_fetchadd_uint = sync_atomic_fetchadd_uint;
	hooks.atomic_fetchadd_long = sync_atomic_fetchadd_long;
	hooks.atomic_fetchadd_char = sync_atomic_fetchadd_char;
	hooks.atomic_fetchadd_int = sync_atomic_fetchadd_int;
	hooks.atomic_fetchadd_8 = sync_atomic_fetchadd_8;
	hooks.atomic_fetchadd_16 = sync_atomic_fetchadd_16;
	hooks.atomic_fetchadd_32_pfn = sync_atomic_fetchadd_32_pfn;
	hooks.atomic_fetchadd_64_pfn = sync_atomic_fetchadd_64_pfn;
	hooks.atomic_fetchadd_ptr = sync_atomic_fetchadd_ptr;

	hooks.atomic_fetchsub_char = sync_atomic_fetchsub_char;
	hooks.atomic_fetchsub_int = sync_atomic_fetchsub_int;
	hooks.atomic_fetchsub_uint = sync_atomic_fetchsub_uint;
	hooks.atomic_fetchsub_long = sync_atomic_fetchsub_long;
	hooks.atomic_fetchsub_8 = sync_atomic_fetchsub_8;
	hooks.atomic_fetchsub_16 = sync_atomic_fetchsub_16;
	hooks.atomic_fetchsub_32 = sync_atomic_fetchsub_32_pfn;
	hooks.atomic_fetchsub_64= sync_atomic_fetchsub_64_pfn;
	hooks.atomic_fetchsub_ptr = sync_atomic_fetchsub_ptr;


	hooks.atomic_fetchand_char = sync_atomic_fetchand_char;
	hooks.atomic_fetchand_int = sync_atomic_fetchand_int;
	hooks.atomic_fetchand_uint = sync_atomic_fetchand_uint;
	hooks.atomic_fetchand_long = sync_atomic_fetchand_long;
	hooks.atomic_fetchand_8 = sync_atomic_fetchand_8;
	hooks.atomic_fetchand_16 = sync_atomic_fetchand_16;
	hooks.atomic_fetchand_32 = sync_atomic_fetchand_32_pfn;
	hooks.atomic_fetchand_64 = sync_atomic_fetchand_64_pfn;
	hooks.atomic_fetchand_ptr = sync_atomic_fetchand_ptr;

	hooks.atomic_fetchor_char = sync_atomic_fetchor_char;
	hooks.atomic_fetchor_int = sync_atomic_fetchor_int;
	hooks.atomic_fetchor_uint = sync_atomic_fetchor_uint;
	hooks.atomic_fetchor_long = sync_atomic_fetchor_long;
	hooks.atomic_fetchor_8 = sync_atomic_fetchor_8;
	hooks.atomic_fetchor_16 = sync_atomic_fetchor_16;
	hooks.atomic_fetchor_32 = sync_atomic_fetchor_32_pfn;
	hooks.atomic_fetchor_64 = sync_atomic_fetchor_64_pfn;
	hooks.atomic_fetchor_ptr = sync_atomic_fetchor_ptr;

	hooks.atomic_fetchxor_char = sync_atomic_fetchxor_char;
	hooks.atomic_fetchxor_int = sync_atomic_fetchxor_int;
	hooks.atomic_fetchxor_uint = sync_atomic_fetchxor_uint;
	hooks.atomic_fetchxor_long = sync_atomic_fetchxor_long;
	hooks.atomic_fetchxor_8 = sync_atomic_fetchxor_8;
	hooks.atomic_fetchxor_16 = sync_atomic_fetchxor_16;
	hooks.atomic_fetchxor_32 = sync_atomic_fetchxor_32_pfn;
	hooks.atomic_fetchxor_64 = sync_atomic_fetchxor_64_pfn;
	hooks.atomic_fetchxor_ptr = sync_atomic_fetchxor_ptr;

	hooks.sync_bool_compare_and_swap_char = sync_bool_compare_and_swap_char;
	hooks.sync_val_compare_and_swap_char = sync_val_compare_and_swap_char;
	hooks.sync_bool_compare_and_swap_int = sync_bool_compare_and_swap_int;
	hooks.sync_val_compare_and_swap_int = sync_val_compare_and_swap_int;
	hooks.sync_bool_compare_and_swap_uint = sync_bool_compare_and_swap_uint;
	hooks.sync_val_compare_and_swap_uint = sync_val_compare_and_swap_uint;
	hooks.sync_bool_compare_and_swap_8 = sync_bool_compare_and_swap_8;
	hooks.sync_val_compare_and_swap_8 = sync_val_compare_and_swap_8;
	hooks.sync_bool_compare_and_swap_16 = sync_bool_compare_and_swap_16;
	hooks.sync_val_compare_and_swap_16 = sync_val_compare_and_swap_16;
	hooks.sync_bool_compare_and_swap_32 = sync_bool_compare_and_swap_32;
	hooks.sync_val_compare_and_swap_32 = sync_val_compare_and_swap_32;
	hooks.sync_bool_compare_and_swap_64 = sync_bool_compare_and_swap_64;
	hooks.sync_val_compare_and_swap_64 = sync_val_compare_and_swap_64;
	hooks.sync_bool_compare_and_swap_ptr = sync_bool_compare_and_swap_ptr;
	hooks.sync_val_compare_and_swap_ptr = sync_val_compare_and_swap_ptr;

	InitStdoutHooks(port_putchar);

	InitAtomicHooks(&hooks);
	InitMemHooks(&memfun);
	InitMMapHooks(&mmapfun);

	so_sx_lock_hooks sxlockhooks;
	sxlockhooks.initialize_sx_lock = initialize_sx_lock;
	sxlockhooks.uninitialize_sx_lock = uninitialize_sx_lock;
	sxlockhooks.acquire_lock_exclusive = acquire_lock_exclusive;
	sxlockhooks.release_lock_exclusive = release_lock_exclusive;
	sxlockhooks.acquire_lock_shared = acquire_lock_shared;
	sxlockhooks.release_lock_shared = release_lock_shared;

	InitSXLockHooks(&sxlockhooks);

	so_rw_lock_hooks rwlockhooks;
	rwlockhooks.initialize_rw_lock = port_rw_init;
	rwlockhooks.uninitialize_rw_lock = port_rw_destroy;
	rwlockhooks.rw_rlock = port_rw_rlock;
	rwlockhooks.rw_runlock = port_rw_runlock;
	rwlockhooks.rw_wlock = port_rw_wlock;
	rwlockhooks.rw_wunlock = port_rw_wunlock;
	InitRWLockHooks(&rwlockhooks);

	so_thread_hooks threadhooks;

	threadhooks.thread_create = thread_create;
	threadhooks.thread_destroy = thread_destroy;

	InitThreadHooks(&threadhooks);
	InitTimeHooks(port_mssleep, port_ussleep);

	InitCritical(port_critical_enter,port_critical_exit);

	InitMemBarrier(port_memory_barrier);
	InitYield(port_yield);
	InitCPUTicks(port_cpud_ticks);

	InitRdtsc(port_rdtsc);
	InitRdtscp(port_rdtscp);
	InitRdtscpAux(port_rdtscp_aux);
	InitRdtsc32(port_rdtsc32);
	InitRdtscp32(port_rdtscp32);


	Initsfence(port_sfence);
	Initlfence(port_lfence);
	Initmfence(port_mfence);
	InitBreak(port_breakpoint);

	InitRandBytes(port_randbyte);

	InitSetErrno(port_set_errno);

}