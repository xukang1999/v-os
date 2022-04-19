/*-
 * SPDX-License-Identifier: BSD-4-Clause
 *
 * Copyright (c) 2003 Peter Wemm.
 * Copyright (c) 1992 Terrence R. Lambert.
 * Copyright (c) 1982, 1987, 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * William Jolitz.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	from: @(#)machdep.c	7.4 (Berkeley) 6/3/91
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");


#include "opt_ddb.h"
#include "opt_inet.h"

#include "opt_sched.h"

#include <sys/param.h>
#include <sys/proc.h>
#include <sys/systm.h>

#include <sys/bio.h>
#include <sys/buf.h>
#include <sys/bus.h>
#include <sys/callout.h>
#include <sys/cons.h>
#include <sys/cpu.h>
#include <sys/csan.h>
#include <sys/efi.h>
#include <sys/eventhandler.h>
#include <sys/exec.h>
#include <sys/imgact.h>
#include <sys/kdb.h>
#include <sys/kernel.h>
#include <sys/ktr.h>
#include <sys/linker.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/memrange.h>
#include <sys/msgbuf.h>
#include <sys/mutex.h>
#include <sys/pcpu.h>
#include <sys/ptrace.h>
#include <sys/reboot.h>
#include <sys/rwlock.h>
#include <sys/sched.h>
#include <sys/signalvar.h>
#ifdef SMP
#include <sys/smp.h>
#endif
#include <sys/syscallsubr.h>
#include <sys/sysctl.h>
#include <sys/sysent.h>
#include <sys/sysproto.h>
#include <sys/ucontext.h>
#include <sys/vmmeter.h>

#include <vm/vm.h>
#include <vm/vm_param.h>
#include <vm/vm_extern.h>
#include <vm/vm_kern.h>
#include <vm/vm_page.h>
#include <vm/vm_map.h>
#include <vm/vm_object.h>
#include <vm/vm_pager.h>
#include <vm/vm_phys.h>
#include <vm/vm_dumpset.h>

#ifdef DDB
#ifndef KDB
#error KDB must be enabled in order for DDB to work!
#endif
#include <ddb/ddb.h>
#include <ddb/db_sym.h>
#endif

#include <net/netisr.h>

#include <machine/clock.h>
#include <machine/cpu.h>
#include <machine/cputypes.h>
#include <machine/frame.h>
#include <machine/intr_machdep.h>
#include <x86/mca.h>
#include <machine/md_var.h>
#include <machine/metadata.h>
#include <machine/mp_watchdog.h>
#include <machine/pc/bios.h>
#include <machine/pcb.h>
#include <machine/proc.h>
#include <machine/reg.h>
#include <machine/sigframe.h>
#include <machine/specialreg.h>
#include <machine/trap.h>
#include <machine/tss.h>
#include <x86/ucode.h>
#include <x86/ifunc.h>
#ifdef SMP
#include <machine/smp.h>
#endif
#ifdef FDT
#include <x86/fdt.h>
#endif

#ifdef DEV_ATPIC
#include <x86/isa/icu.h>
#else
#include <x86/apicvar.h>
#endif

#include <x86/init.h>

#include "context.h"

u_int	cpu_feature;
u_long* intrcnt;
char* intrnames;
size_t sintrcnt;
size_t sintrnames;

/* Sanity check for __curthread() */
CTASSERT(offsetof(struct pcpu, pc_curthread) == 0);

/*
 * The PTI trampoline stack needs enough space for a hardware trapframe and a
 * couple of scratch registers, as well as the trapframe left behind after an
 * iret fault.
 */
CTASSERT(PC_PTI_STACK_SZ * sizeof(register_t) >= 2 * sizeof(struct pti_frame) -
	offsetof(struct pti_frame, pti_rip));

extern u_int64_t hammer_time(u_int64_t, u_int64_t);

static void cpu_startup(void*);
SYSINIT(cpu, SI_SUB_CPU, SI_ORDER_FIRST, cpu_startup, NULL);

/* Preload data parse function */
static caddr_t native_parse_preload_data(u_int64_t);

/* Native function to fetch and parse the e820 map */
static void native_parse_memmap(caddr_t, vm_paddr_t*, int*);

