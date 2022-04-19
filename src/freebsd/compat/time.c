#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_callout_profiling.h"
#include "opt_ddb.h"
#if defined(__arm__)
#include "opt_timer.h"
#endif
#include "opt_rss.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/bus.h>
#include <sys/callout.h>
#include <sys/file.h>
#include <sys/interrupt.h>
#include <sys/kernel.h>
#include <sys/ktr.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/mutex.h>
#include <sys/proc.h>
#include <sys/sdt.h>
#include <sys/sleepqueue.h>
#include <sys/sysctl.h>
#include <sys/smp.h>
#include <sys/timetc.h>
#include "context.h"
extern internal_hooks global_hooks;
uint64_t get_tsc_ns()
{
	return global_hooks.get_tsc_ns();
}