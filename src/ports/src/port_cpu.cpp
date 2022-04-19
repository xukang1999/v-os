#include "ports/port_cpu.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>  
#include <chrono>  
#include <thread> 
#include <atomic>
#include <mutex>
#include <vos/vos_yield.h>
#include <vos/vos_time.h>
void port_yield(void)
{
	//YieldCurrentThread();
#if defined(VOS_WIN32)
    YieldProcessor();
#elif defined(VOS_MACOS)
    sched_yield();
#else
    static const struct timespec ts_null = { 0 };
    nanosleep(&ts_null, NULL);
#endif
}

uint64_t port_cpud_ticks(void)
{
	return VOS_GetCurrentTimeNanos();
}
#if _WIN32

#include <intrin.h>
uint64_t port_rdtsc()  // win
{
    return __rdtsc();
}
uint64_t port_rdtscp()  // win
{
    uint64_t i;
    unsigned int ui;
    i = __rdtscp(&ui);
    return i;
}
uint64_t port_rdtscp_aux(uint32_t* aux)
{
    return __rdtscp(aux);
}

uint32_t port_rdtsc32(void)
{
    uint64_t nrd= __rdtsc();
    nrd = nrd << 32;
    return nrd >> 32;
}
uint32_t port_rdtscp32(void)
{
    uint64_t i;
    unsigned int ui;
    i = __rdtscp(&ui);
    i = i << 32;
    return i >> 32;
}
void port_sfence()
{
    _mm_sfence();
}
void port_lfence()
{
    _mm_lfence();
}
void port_mfence()
{
    _mm_mfence();
}
int port_breakpoint(void)
{
    exit(0);
    return 0;
}
#else

uint64_t port_rdtsc() // linux
{
#if _CPU_TYPE==X86_64
    unsigned int lo, hi;
    __asm__ volatile ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
#else
	return 0;
#endif
}
uint64_t port_rdtscp() // linux
{
#if _CPU_TYPE==X86_64
    uint32_t low, high;

    __asm __volatile("rdtscp" : "=a" (low), "=d" (high) : : "ecx");
    return (low | ((uint64_t)high << 32));
#else
	return 0;
#endif
}
uint64_t port_rdtscp_aux(uint32_t* aux)
{
#if _CPU_TYPE==X86_64
    uint32_t low, high;

    __asm __volatile("rdtscp" : "=a" (low), "=d" (high), "=c" (*aux));
    return (low | ((uint64_t)high << 32));
#else
	return 0;
#endif
}
uint32_t
port_rdtsc32(void)
{
#if _CPU_TYPE==X86_64
    uint32_t rv;

    __asm __volatile("rdtsc" : "=a" (rv) : : "edx");
    return (rv);
#else
	return 0;
#endif
}

uint32_t
port_rdtscp32(void)
{
#if _CPU_TYPE==X86_64
    uint32_t rv;

    __asm __volatile("rdtscp" : "=a" (rv) : : "ecx", "edx");
    return (rv);
#else
	return 0;
#endif
}
void port_sfence()
{
#if _CPU_TYPE==X86_64
    __asm __volatile("sfence;" : : : "memory");
#else
	return;
#endif
}
void port_lfence()
{
#if _CPU_TYPE==X86_64
    __asm __volatile("lfence;" : : : "memory");
#else
	return;
#endif
}
void port_mfence()
{
#if _CPU_TYPE==X86_64
    __asm __volatile("mfence;" : : : "memory");
#else
	return;
#endif
}
int port_breakpoint(void)
{
    exit(0);
    return -1;
}
#endif