/* Default init_ops implementation. */
struct init_ops init_ops = {
	.parse_preload_data = native_parse_preload_data,
	.early_clock_source_init = NULL,
	.early_delay = NULL,
	.parse_memmap = native_parse_memmap,
#ifdef SMP
	.start_all_aps = native_start_all_aps,
#endif
#ifdef DEV_PCI
	.msi_init = msi_init,
#endif
};
struct soft_segment_descriptor gdt_segs[] = {
	/* GNULL_SEL	0 Null Descriptor */
	{.ssd_base = 0x0,
		.ssd_limit = 0x0,
		.ssd_type = 0,
		.ssd_dpl = 0,
		.ssd_p = 0,
		.ssd_long = 0,
		.ssd_def32 = 0,
		.ssd_gran = 0		},
		/* GNULL2_SEL	1 Null Descriptor */
		{.ssd_base = 0x0,
			.ssd_limit = 0x0,
			.ssd_type = 0,
			.ssd_dpl = 0,
			.ssd_p = 0,
			.ssd_long = 0,
			.ssd_def32 = 0,
			.ssd_gran = 0		},
			/* GUFS32_SEL	2 32 bit %gs Descriptor for user */
			{.ssd_base = 0x0,
				.ssd_limit = 0xfffff,
				.ssd_type = SDT_MEMRWA,
				.ssd_dpl = SEL_UPL,
				.ssd_p = 1,
				.ssd_long = 0,
				.ssd_def32 = 1,
				.ssd_gran = 1		},
				/* GUGS32_SEL	3 32 bit %fs Descriptor for user */
				{.ssd_base = 0x0,
					.ssd_limit = 0xfffff,
					.ssd_type = SDT_MEMRWA,
					.ssd_dpl = SEL_UPL,
					.ssd_p = 1,
					.ssd_long = 0,
					.ssd_def32 = 1,
					.ssd_gran = 1		},
					/* GCODE_SEL	4 Code Descriptor for kernel */
					{.ssd_base = 0x0,
						.ssd_limit = 0xfffff,
						.ssd_type = SDT_MEMERA,
						.ssd_dpl = SEL_KPL,
						.ssd_p = 1,
						.ssd_long = 1,
						.ssd_def32 = 0,
						.ssd_gran = 1		},
						/* GDATA_SEL	5 Data Descriptor for kernel */
						{.ssd_base = 0x0,
							.ssd_limit = 0xfffff,
							.ssd_type = SDT_MEMRWA,
							.ssd_dpl = SEL_KPL,
							.ssd_p = 1,
							.ssd_long = 1,
							.ssd_def32 = 0,
							.ssd_gran = 1		},
							/* GUCODE32_SEL	6 32 bit Code Descriptor for user */
							{.ssd_base = 0x0,
								.ssd_limit = 0xfffff,
								.ssd_type = SDT_MEMERA,
								.ssd_dpl = SEL_UPL,
								.ssd_p = 1,
								.ssd_long = 0,
								.ssd_def32 = 1,
								.ssd_gran = 1		},
								/* GUDATA_SEL	7 32/64 bit Data Descriptor for user */
								{.ssd_base = 0x0,
									.ssd_limit = 0xfffff,
									.ssd_type = SDT_MEMRWA,
									.ssd_dpl = SEL_UPL,
									.ssd_p = 1,
									.ssd_long = 0,
									.ssd_def32 = 1,
									.ssd_gran = 1		},
									/* GUCODE_SEL	8 64 bit Code Descriptor for user */
									{.ssd_base = 0x0,
										.ssd_limit = 0xfffff,
										.ssd_type = SDT_MEMERA,
										.ssd_dpl = SEL_UPL,
										.ssd_p = 1,
										.ssd_long = 1,
										.ssd_def32 = 0,
										.ssd_gran = 1		},
										/* GPROC0_SEL	9 Proc 0 Tss Descriptor */
										{.ssd_base = 0x0,
											.ssd_limit = sizeof(struct amd64tss) + IOPERM_BITMAP_SIZE - 1,
											.ssd_type = SDT_SYSTSS,
											.ssd_dpl = SEL_KPL,
											.ssd_p = 1,
											.ssd_long = 0,
											.ssd_def32 = 0,
											.ssd_gran = 0		},
											/* Actually, the TSS is a system descriptor which is double size */
											{.ssd_base = 0x0,
												.ssd_limit = 0x0,
												.ssd_type = 0,
												.ssd_dpl = 0,
												.ssd_p = 0,
												.ssd_long = 0,
												.ssd_def32 = 0,
												.ssd_gran = 0		},
												/* GUSERLDT_SEL	11 LDT Descriptor */
												{.ssd_base = 0x0,
													.ssd_limit = 0x0,
													.ssd_type = 0,
													.ssd_dpl = 0,
													.ssd_p = 0,
													.ssd_long = 0,
													.ssd_def32 = 0,
													.ssd_gran = 0		},
													/* GUSERLDT_SEL	12 LDT Descriptor, double size */
													{.ssd_base = 0x0,
														.ssd_limit = 0x0,
														.ssd_type = 0,
														.ssd_dpl = 0,
														.ssd_p = 0,
														.ssd_long = 0,
														.ssd_def32 = 0,
														.ssd_gran = 0		},
};
_Static_assert(nitems(gdt_segs) == NGDT, "Stale NGDT");
/*
 * Physical address of the EFI System Table. Stashed from the metadata hints
 * passed into the kernel and used by the EFI code to call runtime services.
 */
