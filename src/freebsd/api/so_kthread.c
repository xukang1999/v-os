#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/cpuset.h>
#include <sys/kthread.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/proc.h>
#include <sys/resourcevar.h>
#include <sys/rwlock.h>
#include <sys/signalvar.h>
#include <sys/sysent.h>
#include <sys/sx.h>
#include <sys/umtx.h>
#include <sys/unistd.h>
#include <sys/wait.h>
#include <sys/sched.h>
#include <sys/tslog.h>
#include <vm/vm.h>
#include <vm/vm_extern.h>

#include <machine/stdarg.h>
#include <compat/context.h>
#include "staros/staros.h"
static void context_init_thread_kthread(void* p)
{
	struct thread* newtd = (struct thread*)p;
	newtd->td_state = TDS_RUNNING;
	pcurthread = newtd;
	thread_unlock(newtd);
	return (0);
}
static int
kthread_add_(void (*func)(void*), void* arg, struct proc* p,
	struct thread** newtdp, int flags, int pages, const char* fmt, ...)
{
	va_list ap;
	struct thread* newtd, * oldtd;

	if (!proc0.p_stats)
		panic("kthread_add called too soon");

	/* If no process supplied, put it on proc0 */
	if (p == NULL)
		p = &proc0;

	/* Initialize our new td  */
	newtd = thread_alloc(pages);
	if (newtd == NULL)
		return (ENOMEM);

	PROC_LOCK(p);
	oldtd = FIRST_THREAD_IN_PROC(p);

	bzero(&newtd->td_startzero,
		__rangeof(struct thread, td_startzero, td_endzero));
	bcopy(&oldtd->td_startcopy, &newtd->td_startcopy,
		__rangeof(struct thread, td_startcopy, td_endcopy));

	/* set up arg0 for 'ps', et al */
	va_start(ap, fmt);
	vsnprintf(newtd->td_name, sizeof(newtd->td_name), fmt, ap);
	va_end(ap);

	TSTHREAD(newtd, newtd->td_name);

	newtd->td_proc = p;  /* needed for cpu_copy_thread */
	newtd->td_pflags |= TDP_KTHREAD;

	/* might be further optimized for kthread */
	cpu_copy_thread(newtd, oldtd);

	/* put the designated function(arg) as the resume context */
	cpu_fork_kthread_handler(newtd, func, arg);
	context_thread_create(func, context_init_thread_kthread, arg, newtd);

	thread_cow_get_proc(newtd, p);

	/* this code almost the same as create_thread() in kern_thr.c */
	p->p_flag |= P_HADTHREADS;
	thread_link(newtd, p);
	thread_lock(oldtd);
	/* let the scheduler know about these things. */
	sched_fork_thread(oldtd, newtd);
	TD_SET_CAN_RUN(newtd);
	thread_unlock(oldtd);
	PROC_UNLOCK(p);

	tidhash_add(newtd);

	/* Avoid inheriting affinity from a random parent. */
	cpuset_kernthread(newtd);
#ifdef HWPMC_HOOKS
	if (PMC_SYSTEM_SAMPLING_ACTIVE())
		PMC_CALL_HOOK_UNLOCKED(td, PMC_FN_THR_CREATE_LOG, NULL);
#endif
	/* Delay putting it on the run queue until now. */
	if (!(flags & RFSTOPPED)) {
		thread_lock(newtd);
		sched_add(newtd, SRQ_BORING);
	}
	if (newtdp)
		*newtdp = newtd;
	return 0;
}
int kthread_create(void (*func)(void*),void* arg, const char *name)
{
	struct proc* p;
	struct thread* td;
	int error;
	p = curproc;
	td = curthread;
	error = kthread_add_(func,
		(void*)arg, p, NULL, 0, 0, name);
	if (error != 0)
		panic("starting kthread %s: %d\n", name,error);

	return error;
}