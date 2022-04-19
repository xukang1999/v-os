/*
 * Copyright (c) 2010 Kip Macy All rights reserved.
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
 * Derived in part from libplebnet's pn_init.c.
 */

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
#include "host_interface.h"
#include <sys/nlist_aout.h>
#include <sys/link_aout.h>
#include "context.h"

#include "user_sysinit_api.h"
#include "user_sysinit.h"

char version[] = "13.0.0.1";
char compiler_version[] = "13.0.0.1";
int osreldate;

char __build_id_start[1028];
char __build_id_end[1028];
#ifdef _WIN32
struct _dynamic _DYNAMIC;
#else
#if defined(__APPLE__) && defined(__MACH__)
struct _dynamic _DYNAMIC;
#else
/*do nothing*/
#endif
#endif
extern void mutex_init(void);
extern void mi_startup(void);
extern void uma_startup(void *, int);
extern void uma_startup2(void);

extern void vos_init_thread0(void);

struct pcpu cpupcpu[16];
struct pcpu *pcpup=NULL;
struct uma_page_head *uma_page_slab_hash;
int uma_page_mask;
extern cpuset_t all_cpus;
bool lse_supported = false;
u_long physmem;
vm_paddr_t dmap_phys_base;	/* The start of the dmap region */
extern struct	proc proc0;
extern void	uma_startup1(vm_offset_t);

int testscan() {
    int day, year;
    char weekday[20], month[20], dtm[100];

    strcpy(dtm, "Saturday March 25 1989");
    sscanf(dtm, "%s %s %d  %d", weekday, month, &day, &year);

    printf("%s %d, %d = %s\n", month, day, year, weekday);

    return(0);
}

void PrintFError(const char* format, ...)
{
}
char buffer[1024];
char vmem[1024 * 106];
char mem[1024 * 106];
extern vm_offset_t virtual_avail;
extern vm_offset_t virtual_end;
extern u_long realmem;
char boot1_env[4096];
extern vm_paddr_t phys_avail[PHYS_AVAIL_COUNT];
extern vm_paddr_t dump_avail[PHYS_AVAIL_COUNT];
void cninit(void);
extern void init_proc0();
/*
 * Initialize per cpu data structures, include curthread.
 */
void
vos_pcpu0_init()
{
    /* Initialize pcpu info of cpu-zero */
    pcpup = malloc(sizeof(struct pcpu), M_DEVBUF, M_ZERO);
    pcpu_init(pcpup, 0, sizeof(struct pcpu));
    PCPU_SET(prvspace, pcpup);
    CPU_SET(0, &all_cpus);
}