vm_paddr_t efi_systbl_phys;

/* Intel ICH registers */
#define ICH_PMBASE	0x400
#define ICH_SMI_EN	ICH_PMBASE + 0x30

int	_udatasel, _ucodesel, _ucode32sel, _ufssel, _ugssel;

int cold = 1;

u_long Maxmem = 0;
u_long realmem = 0;

struct kva_md_info kmi;

static struct trapframe proc0_tf;
struct region_descriptor r_idt;

struct pcpu* __pcpu;
struct pcpu temp_bsp_pcpu;

struct mtx icu_lock;

struct mem_range_softc mem_range_softc;

struct mtx dt_lock;	/* lock for GDT and LDT */

void (*vmm_resume_p)(void);

bool efi_boot;

static void
cpu_startup(dummy)
void* dummy;
{
	uintmax_t memsize;
	char* sysenv;
#ifdef VOS_KERNEL
	/*return;*/
#endif
	/*
	 * On MacBooks, we need to disallow the legacy USB circuit to
	 * generate an SMI# because this can cause several problems,
	 * namely: incorrect CPU frequency detection and failure to
	 * start the APs.
	 * We do this by disabling a bit in the SMI_EN (SMI Control and
	 * Enable register) of the Intel ICH LPC Interface Bridge.
	 */
	sysenv = kern_getenv("smbios.system.product");
	if (sysenv != NULL) {
		if (strncmp(sysenv, "MacBook1,1", 10) == 0 ||
			strncmp(sysenv, "MacBook3,1", 10) == 0 ||
			strncmp(sysenv, "MacBook4,1", 10) == 0 ||
			strncmp(sysenv, "MacBookPro1,1", 13) == 0 ||
			strncmp(sysenv, "MacBookPro1,2", 13) == 0 ||
			strncmp(sysenv, "MacBookPro3,1", 13) == 0 ||
			strncmp(sysenv, "MacBookPro4,1", 13) == 0 ||
			strncmp(sysenv, "Macmini1,1", 10) == 0) {
			if (bootverbose)
				printf("Disabling LEGACY_USB_EN bit on "
					"Intel ICH.\n");
			outl(ICH_SMI_EN, inl(ICH_SMI_EN) & ~0x8);
		}
		freeenv(sysenv);
	}

	/*
	 * Display physical memory if SMBIOS reports reasonable amount.
	 */
	memsize = 0;
	sysenv = kern_getenv("smbios.memory.enabled");
	if (sysenv != NULL) {
		memsize = (uintmax_t)strtoul(sysenv, (char**)NULL, 10) << 10;
		freeenv(sysenv);
	}
	if (memsize < ptoa((uintmax_t)vm_free_count()))
		memsize = ptoa((uintmax_t)Maxmem);
	printf("real memory  = %ju (%ju MB)\n", memsize, memsize >> 20);
	realmem = atop(memsize);

	/*
	 * Display any holes after the first chunk of extended memory.
	 */
	if (bootverbose) {
		int indx;

		printf("Physical memory chunk(s):\n");
		for (indx = 0; phys_avail[indx + 1] != 0; indx += 2) {
			vm_paddr_t size;

			size = phys_avail[indx + 1] - phys_avail[indx];
			printf(
				"0x%016jx - 0x%016jx, %ju bytes (%ju pages)\n",
				(uintmax_t)phys_avail[indx],
				(uintmax_t)phys_avail[indx + 1] - 1,
				(uintmax_t)size, (uintmax_t)size / PAGE_SIZE);
		}
	}

	vm_ksubmap_init(&kmi);

	printf("avail memory = %ju (%ju MB)\n",
		ptoa((uintmax_t)vm_free_count()),
		ptoa((uintmax_t)vm_free_count()) / 1048576);
#ifdef DEV_PCI
	if (bootverbose && intel_graphics_stolen_base != 0)
		printf("intel stolen mem: base %#jx size %ju MB\n",
			(uintmax_t)intel_graphics_stolen_base,
			(uintmax_t)intel_graphics_stolen_size / 1024 / 1024);
#endif

	/*
	 * Set up buffers, so they can be used to read disk labels.
	 */
	/*bufinit();*/
	vm_pager_bufferinit();

	cpu_setregs();
}

