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

#include <ck_epoch.h>
#include <ck_stack.h>

#include "host_interface.h"
#include "context.h"
void (*wdog_software_attach)(void);

#ifndef MAXMEMDOM
#define MAXMEMDOM 1
#endif

struct domainset  domainset_fixed[MAXMEMDOM];
struct domainset  domainset_prefer[MAXMEMDOM];
struct domainset  domainset_roundrobin;

domainset_t  vm_min_domains;

int bootverbose;

static void configure_final(void *dummy);

SYSINIT(configure3, SI_SUB_CONFIGURE, SI_ORDER_ANY, configure_final, NULL);

volatile int ticks;
int cpu_disable_deep_sleep;

static int sysctl_kern_smp_active(SYSCTL_HANDLER_ARGS);

/* This is used in modules that need to work in both SMP and UP. */
cpuset_t all_cpus;

int mp_ncpus = 1;

volatile int smp_started;
u_int mp_maxid;

static SYSCTL_NODE(_kern, OID_AUTO, smp, CTLFLAG_RD|CTLFLAG_CAPRD, NULL,
    "Kernel SMP");

SYSCTL_INT(_kern_smp, OID_AUTO, maxid, CTLFLAG_RD|CTLFLAG_CAPRD, &mp_maxid, 0,
    "Max CPU ID.");

SYSCTL_INT(_kern_smp, OID_AUTO, maxcpus, CTLFLAG_RD|CTLFLAG_CAPRD, &mp_maxcpus,
    0, "Max number of CPUs that the system was compiled for.");

SYSCTL_PROC(_kern_smp, OID_AUTO, active, CTLFLAG_RD | CTLTYPE_INT, NULL, 0,
    sysctl_kern_smp_active, "I", "Indicates system is running in SMP mode");


SYSCTL_DEFINE(_vfs_devfs);

u_int vn_lock_pair_pause_max = 1; // ff_global_cfg.freebsd.hz / 100;

long long first_page = 0;

vm_map_t kernel_map = 0;
vm_map_t kmem_map = 0;


struct vm_object kernel_object_store;
struct vm_object kmem_object_store;

struct filterops fs_filtops;
struct filterops sig_filtops;

extern int cold;


int cpu_deepest_sleep = 0;    /* Deepest Cx state available. */


static void timevalfix(struct timeval *);

char	ostype[]="unknow";
char	kern_ident[]="vos";
char	osrelease[] = "123";
char static_env[]="123";
char static_hints[] = "hints";
/* Character device of /dev/console. */
static struct cdev* dev_console;
static const char* dev_console_filename;

/* Extra care is taken with this sysctl because the data type is volatile */
static int
sysctl_kern_smp_active(SYSCTL_HANDLER_ARGS)
{
    int error, active;

    active = smp_started;
    error = SYSCTL_OUT(req, &active, sizeof(active));
    return (error);
}


#if 0
int
jailed(struct ucred *cred)
{
    return (0);
}
#endif

int
copyin(const void *uaddr, void *kaddr, size_t len)
{
    memcpy(kaddr, uaddr, len);
    return (0);
}

int
copyout(const void *kaddr, void *uaddr, size_t len)
{
    memcpy(uaddr, kaddr, len);
    return (0);
}

#if 0
int
copystr(const void *kfaddr, void *kdaddr, size_t len, size_t *done)
{
    size_t bytes;

    bytes = strlcpy(kdaddr, kfaddr, len);
    if (done != NULL)
        *done = bytes;

    return (0);
}
#endif

int
copyinstr(const void *uaddr, void *kaddr, size_t len, size_t *done)
{    
    size_t bytes;

    bytes = strlcpy(kaddr, uaddr, len);
    if (done != NULL)
        *done = bytes;

    return (0);
}

int
subyte(volatile void *base, int byte)
{
    *(volatile char *)base = (uint8_t)byte;
    return (0);
}


#if 0
struct proc *
zpfind(pid_t pid)
{
    return (NULL);
}
#endif


int
cr_canseeinpcb(struct ucred *cred, struct inpcb *inp)
{
    return (0);
}


static void
configure_final(void *dummy)
{
    cold = 0;
}


void
DELAY(int delay)
{
#if 0
    struct timespec rqt;

    if (delay < 1000)
        return;
    
    rqt.tv_nsec = 1000*((unsigned long long)delay);
    rqt.tv_sec = 0;
    /*
     * FIXME: We shouldn't sleep in dpdk apps.
     */
    nanosleep(&rqt, NULL);
#endif
    context_usleep(delay);
}


#if 0
void
sf_ext_free(void *arg1, void *arg2)
{
    panic("sf_ext_free not implemented.\n");
}

