/*
 * Copyright (c) 2013 Patrick Kelsey. All rights reserved.
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
 * Derived in part from libuinet's uinet_host_interface.c.
 */
#if 0
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
/*#include <pthread.h>*/
/*#include <sched.h>*/
#include <time.h>

#include <openssl/rand.h>
/*#include <rte_malloc.h>*/
#endif
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "host_interface.h"
#include "staros/so_errno.h"
#include "context.h"

extern internal_hooks global_hooks;


static struct timespec current_ts;
extern void* ff_mem_get_page();
extern int ff_mem_free_addr(void* p);
void*
ff_mmap(void* addr, uint64_t len, int prot, int flags, int fd, uint64_t offset)
{
    //return rte_malloc("", len, 4096);
    int host_prot;
    int host_flags;

#ifdef FF_USE_PAGE_ARRAY
    if (len == 4096) {
        return ff_mem_get_page();
    }
    else
#endif
    {
        host_prot = 0;
        if ((prot & ff_PROT_READ) == ff_PROT_READ)   host_prot |= PROT_READ;
        if ((prot & ff_PROT_WRITE) == ff_PROT_WRITE) host_prot |= PROT_WRITE;

        host_flags = 0;
        if ((flags & ff_MAP_SHARED) == ff_MAP_SHARED)   host_flags |= MAP_SHARED;
        if ((flags & ff_MAP_PRIVATE) == ff_MAP_PRIVATE) host_flags |= MAP_PRIVATE;
        if ((flags & ff_MAP_ANON) == ff_MAP_ANON)       host_flags |= MAP_ANON;

        void* ret = (global_hooks.mmap(addr, len, host_prot, host_flags, fd, offset));

        return ret;
    }
}

int
ff_munmap(void* addr, uint64_t len)
{
#ifdef FF_USE_PAGE_ARRAY
    if (len == 4096) {
        return ff_mem_free_addr(addr);
    }
#endif
    //rte_free(addr);
    //return 0;
    return (global_hooks.munmap(addr, len));
}



void *
ff_malloc(uint64_t size)
{
    //return rte_malloc("", size, 0);
    return global_hooks.allocate(size);
}


void *
ff_calloc(uint64_t number, uint64_t size)
{
    //return rte_calloc("", number, size, 0);
    return global_hooks.calloc(number, size);
}


void *
ff_realloc(void *p, uint64_t size)
{
    return (global_hooks.reallocate(p, size));
}


void
ff_free(void *p)
{
    global_hooks.deallocate(p);

}

const char *panicstr = NULL;


void
ff_clock_gettime(int id, int64_t *sec, long *nsec)
{


}
uint64_t ff_get_tsc_ns(void)
{
    return 0;
}
uint64_t
ff_clock_gettime_ns(int id)
{
    int64_t sec;
    long nsec;

    ff_clock_gettime(id, &sec, &nsec);

    return ((uint64_t)sec * ff_NSEC_PER_SEC + nsec);
}

void
ff_get_current_time(time_t *sec, long *nsec)
{
    if (sec) {
        *sec = current_ts.tv_sec;
    }

    if (nsec) {
        *nsec = current_ts.tv_nsec;
    }
}

void
ff_update_current_ts()
{

}

void
ff_arc4rand(void *ptr, unsigned int len, int reseed)
{
    (void)reseed;

    context_rand_bytes(ptr, len);
}

uint32_t
ff_arc4random(void)
{
    uint32_t ret;
    ff_arc4rand(&ret, sizeof ret, 0);
    return ret;
}

int ff_setenv(const char *name, const char *value)
{
    return 0;
}

char *ff_getenv(const char *name)
{
    return 0;
}

void ff_os_errno(int error)
{
    global_hooks.set_errno(error);
}

int ff_in_pcbladdr(uint16_t family, void* faddr, uint16_t fport, void* laddr)
{
    return 0;
}

int ff_rss_check(void* softc, uint32_t saddr, uint32_t daddr,
    uint16_t sport, uint16_t dport)
{
    return 0;
}

