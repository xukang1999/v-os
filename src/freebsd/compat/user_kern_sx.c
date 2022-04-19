#include "opt_ddb.h"
#include "opt_hwpmc_hooks.h"

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kdb.h>
#include <sys/kernel.h>
#include <sys/ktr.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/proc.h>
#include <sys/sched.h>
#include <sys/sleepqueue.h>
#include <sys/sx.h>
#include <sys/smp.h>
#include <sys/sysctl.h>

#include "context.h"


extern internal_hooks global_hooks;

void lock_sx_initialize_(struct lock_object* lock)
{
	char* p = NULL;
	global_hooks.initialize_sx_lock(&p);
	lock->lo_witness = p;
}

void lock_sx_uninitialize_(struct lock_object* lock)
{
	global_hooks.uninitialize_sx_lock(lock->lo_witness);
}

int __noinline
_sx_slock_hard_(struct sx *sx, int opts, uintptr_t x LOCK_FILE_LINE_ARG_DEF)
{
	global_hooks.release_lock_shared(sx->lock_object.lo_witness);
	return 0;
}

int
_sx_xlock_hard_(struct sx* sx, uintptr_t x, int opts LOCK_FILE_LINE_ARG_DEF)
{
	global_hooks.acquire_lock_exclusive(sx->lock_object.lo_witness);
	return 0;
}

void
_sx_xunlock_hard_(struct sx* sx, uintptr_t x LOCK_FILE_LINE_ARG_DEF)
{
	global_hooks.release_lock_exclusive(sx->lock_object.lo_witness);
}
void __noinline
_sx_sunlock_hard_(struct sx* sx, struct thread* td, uintptr_t x
	LOCK_FILE_LINE_ARG_DEF)
{
	global_hooks.release_lock_shared(sx->lock_object.lo_witness);
}
