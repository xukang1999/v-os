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

struct kobjop_desc bus_print_child_desc;
struct kobjop_desc bus_probe_nomatch_desc;
struct kobjop_desc bus_child_deleted_desc;
struct kobjop_desc bus_child_detached_desc;
struct kobjop_desc bus_driver_added_desc;
struct kobjop_desc bus_rescan_desc;
struct kobjop_desc bus_alloc_resource_desc;
struct kobjop_desc bus_activate_resource_desc;
struct kobjop_desc bus_map_resource_desc;
struct kobjop_desc bus_unmap_resource_desc;
struct kobjop_desc bus_deactivate_resource_desc;
struct kobjop_desc bus_adjust_resource_desc;
struct kobjop_desc bus_release_resource_desc;
struct kobjop_desc bus_setup_intr_desc;
struct kobjop_desc bus_teardown_intr_desc;
struct kobjop_desc bus_suspend_intr_desc;
struct kobjop_desc bus_resume_intr_desc;
struct kobjop_desc bus_set_resource_desc;
struct kobjop_desc bus_get_resource_desc;
struct kobjop_desc bus_delete_resource_desc;
struct kobjop_desc bus_get_resource_list_desc;
struct kobjop_desc bus_child_present_desc;
struct kobjop_desc bus_child_pnpinfo_str_desc;
struct kobjop_desc bus_child_location_str_desc;
struct kobjop_desc bus_bind_intr_desc;
struct kobjop_desc bus_config_intr_desc;
struct kobjop_desc bus_describe_intr_desc;
struct kobjop_desc bus_hinted_child_desc;
struct kobjop_desc bus_get_dma_tag_desc;
struct kobjop_desc bus_get_bus_tag_desc;
struct kobjop_desc bus_hint_device_unit_desc;
struct kobjop_desc bus_new_pass_desc;
struct kobjop_desc bus_suspend_child_desc;
struct kobjop_desc bus_resume_child_desc;
struct kobjop_desc bus_get_domain_desc;
struct kobjop_desc bus_get_cpus_desc;
struct kobjop_desc bus_reset_prepare_desc;
struct kobjop_desc bus_reset_post_desc;
struct kobjop_desc bus_reset_child_desc;