static void
late_ifunc_resolve(void* dummy __unused)
{

}
SYSINIT(late_ifunc_resolve, SI_SUB_CPU, SI_ORDER_ANY, late_ifunc_resolve, NULL);


void
cpu_setregs(void)
{

}

/*
 * Initialize amd64 and configure to run kernel
 */

 /*
  * Initialize segments & interrupt table
  */
static struct gate_descriptor idt0[NIDT];
struct gate_descriptor* idt = &idt0[0];	/* interrupt descriptor table */

static __begin_aligned(16) char dblfault_stack[DBLFAULT_STACK_SIZE] __aligned(16);
static __begin_aligned(16) char mce0_stack[MCE_STACK_SIZE] __aligned(16);
static __begin_aligned(16) char nmi0_stack[NMI_STACK_SIZE] __aligned(16);
static __begin_aligned(16) char dbg0_stack[DBG_STACK_SIZE] __aligned(16);
CTASSERT(sizeof(struct nmi_pcpu) == 16);


void
setidt(int idx, inthand_t* func, int typ, int dpl, int ist)
{

}

u_long basemem;

static int
add_physmap_entry(uint64_t base, uint64_t length, vm_paddr_t* physmap,
	int* physmap_idxp)
{
	int i, insert_idx, physmap_idx;

	physmap_idx = *physmap_idxp;

	if (length == 0)
		return (1);

	/*
	 * Find insertion point while checking for overlap.  Start off by
	 * assuming the new entry will be added to the end.
	 *
	 * NB: physmap_idx points to the next free slot.
	 */
	insert_idx = physmap_idx;
	for (i = 0; i <= physmap_idx; i += 2) {
		if (base < physmap[i + 1]) {
			if (base + length <= physmap[i]) {
				insert_idx = i;
				break;
			}
			if (boothowto & RB_VERBOSE)
				printf(
					"Overlapping memory regions, ignoring second region\n");
			return (1);
		}
	}

	/* See if we can prepend to the next entry. */
	if (insert_idx <= physmap_idx && base + length == physmap[insert_idx]) {
		physmap[insert_idx] = base;
		return (1);
	}

	/* See if we can append to the previous entry. */
	if (insert_idx > 0 && base == physmap[insert_idx - 1]) {
		physmap[insert_idx - 1] += length;
		return (1);
	}

	physmap_idx += 2;
	*physmap_idxp = physmap_idx;
	if (physmap_idx == PHYS_AVAIL_ENTRIES) {
		printf(
			"Too many segments in the physical address map, giving up\n");
		return (0);
	}

	/*
	 * Move the last 'N' entries down to make room for the new
	 * entry if needed.
	 */
	for (i = (physmap_idx - 2); i > insert_idx; i -= 2) {
		physmap[i] = physmap[i - 2];
		physmap[i + 1] = physmap[i - 1];
	}

	/* Insert the new entry. */
	physmap[insert_idx] = base;
	physmap[insert_idx + 1] = base + length;
	return (1);
}