void platform_start()
{
    int dump_pages, phys_pages;
    int i;
    int boot_pages;
    void* bootmem;
    unsigned int num_hash_buckets;
    int memsize = 100 * 1024 * 1024;
    physmem = realmem = btoc(memsize);

    int real = ctob(realmem);

    for (i = 0; i < PHYS_AVAIL_COUNT; i++) {
        phys_avail[i] = 0;
        dump_avail[i] = 0;
    }
    int totalsize = 0;
    char* pstart = NULL;
    printf("realmem=%d, memsize=%d, PHYS_AVAIL_COUNT=%d\n", real, memsize, PHYS_AVAIL_COUNT);
    pstart = context_malloc(memsize);
    printf("pstart=%p\n", pstart);
    phys_avail[0] = pstart;
    phys_avail[1] = pstart + memsize;

    dump_avail[0] = phys_avail[0];
    dump_avail[1] = pstart + memsize;

    totalsize += memsize;
    printf("totalsize=%d, totalsize=%d(M)\n", totalsize,totalsize/(1024*1204));


    dump_pages = 0;
    for (i = 0; dump_avail[i + 1] != 0; i += 2) {
        dump_pages += howmany(dump_avail[i + 1], PAGE_SIZE) -dump_avail[i] / PAGE_SIZE;
    }
    printf("dump_pages : %d\n", dump_pages);


    phys_pages = 0;
    for (i = 0; phys_avail[i + 1] != 0; i += 2) {
        phys_pages += howmany(phys_avail[i + 1], PAGE_SIZE) - phys_avail[i] / PAGE_SIZE;
    }
    printf("phys_pages : %d, PAGE_SIZE=%d, mempage=%d\n", phys_pages, PAGE_SIZE, memsize/ PAGE_SIZE);

    vos_pcpu0_init();

    init_param1();
    cninit();
    init_static_kenv(boot1_env, sizeof(boot1_env));

    printf("pyend=%p\n", pstart + memsize);
    init_param2(physmem);
    /*pmap_bootstrap(&pstart);*/
    msgbufinit(buffer, sizeof(buffer));
    fpuinit();

    init_proc0();

    vos_init_thread0();

    mutex_init();

    /*
     * Set the start and end of kva.
     */
    boot_pages = 16;
    bootmem = (void*)context_malloc(boot_pages * PAGE_SIZE);
    num_hash_buckets = 8192;
    uma_page_slab_hash = (struct uma_page_head*)context_malloc(sizeof(struct uma_page) * num_hash_buckets);
    uma_page_mask = num_hash_buckets - 1;
    virtual_avail = bootmem;
    virtual_end = (vm_offset_t)bootmem + boot_pages * PAGE_SIZE;
    /*uma_startup1((vm_offset_t)bootmem);*/
    /*uma_startup2();*/
    printf("%p, %p\n", bootmem, virtual_avail);

    vmmeter_init(&proc0);
}
int
ff_freebsd_init()
{
    int boot_pages;
    unsigned int num_hash_buckets;
    char tmpbuf[32] = {0};
    void *bootmem;
    int error;

    platform_start();

    printf("vos_freebsd_init start\n");

    boot_pages = 16;
    bootmem = (void *)kmem_malloc(boot_pages*PAGE_SIZE, M_ZERO);
    //uma_startup(bootmem, boot_pages);
#if 0
    uma_startup1((vm_offset_t)bootmem);
    uma_startup2();

    num_hash_buckets = 8192;
    uma_page_slab_hash = (struct uma_page_head *)kmem_malloc(sizeof(struct uma_page)*num_hash_buckets, M_ZERO);
    uma_page_mask = num_hash_buckets - 1;
#endif
    num_hash_buckets = 8192;
    uma_page_slab_hash = (struct uma_page_head *)kmem_malloc(sizeof(struct uma_page)*num_hash_buckets, M_ZERO);
    uma_page_mask = num_hash_buckets - 1;

#if 0
    msgbufinit(buffer, sizeof(buffer));
    vm_mem_init(NULL);

    kmem_init(mem, mem + sizeof(mem));
#endif
    /*vmcounter_startup();*/
    printf("vm_mem_init\n");
    msgbufinit(buffer, sizeof(buffer));

    mallocinit(NULL);
    printf("vm_stats_init\n");
    proc0_init(NULL);

    printf("pcpu_zones_startup\n");
    pcpu_zones_startup();

    printf("pcpu_zones_startup\n");
    vmcounter_startup();

    printf("kmem_init\n");
    kmem_init(mem, mem + sizeof(mem));
    printf("mutex_init\n");
    /*uma_startup1((vm_offset_t)bootmem);*/
    /*uma_startup2();*/

    mutex_init();

    printf("mi_startup\n");



    mi_startup();
    sx_init(&proctree_lock, "proctree");

    printf("vos_freebsd_init end\n");

    return (0);
}
int vos_freebsd_init(void)
{
    char* ret = NULL;
    char* ret2 = NULL;

    char tmpbuf[32] = { 0 };
    int error;
    printf("==================vos_freebsd_init start====================\n");
    platform_start();
    InitializeObjects();

    mi_startup();

    uma_zone_t test_zone = uma_zcreate("test_zone", 1024, NULL, NULL, NULL, NULL,
        UMA_ALIGN_PTR, 0);
    uma_zdestroy(test_zone);
    printf("uma_zcreate\n");
    static int count=0;
    for (;;)
    {
        test_zone = uma_zcreate("test_zone", 1024, NULL, NULL, NULL, NULL,
            UMA_ALIGN_PTR, 0);
        ret = uma_zalloc(test_zone, M_WAITOK | M_ZERO);
        printf("ret=%p, count=%d\n", ret, count);
        memset(ret, 0, 1024);
        printf("uma_zcreate end\n");
        uma_zfree(test_zone, ret);
        uma_zdestroy(test_zone);
        count++;
        break;
    }
    uma_zone_t test_zone2 = uma_zcreate("test_zone2", 64, NULL, NULL, NULL, NULL,
        UMA_ALIGN_PTR, 0);
    ret = uma_zalloc(test_zone2, M_WAITOK | M_ZERO);
    memset(ret, 0, 64);
    uma_zfree(test_zone2, ret);
    ret = NULL;


    printf("==================vos_freebsd_init end====================\n");
    return 0;
}
int
vos_freebsd_init2(void)
{
    int boot_pages;
    unsigned int num_hash_buckets;
    char tmpbuf[32] = { 0 };
    void* bootmem;
    int error;
    printf("==================vos_freebsd_init start====================\n");

    platform_start();
    InitializeObjects();

    vos_init_thread0();

    boot_pages = 16;
    bootmem = (void*)kmem_malloc(boot_pages * PAGE_SIZE, M_ZERO);
    //uma_startup(bootmem, boot_pages);
#if 0
    uma_startup1((vm_offset_t)bootmem);
    uma_startup2();

    num_hash_buckets = 8192;
    uma_page_slab_hash = (struct uma_page_head*)kmem_malloc(sizeof(struct uma_page) * num_hash_buckets, M_ZERO);
    uma_page_mask = num_hash_buckets - 1;
#endif
    num_hash_buckets = 8192;
    uma_page_slab_hash = (struct uma_page_head*)kmem_malloc(sizeof(struct uma_page) * num_hash_buckets, M_ZERO);
    uma_page_mask = num_hash_buckets - 1;

    uma_startup1((vm_offset_t)bootmem);
    uma_startup2();

    mutex_init();
    mi_startup();

    proc0_init(NULL);


    sx_init(&proctree_lock, "proctree");

    printf("=====================vos_freebsd_init end===================\n");
    return (0);
}