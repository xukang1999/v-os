/*-
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 *
 * Copyright (c) 2006 John Baldwin <jhb@FreeBSD.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

 /*
  * Machine independent bits of reader/writer lock implementation.
  */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_ddb.h"
#include "opt_hwpmc_hooks.h"

#include <sys/param.h>
#include <sys/kdb.h>
#include <sys/ktr.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/proc.h>
#include <sys/rwlock.h>
#include <sys/sched.h>
#include <sys/smp.h>
#include <sys/sysctl.h>
#include <sys/systm.h>
#include <sys/turnstile.h>

#include <machine/cpu.h>
#include "context.h"
#if defined(SMP) && !defined(NO_ADAPTIVE_RWLOCKS)
#define	ADAPTIVE_RWLOCKS
#endif

#ifdef HWPMC_HOOKS
#include <sys/pmckern.h>
PMC_SOFT_DECLARE(, , lock, failed);
#endif
extern internal_hooks global_hooks;
#if 0
void 
__rw_rlock_hard(struct rwlock* rw, struct thread* td, uintptr_t v
	LOCK_FILE_LINE_ARG_DEF)
{
	global_hooks.rw_rlock_cb((void*)(rw->lock_object.lo_witness));
}

void
__rw_runlock_hard(struct rwlock* rw, struct thread* td, uintptr_t v
	LOCK_FILE_LINE_ARG_DEF)
{
	global_hooks.rw_runlock_cb((void*)(rw->lock_object.lo_witness));
}

void rw_lock_init(struct lock_object* lock)
{
	char* p = NULL;
	global_hooks.initialize_rw_lock(&p);
	lock->lo_witness = (uintptr_t)p;
}

void rw_lock_destroy(struct lock_object* lock)
{
	global_hooks.uninitialize_rw_lock((void *)lock->lo_witness);
}

void
__rw_wlock_hard(struct rwlock* rw, struct thread* td, uintptr_t v
	LOCK_FILE_LINE_ARG_DEF)
{
	global_hooks.rw_wlock_cb((void*)(rw->lock_object.lo_witness));
}

void
__rw_wunlock_hard(struct rwlock* rw, struct thread* td, uintptr_t v
	LOCK_FILE_LINE_ARG_DEF)
{
	global_hooks.rw_wunlock_cb((void*)(rw->lock_object.lo_witness));
}
#endif