void
bios_add_smap_entries(struct bios_smap* smapbase, u_int32_t smapsize,
	vm_paddr_t* physmap, int* physmap_idx)
{
	struct bios_smap* smap, * smapend;

	smapend = (struct bios_smap*)((uintptr_t)smapbase + smapsize);

	for (smap = smapbase; smap < smapend; smap++) {
		if (boothowto & RB_VERBOSE)
			printf("SMAP type=%02x base=%016lx len=%016lx\n",
				smap->type, smap->base, smap->length);

		if (smap->type != SMAP_TYPE_MEMORY)
			continue;

		if (!add_physmap_entry(smap->base, smap->length, physmap,
			physmap_idx))
			break;
	}
}

static void
add_efi_map_entries(struct efi_map_header* efihdr, vm_paddr_t* physmap,
	int* physmap_idx)
{

}

static char bootmethod[16] = "";
SYSCTL_STRING(_machdep, OID_AUTO, bootmethod, CTLFLAG_RD, bootmethod, 0,
	"System firmware boot method");

static void
native_parse_memmap(caddr_t kmdp, vm_paddr_t* physmap, int* physmap_idx)
{

}

#define	PAGES_PER_GB	(1024 * 1024 * 1024 / PAGE_SIZE)

/*
 * Populate the (physmap) array with base/bound pairs describing the
 * available physical memory in the system, then test this memory and
 * build the phys_avail array describing the actually-available memory.
 *
 * Total memory size may be set by the kernel environment variable
 * hw.physmem or the compile-time define MAXMEM.
 *
 * XXX first should be vm_paddr_t.
 */
