#include <sys/cdefs.h>
#include <sys/types.h>
#include <machine/cpufunc.h>
#include "context.h"
u_int	cpu_stdext_feature;
u_int	cpu_stdext_feature2;
u_int	cpu_stdext_feature3;
uint64_t cpu_ia32_arch_caps;
u_int	cpu_fxsr;
u_int	cpu_high;
u_int	cpu_id;
int	workaround_erratum383;
u_int	cpu_vendor_id;
u_int	cpu_feature;
u_int	cpu_feature2;
u_int	amd_feature;
u_int	amd_feature2;

int use_xsave;			/* non-static for cpu_switch.S */
uint64_t xsave_mask;		/* the same */
char	brwsection[1024];
char	btext[1024];
char	_end[1024];
char	etext[1024];
uint64_t lapic_paddr;
u_int	bsfl(u_int mask)
{
	return 0;
}
u_int	bsrl(u_int mask)
{
	return 0;
}

int bsfq(int a)
{
	return 0;
}
void lfence(void)
{
	context_lfence();
}
void mfence(void)
{
	context_mfence();
}
void sfence(void)
{
	context_sfence();
}
void ia32_pause(void)
{
	context_yield();
}

uint64_t rdtsc(void)
{
	return context_rdtsc();
}
uint64_t rdtscp(void)
{
	return context_rdtscp();
}
uint64_t rdtscp_aux(uint32_t* aux)
{
	return context_rdtscp_aux(aux);
}
uint32_t rdtsc32(void)
{
	return context_rdtsc32();
}
uint32_t rdtscp32(void)
{
	return context_rdtscp32();
}
int breakpoint(void)
{
	return context_breakpoint();
}