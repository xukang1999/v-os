#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_ddb.h"
#include "opt_kdb.h"
#include "opt_init_path.h"
#include "opt_verbose_sysinit.h"

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
#include "user_sysinit.h"
extern struct sysinit* __start_set_sysinit_set;
extern struct sysinit* __stop_set_sysinit_set;

struct sysinit *sysinit_table[1024];
void InitializeObjects(void)
{
    int i = 0;
#ifdef _WIN32
    const struct sysinit** x = &__start_set_sysinit_set;
    const struct sysinit** pend = &__stop_set_sysinit_set;
    const struct sysinit** p = NULL;
    for (p = x; p <= pend; p++)
    {
        if (*p != NULL)
        {
            sysinit_table[i] = *p;
            i++;
        }

    }

    /*__start_set_sysinit_set = &sysinit_table[0];*/
    /*__stop_set_sysinit_set = &sysinit_table[i];*/
    /*sysinit_add(start, end);*/

    printf("InitializeObjects i=%d\n", i);
#elif defined(__APPLE__)

#endif
}