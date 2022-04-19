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
#include "staros/so_api.h"
extern int vos_freebsd_init(void);
int vos_init(void)
{
	vos_freebsd_init();
	return 0;
}
int vos_setenv(const char* name, const char* value)
{
	return kern_setenv(name, value);
}