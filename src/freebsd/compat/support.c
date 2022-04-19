#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/proc.h>
#include <sys/systm.h>
#include <sys/bus.h>
#include <sys/cpu.h>
#include <sys/domainset.h>
#include <sys/kdb.h>
#include <sys/kernel.h>
#include <sys/ktr.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/mutex.h>
#include <sys/pcpu.h>
#include <sys/rwlock.h>
#include <sys/sched.h>
#include <sys/smp.h>
#include <sys/sysctl.h>

const char la57_trampoline[1];
const char la57_trampoline_gdt_desc[1];
const char la57_trampoline_gdt[1];
const char la57_trampoline_end[1];

void mds_handler_void(void)
{
}
void mds_handler_verw(void){
}
void mds_handler_ivb(void){
}
void mds_handler_bdw(void){
}
void mds_handler_skl_sse(void){
}
void mds_handler_skl_avx(void){
}
void mds_handler_skl_avx512(void){
}
void mds_handler_silvermont(void){
}

void	pmap_pti_pcid_invalidate(uint64_t ucr3, uint64_t kcr3)
{
}
void	pmap_pti_pcid_invlpg(uint64_t ucr3, uint64_t kcr3, vm_offset_t va)
{
}
void	pmap_pti_pcid_invlrng(uint64_t ucr3, uint64_t kcr3, vm_offset_t sva,
	    vm_offset_t eva)
		{
		}