static void
getmemsize(caddr_t kmdp, u_int64_t first)
{
	int i, physmap_idx, pa_indx, da_indx;
	vm_paddr_t pa, physmap[PHYS_AVAIL_ENTRIES];
	u_long physmem_start, physmem_tunable, memtest;
	pt_entry_t* pte;
	quad_t dcons_addr, dcons_size;
	int page_counter;

	/*
	 * Tell the physical memory allocator about pages used to store
	 * the kernel and preloaded data.  See kmem_bootstrap_free().
	 */
	vm_phys_early_add_seg((vm_paddr_t)kernphys, trunc_page(first));

	bzero(physmap, sizeof(physmap));
	physmap_idx = 0;

	init_ops.parse_memmap(kmdp, physmap, &physmap_idx);
	physmap_idx -= 2;

	/*
	 * Find the 'base memory' segment for SMP
	 */
	basemem = 0;
	for (i = 0; i <= physmap_idx; i += 2) {
		if (physmap[i] <= 0xA0000) {
			basemem = physmap[i + 1] / 1024;
			break;
		}
	}
	if (basemem == 0 || basemem > 640) {
		if (bootverbose)
			printf(
				"Memory map doesn't contain a basemem segment, faking it");
		basemem = 640;
	}

	/*
	 * Maxmem isn't the "maximum memory", it's one larger than the
	 * highest page of the physical address space.  It should be
	 * called something like "Maxphyspage".  We may adjust this
	 * based on ``hw.physmem'' and the results of the memory test.
	 */
	Maxmem = atop(physmap[physmap_idx + 1]);

#ifdef MAXMEM
	Maxmem = MAXMEM / 4;
#endif

	if (TUNABLE_ULONG_FETCH("hw.physmem", &physmem_tunable))
		Maxmem = atop(physmem_tunable);

	/*
	 * The boot memory test is disabled by default, as it takes a
	 * significant amount of time on large-memory systems, and is
	 * unfriendly to virtual machines as it unnecessarily touches all
	 * pages.
	 *
	 * A general name is used as the code may be extended to support
	 * additional tests beyond the current "page present" test.
	 */
	memtest = 0;
	TUNABLE_ULONG_FETCH("hw.memtest.tests", &memtest);

	/*
	 * Don't allow MAXMEM or hw.physmem to extend the amount of memory
	 * in the system.
	 */
	if (Maxmem > atop(physmap[physmap_idx + 1]))
		Maxmem = atop(physmap[physmap_idx + 1]);

	if (atop(physmap[physmap_idx + 1]) != Maxmem &&
		(boothowto & RB_VERBOSE))
		printf("Physical memory use set to %ldK\n", Maxmem * 4);

	/* call pmap initialization to make new kernel address space */
	pmap_bootstrap(&first);

	/*
	 * Size up each available chunk of physical memory.
	 *
	 * XXX Some BIOSes corrupt low 64KB between suspend and resume.
	 * By default, mask off the first 16 pages unless we appear to be
	 * running in a VM.
	 */
	physmem_start = (vm_guest > VM_GUEST_NO ? 1 : 16) << PAGE_SHIFT;
	TUNABLE_ULONG_FETCH("hw.physmem.start", &physmem_start);
	if (physmap[0] < physmem_start) {
		if (physmem_start < PAGE_SIZE)
			physmap[0] = PAGE_SIZE;
		else if (physmem_start >= physmap[1])
			physmap[0] = round_page(physmap[1] - PAGE_SIZE);
		else
			physmap[0] = round_page(physmem_start);
	}
	pa_indx = 0;
	da_indx = 1;
	phys_avail[pa_indx++] = physmap[0];
	phys_avail[pa_indx] = physmap[0];
	dump_avail[da_indx] = physmap[0];
	pte = CMAP1;

	/*
	 * Get dcons buffer address
	 */
	if (getenv_quad("dcons.addr", &dcons_addr) == 0 ||
		getenv_quad("dcons.size", &dcons_size) == 0)
		dcons_addr = 0;

	/*
	 * physmap is in bytes, so when converting to page boundaries,
	 * round up the start address and round down the end address.
	 */
	page_counter = 0;
	if (memtest != 0)
		printf("Testing system memory");
	for (i = 0; i <= physmap_idx; i += 2) {
		vm_paddr_t end;

		end = ptoa((vm_paddr_t)Maxmem);
		if (physmap[i + 1] < end)
			end = trunc_page(physmap[i + 1]);
		for (pa = round_page(physmap[i]); pa < end; pa += PAGE_SIZE) {
			int tmp, page_bad, full;
			int* ptr = (int*)CADDR1;

			full = FALSE;
			/*
			 * block out kernel memory as not available.
			 */
			if (pa >= (vm_paddr_t)kernphys && pa < first)
				goto do_dump_avail;

			/*
			 * block out dcons buffer
			 */
			if (dcons_addr > 0
				&& pa >= trunc_page(dcons_addr)
				&& pa < dcons_addr + dcons_size)
				goto do_dump_avail;

			page_bad = FALSE;
			if (memtest == 0)
				goto skip_memtest;

			/*
			 * Print a "." every GB to show we're making
			 * progress.
			 */
			page_counter++;
			if ((page_counter % PAGES_PER_GB) == 0)
				printf(".");

			/*
			 * map page into kernel: valid, read/write,non-cacheable
			 */
			*pte = pa | PG_V | PG_RW | PG_NC_PWT | PG_NC_PCD;
			invltlb();

			tmp = *(int*)ptr;
			/*
			 * Test for alternating 1's and 0's
			 */
			*(volatile int*)ptr = 0xaaaaaaaa;
			if (*(volatile int*)ptr != 0xaaaaaaaa)
				page_bad = TRUE;
			/*
			 * Test for alternating 0's and 1's
			 */
			*(volatile int*)ptr = 0x55555555;
			if (*(volatile int*)ptr != 0x55555555)
				page_bad = TRUE;
			/*
			 * Test for all 1's
			 */
			*(volatile int*)ptr = 0xffffffff;
			if (*(volatile int*)ptr != 0xffffffff)
				page_bad = TRUE;
			/*
			 * Test for all 0's
			 */
			*(volatile int*)ptr = 0x0;
			if (*(volatile int*)ptr != 0x0)
				page_bad = TRUE;
			/*
			 * Restore original value.
			 */
			*(int*)ptr = tmp;

		skip_memtest:
			/*
			 * Adjust array of valid/good pages.
			 */
			if (page_bad == TRUE)
				continue;
			/*
			 * If this good page is a continuation of the
			 * previous set of good pages, then just increase
			 * the end pointer. Otherwise start a new chunk.
			 * Note that "end" points one higher than end,
			 * making the range >= start and < end.
			 * If we're also doing a speculative memory
			 * test and we at or past the end, bump up Maxmem
			 * so that we keep going. The first bad page
			 * will terminate the loop.
			 */
			if (phys_avail[pa_indx] == pa) {
				phys_avail[pa_indx] += PAGE_SIZE;
			}
			else {
				pa_indx++;
				if (pa_indx == PHYS_AVAIL_ENTRIES) {
					printf(
						"Too many holes in the physical address space, giving up\n");
					pa_indx--;
					full = TRUE;
					goto do_dump_avail;
				}
				phys_avail[pa_indx++] = pa;	/* start */
				phys_avail[pa_indx] = pa + PAGE_SIZE; /* end */
			}
			physmem++;
		do_dump_avail:
			if (dump_avail[da_indx] == pa) {
				dump_avail[da_indx] += PAGE_SIZE;
			}
			else {
				da_indx++;
				if (da_indx == PHYS_AVAIL_ENTRIES) {
					da_indx--;
					goto do_next;
				}
				dump_avail[da_indx++] = pa; /* start */
				dump_avail[da_indx] = pa + PAGE_SIZE; /* end */
			}
		do_next:
			if (full)
				break;
		}
	}
	*pte = 0;
	invltlb();
	if (memtest != 0)
		printf("\n");

	/*
	 * XXX
	 * The last chunk must contain at least one page plus the message
	 * buffer to avoid complicating other code (message buffer address
	 * calculation, etc.).
	 */
	while (phys_avail[pa_indx - 1] + PAGE_SIZE +
		round_page(msgbufsize) >= phys_avail[pa_indx]) {
		physmem -= atop(phys_avail[pa_indx] - phys_avail[pa_indx - 1]);
		phys_avail[pa_indx--] = 0;
		phys_avail[pa_indx--] = 0;
	}

	Maxmem = atop(phys_avail[pa_indx]);

	/* Trim off space for the message buffer. */
	phys_avail[pa_indx] -= round_page(msgbufsize);

	/* Map the message buffer. */
	msgbufp = (struct msgbuf*)PHYS_TO_DMAP(phys_avail[pa_indx]);
}

