#include <sys/cdefs.h>
#include <sys/param.h>
#include <sys/pcpu.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/lock.h>
#include <sys/sx.h>
#include <sys/vmmeter.h>
#include <sys/cpuset.h>
#include <sys/sysctl.h>
#include <sys/filedesc.h>
#include <vm/vm.h>
#include <vm/vm_param.h>
#include <vm/vm_domainset.h>
#include <vm/vm_object.h>
#include <vm/vm_page.h>
#include <vm/vm_pageout.h>
#include <vm/vm_phys.h>
#include <vm/vm_pagequeue.h>
#include <vm/vm_map.h>
#include <vm/vm_kern.h>
#include <vm/vm_extern.h>
#include <vm/vm_dumpset.h>
#include <vm/uma.h>
#include <vm/uma_int.h>
#include <vm/uma_dbg.h>
#include <sys/_stdarg.h>
#include <sys/nlist_aout.h>
#include <sys/link_aout.h>
#include "staros/staros.h"
#include "compat/syscall_wrapper.h"
int vos_kqueue(void)
{
    return ff_kqueue();
}
int vos_kevent(int kq, const struct so_kevent* changelist, int nchanges,
    struct so_kevent* eventlist, int nevents, const struct timespec* timeout)
{
    return ff_kevent(kq, changelist, nchanges, eventlist, nevents, timeout);
}
typedef void (*do_each_type)(void**, struct kevent*);
int vos_kevent_do_each(int kq, const struct so_kevent* changelist, int nchanges,
    void* eventlist, int nevents, const struct timespec* timeout,
    void (*do_each)(void**, struct so_kevent*))
{

    return ff_kevent_do_each(kq, (const struct kevent*)changelist, nchanges, eventlist, nevents, timeout, (do_each_type)do_each);
}