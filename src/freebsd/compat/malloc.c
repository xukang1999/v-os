/*
 * Copyright (c) 2010 Kip Macy. All rights reserved.
 * Copyright (C) 2017-2021 THL A29 Limited, a Tencent company.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Derived in part from libplebnet's pn_glue.c.
 */

#include <sys/cdefs.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/kernel.h>
#include <sys/kthread.h>
#include <sys/event.h>
#include <sys/jail.h>
#include <sys/limits.h>
#include <sys/malloc.h>
#include <sys/refcount.h>
#include <sys/resourcevar.h>
#include <sys/sysctl.h>
#include <sys/sysent.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/priv.h>
#include <sys/time.h>
#include <sys/ucred.h>
#include <sys/uio.h>
#include <sys/param.h>
#include <sys/bus.h>
#include <sys/buf.h>
#include <sys/file.h>
#include <sys/vmem.h>
#include <sys/mbuf.h>
#include <sys/smp.h>
#include <sys/sched.h>
#include <sys/vmmeter.h>
#include <sys/unpcb.h>
#include <sys/eventfd.h>
#include <sys/linker.h>
#include <sys/sleepqueue.h>

#include <vm/vm.h>
#include <vm/vm_param.h>
#include <vm/pmap.h>
#include <vm/vm_object.h>
#include <vm/vm_map.h>
#include <vm/vm_extern.h>
#include <vm/vm_domainset.h>
#include <vm/vm_page.h>
#include <vm/vm_pagequeue.h>

#include <netinet/in_systm.h>
#include "context.h"
extern internal_hooks global_hooks;

void
malloc_init(void *data)
{
    /* Nothing to do here */ 
}


void
malloc_uninit(void *data)
{
    /* Nothing to do here */ 
}

char *
malloc(unsigned long size, struct malloc_type *type, int flags)
{
    void *alloc;

    do {
        alloc = global_hooks.allocate(size);
        if (alloc || !(flags & M_WAITOK))
            break;

        pause("malloc", hz/100);
    } while (alloc == NULL);

    if ((flags & M_ZERO) && alloc != NULL)
        bzero(alloc, size);
    return (alloc);
}

void
free(void *addr, struct malloc_type *type)
{
    global_hooks.deallocate(addr);
}

void *
realloc(void *addr, unsigned long size, struct malloc_type *type,
    int flags)
{
    return (global_hooks.reallocate(addr, size));
}

void *
reallocf(void *addr, unsigned long size, struct malloc_type *type,
     int flags)
{
    void *mem;

    if ((mem = global_hooks.reallocate(addr, size)) == NULL)
        global_hooks.deallocate(addr);

    return (mem);
}