static caddr_t
native_parse_preload_data(u_int64_t modulep)
{
	return 0;
}

u_int64_t
hammer_time(u_int64_t modulep, u_int64_t physfree)
{
	return 0;
}

void
cpu_pcpu_init(struct pcpu* pcpu, int cpuid, size_t size)
{

	pcpu->pc_acpi_id = 0xffffffff;
}

static int
smap_sysctl_handler(SYSCTL_HANDLER_ARGS)
{
	struct bios_smap* smapbase;
	struct bios_smap_xattr smap;
	caddr_t kmdp;
	uint32_t* smapattr;
	int count, error, i;

	/* Retrieve the system memory map from the loader. */
	kmdp = preload_search_by_type("elf kernel");
	if (kmdp == NULL)
		kmdp = preload_search_by_type("elf64 kernel");
	smapbase = (struct bios_smap*)preload_search_info(kmdp,
		MODINFO_METADATA | MODINFOMD_SMAP);
	if (smapbase == NULL)
		return (0);
	smapattr = (uint32_t*)preload_search_info(kmdp,
		MODINFO_METADATA | MODINFOMD_SMAP_XATTR);
	count = *((uint32_t*)smapbase - 1) / sizeof(*smapbase);
	error = 0;
	for (i = 0; i < count; i++) {
		smap.base = smapbase[i].base;
		smap.length = smapbase[i].length;
		smap.type = smapbase[i].type;
		if (smapattr != NULL)
			smap.xattr = smapattr[i];
		else
			smap.xattr = 0;
		error = SYSCTL_OUT(req, &smap, sizeof(smap));
	}
	return (error);
}
SYSCTL_PROC(_machdep, OID_AUTO, smap,
	CTLTYPE_OPAQUE | CTLFLAG_RD | CTLFLAG_MPSAFE, NULL, 0,
	smap_sysctl_handler, "S,bios_smap_xattr",
	"Raw BIOS SMAP data");

