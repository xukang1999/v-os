#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/proc.h>
#include <sys/bitstring.h>
#include <sys/epoch.h>
#include <sys/rangelock.h>
#include <sys/resourcevar.h>
#include <sys/sdt.h>
#include <sys/smp.h>
#include <sys/sched.h>
#include <sys/sleepqueue.h>
#include <sys/selinfo.h>
#include <sys/syscallsubr.h>
#include <sys/dtrace_bsd.h>
#include <sys/sysent.h>
#include <sys/turnstile.h>
#include <sys/taskqueue.h>
#include <sys/ktr.h>
#include <sys/rwlock.h>
#include <sys/umtx.h>
#include <sys/vmmeter.h>
#include <sys/cpuset.h>
#ifdef	HWPMC_HOOKS
#include <sys/pmckern.h>
#endif
#include <sys/priv.h>

#include <security/audit/audit.h>

#include <vm/pmap.h>
#include <vm/vm.h>
#include <vm/vm_extern.h>
#include <vm/uma.h>
#include <vm/vm_phys.h>
#include <sys/eventhandler.h>
#include "context.h"
extern internal_hooks global_hooks;
static void context_init_thread_(void* p)
{
    struct thread* newtd = (struct thread*)p;
    newtd->td_state = TDS_RUNNING;
    pcurthread = newtd;
}
uintptr_t context_thread_create(void (*func)(void*), void (*init)(void*),void* arg, void *newtd)
{
    uintptr_t id = 0;
    global_hooks.thread_create(func, context_init_thread_, arg, (void**)&id, newtd);
    return id;
}
void context_thread_destroy(uintptr_t threadid)
{
    global_hooks.thread_destroy((void*)threadid);
}