void
sf_ext_free_nocache(void *arg1, void *arg2)
{
    panic("sf_ext_free_nocache not implemented.\n");
}
#endif

#if 0
void
wakeup_any(const void *ident)
{

}
#endif


#if 0
int
elf_cpu_parse_dynamic(caddr_t loadbase __unused, Elf_Dyn *dynamic __unused)
{
    return (0);
}
#endif


void *
memset_early(void *buf, int c, size_t len)
{
    return (memset(buf, c, len));
}

int
elf_reloc_late(linker_file_t lf, Elf_Addr relocbase, const void *data,
    int type, elf_lookup_fn lookup)
{
    return (0);
}

bool
elf_is_ifunc_reloc(Elf_Size r_info) 
{
    return (true);
}


static void
softdep_ast_cleanup_proc(struct thread* td)
{

}


void cpu_throw(struct thread* t1, struct thread* t2)
{

}
void context_init_thread(void* p)
{
    struct thread* newtd = (struct thread*)p;
    pcurthread = newtd;
}
void cpu_switch(struct thread* oldtd, struct thread* newtd, struct mtx* tx)
{
    void (*func)(void*);
    void* arg;
    func= (void (*)(void*))newtd->td_pcb->pcb_r12;	/* function */
    arg=newtd->td_pcb->pcb_rbx;	/* first arg */
}

void __membar()
{
    context_mb();
}


int
suword32(volatile void* base, int32_t word)
{
    return -1;
}
void
g_waitidle(void)
{

}
int
fueword32(volatile const void* base, int32_t* val)
{
    return 0;
}
int	fueword64(volatile const void* base, int64_t* val)
{
    return 0;
}
int
casueword32(volatile uint32_t* base, uint32_t oldval, uint32_t* oldvalp,
    uint32_t newval)
{
    return (-1);
}

int
casueword(volatile u_long* base, u_long oldval, u_long* oldvalp, u_long newval)
{
    return (-1);
}

int
suword(volatile void* base, long long word)
{
    return -1;
}

void
shmfork(struct proc* p1, struct proc* p2)
{

}


void	clear_pcb_flags(struct pcb* pcb, const u_int flags)
{

}

void	set_pcb_flags(struct pcb* pcb, const u_int flags) {

}
void	set_pcb_flags_raw(struct pcb* pcb, const u_int flags) {

}

void
cpu_lock_delay(void)
{
    DELAY(1);
}

int read_random_uio(struct uio* a, bool b)
{
    return -1;
}
bool is_random_seeded(void)
{
    return false;
}
void
shmexit(struct vmspace* vm)
{

}


int
uiomove_fromphys(vm_page_t ma[], vm_offset_t offset, int n, struct uio* uio)
{
    return -1;
}
void	fork_trampoline(void)
{

}
void
g_dev_print(void)
{
}

void
devfs_free(struct cdev* cdev)
{
}
struct cdev*
    devfs_alloc(int flags)
{
    return NULL;
}


ino_t
devfs_alloc_cdp_inode(void)
{
    return 0;
}

void
devfs_free_cdp_inode(ino_t ino)
{

}

void
devfs_devs_init(void* junk __unused)
{
}

int
wdog_kern_pat(u_int utim)
{
    return -1;
}


void
inittodr(time_t base)
{
}


#if 0
int
xos_sleep(const void* _Nonnull chan, struct lock_object* lock, int priority,
    const char* wmesg, sbintime_t sbt, sbintime_t pr, int flags)
{
    printf("xos_sleep %d\n", sbt);
    context_mssleep(sbt);
    return 0;
}
#endif
void lgdt(struct region_descriptor* rdp)
{

}


void	amd64_bsp_pcpu_init1(struct pcpu* pc)
{

}
void	amd64_bsp_pcpu_init2(uint64_t rsp0)
{

}
void	amd64_bsp_ist_init(struct pcpu* pc)
{

}


struct bio*
    g_alloc_bio(void)
{
    return NULL;
}

void
g_destroy_bio(struct bio* bp)
{

}

void kasan_init(void)
{

}
void kasan_shadow_map(vm_offset_t a, size_t b)
{

}

void kasan_mark(const void* a, size_t b, size_t c, uint8_t d)
{

}

void
stack_save(struct stack* st)
{

}

void
lapic_calibrate_timer(void)
{
}
int	devfs_dev_exists(const char* a)
{
    return -1;
}

void
uma_reclaim_domain(int req, int domain)
{

}

int
uma_zone_reserve_kva(uma_zone_t zone, int count)
{
    return 0;
}