static int
efi_map_sysctl_handler(SYSCTL_HANDLER_ARGS)
{
	struct efi_map_header* efihdr;
	caddr_t kmdp;
	uint32_t efisize;

	kmdp = preload_search_by_type("elf kernel");
	if (kmdp == NULL)
		kmdp = preload_search_by_type("elf64 kernel");
	efihdr = (struct efi_map_header*)preload_search_info(kmdp,
		MODINFO_METADATA | MODINFOMD_EFI_MAP);
	if (efihdr == NULL)
		return (0);
	efisize = *((uint32_t*)efihdr - 1);
	return (SYSCTL_OUT(req, efihdr, efisize));
}
SYSCTL_PROC(_machdep, OID_AUTO, efi_map,
	CTLTYPE_OPAQUE | CTLFLAG_RD | CTLFLAG_MPSAFE, NULL, 0,
	efi_map_sysctl_handler, "S,efi_map_header",
	"Raw EFI Memory Map");

void
spinlock_enter(void)
{
	struct thread* td;
	register_t flags;

	td = curthread;
	if (td->td_md.md_spinlock_count == 0) {
		flags = intr_disable();
		td->td_md.md_spinlock_count = 1;
		td->td_md.md_saved_flags = flags;
		critical_enter();
	}
	else
		td->td_md.md_spinlock_count++;
}

void
spinlock_exit(void)
{
	struct thread* td;
	register_t flags;

	td = curthread;
	flags = td->td_md.md_saved_flags;
	td->td_md.md_spinlock_count--;
	if (td->td_md.md_spinlock_count == 0) {
		critical_exit();
		intr_restore(flags);
	}
}

/*
 * Construct a PCB from a trapframe. This is called from kdb_trap() where
 * we want to start a backtrace from the function that caused us to enter
 * the debugger. We have the context in the trapframe, but base the trace
 * on the PCB. The PCB doesn't have to be perfect, as long as it contains
 * enough for a backtrace.
 */
void
makectx(struct trapframe* tf, struct pcb* pcb)
{

	pcb->pcb_r12 = tf->tf_r12;
	pcb->pcb_r13 = tf->tf_r13;
	pcb->pcb_r14 = tf->tf_r14;
	pcb->pcb_r15 = tf->tf_r15;
	pcb->pcb_rbp = tf->tf_rbp;
	pcb->pcb_rbx = tf->tf_rbx;
	pcb->pcb_rip = tf->tf_rip;
	pcb->pcb_rsp = tf->tf_rsp;
}


void*
uma_small_alloc(uma_zone_t zone, vm_size_t bytes, int domain, u_int8_t* flags,
	int wait)
{
	void* va = context_malloc(bytes);
	return (va);
}

void
uma_small_free(void* mem, vm_size_t size, u_int8_t flags)
{
	context_free(mem);
}
void
ssdtosyssd(ssd, sd)
struct soft_segment_descriptor* ssd;
struct system_segment_descriptor* sd;
{

}

void
init_proc0()
{
	proc_linkup0(&proc0, &thread0);
	thread0.td_kstack = (vm_offset_t)context_malloc(kstack_pages * PAGE_SIZE);
	thread0.td_kstack_pages = kstack_pages;

	pcpup->pc_curpcb = thread0.td_pcb;

	set_top_of_stack_td(&thread0);
	thread0.td_pcb = get_pcb_td(&thread0);
	thread0.td_critnest = 0;

	/* setup proc 0's pcb */
	thread0.td_pcb->pcb_flags = 0;
	thread0.td_frame = &proc0_tf;
}

void
critical_enter(void)
{
	struct thread* td;

	td = (struct thread*)curthread;
	td->td_critnest++;
	atomic_interrupt_fence();
}

void
critical_exit(void)
{
	struct thread* td;

	td = (struct thread*)curthread;
	KASSERT(td->td_critnest != 0,
		("critical_exit: td_critnest == 0"));
	atomic_interrupt_fence();
	td->td_critnest--;
	atomic_interrupt_fence();
	if (__predict_false(td->td_owepreempt))
		critical_exit_preempt();

}