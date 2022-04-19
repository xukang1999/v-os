#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_bus.h"
#include "opt_ddb.h"

#include <sys/param.h>
#include <sys/conf.h>
#include <sys/domainset.h>
#include <sys/eventhandler.h>
#include <sys/filio.h>
#include <sys/lock.h>
#include <sys/kernel.h>
#include <sys/kobj.h>
#include <sys/limits.h>
#include <sys/malloc.h>
#include <sys/module.h>
#include <sys/mutex.h>
#include <sys/poll.h>
#include <sys/priv.h>
#include <sys/proc.h>
#include <sys/condvar.h>
#include <sys/queue.h>
#include <machine/bus.h>
#include <sys/random.h>
#include <sys/rman.h>
#include <sys/sbuf.h>
#include <sys/selinfo.h>
#include <sys/signalvar.h>
#include <sys/smp.h>
#include <sys/sysctl.h>
#include <sys/systm.h>
#include <sys/uio.h>
#include <sys/bus.h>
#include <sys/cpuset.h>

#include <net/vnet.h>

#include <machine/cpu.h>
#include <machine/stdarg.h>

#include <vm/uma.h>
#include <vm/vm.h>

#include <ddb/ddb.h>

struct kobjop_desc device_probe_desc;
struct kobjop_desc  device_probe_desc;
struct kobjop_desc device_identify_desc;
struct kobjop_desc device_attach_desc;
struct kobjop_desc  device_attach_desc;
struct kobjop_desc device_detach_desc;
struct kobjop_desc  device_detach_desc;
struct kobjop_desc device_shutdown_desc;
struct kobjop_desc  device_shutdown_desc;
struct kobjop_desc device_suspend_desc;
struct kobjop_desc  device_suspend_desc;
struct kobjop_desc device_resume_desc;
struct kobjop_desc  device_resume_desc;
struct kobjop_desc device_quiesce_desc;