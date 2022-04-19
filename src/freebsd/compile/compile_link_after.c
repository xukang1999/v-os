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
#include "compat/user_sysinit.h"
void link_after_sys_init_cfunc(const void* arg)
{
	printf("link_after_sys_init_cfunc\n");
}
static struct sysinit link_after_sys_init = {
	0,
	0,
	link_after_sys_init_cfunc,
	NULL
};

static struct sysctl_oid link_after_sysctl_init = { 0 };
static struct mod_metadata link_after_modmetadata_init = { 0 };

static uintptr_t link_after_pcpu_init = 0;

#if defined(_MSC_VER)
/*struct sysinit* __stop_set_sysinit_set = NULL;*/
_Pragma("section(\".set_sysinit_set\",write,read)")
__declspec(allocate(".set_sysinit_set"))
const struct sysinit* __stop_set_sysinit_set = (struct sysinit*)&link_after_sys_init;

/* sysctl*/
_Pragma("section(\".set_sysctl_set\",write,read)")
__declspec(allocate(".set_sysctl_set"))
const struct sysctl_oid* __stop_set_sysctl_set = (struct sysctl_oid*)&link_after_sysctl_init;


/*module*/
_Pragma("section(\".set_modmetadata_set\",write,read)")
__declspec(allocate(".set_modmetadata_set"))
const struct mod_metadata* __stop_set_modmetadata_set = (struct mod_metadata*)&link_after_modmetadata_init;

/*pcpu*/
/*module*/
_Pragma("section(\".set_pcpu\",write,read)")
__declspec(allocate(".set_pcpu"))
const uintptr_t* __stop_set_pcpu = (uintptr_t*)&link_after_pcpu_init;

#elif defined(__APPLE__) && defined(__MACH__)
struct sysinit* __stop_set_sysinit_set __attribute__((used, section("__DATA, sysinit_set")))
	= (struct sysinit*)&link_after_sys_init;

/* sysctl*/
struct sysctl_oid* __stop_set_sysctl_set __attribute__((used, section("__DATA, sysctl_set")))
	= (struct sysctl_oid*)&link_after_sysctl_init;

/* sysctl*/
struct mod_metadata* __stop_set_modmetadata_set __attribute__((used, section("__DATA, modmetadata_set")))
	= (struct mod_metadata*)&link_after_modmetadata_init;

/* sysctl*/
uintptr_t* __stop_set_pcpu __attribute__((used, section("__DATA, pcpu_set")))
	= (uintptr_t*)&link_after_pcpu_init;

#else


#endif