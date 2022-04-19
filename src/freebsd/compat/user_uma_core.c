/*-
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 *
 * Copyright (c) 2002-2019 Jeffrey Roberson <jeff@FreeBSD.org>
 * Copyright (c) 2004, 2005 Bosko Milekic <bmilekic@FreeBSD.org>
 * Copyright (c) 2004-2006 Robert N. M. Watson
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice unmodified, this list of conditions, and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

 /*
  * uma_core.c  Implementation of the Universal Memory allocator
  *
  * This allocator is intended to replace the multitude of similar object caches
  * in the standard FreeBSD kernel.  The intent is to be flexible as well as
  * efficient.  A primary design goal is to return unused memory to the rest of
  * the system.  This will make the system as a whole more flexible due to the
  * ability to move memory to subsystems which most need it instead of leaving
  * pools of reserved memory unused.
  *
  * The basic ideas stem from similar slab/zone based allocators whose algorithms
  * are well known.
  *
  */

  /*
   * TODO:
   *	- Improve memory usage for large allocations
   *	- Investigate cache size adjustments
   */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_ddb.h"
#include "opt_param.h"
#include "opt_vm.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/bitset.h>
#include <sys/domainset.h>
#include <sys/eventhandler.h>
#include <sys/kernel.h>
#include <sys/types.h>
#include <sys/limits.h>
#include <sys/queue.h>
#include <sys/malloc.h>
#include <sys/ktr.h>
#include <sys/lock.h>
#include <sys/sysctl.h>
#include <sys/mutex.h>
#include <sys/proc.h>
#include <sys/random.h>
#include <sys/rwlock.h>
#include <sys/sbuf.h>
#include <sys/sched.h>
#include <sys/sleepqueue.h>
#include <sys/smp.h>
#include <sys/smr.h>
#include <sys/taskqueue.h>
#include <sys/vmmeter.h>
#include <sys/counter.h>
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

#include <ddb/ddb.h>

#ifdef DEBUG_MEMGUARD
#include <vm/memguard.h>
#endif

#include <machine/md_var.h>

#include "context.h"
extern internal_hooks global_hooks;

#if 0
void *
startup_alloc(uma_zone_t zone, vm_size_t bytes, int domain, uint8_t *pflag,
    int wait)
{
	*pflag = UMA_SLAB_BOOT;
	char *pmem = global_hooks.allocate(bytes);
	if (pmem)
	{
		memset(pmem, 0, bytes);
	}
	return pmem;
}

void
startup_free(void *mem, vm_size_t bytes)
{
	global_hooks.deallocate(mem);
}

#endif
