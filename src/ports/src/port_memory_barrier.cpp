#include "ports/port_memory_barrier.h"
#include <stdio.h>
#include <stdlib.h>

void port_memory_barrier(void)
{
#ifdef _WIN32
	MemoryBarrier();
#else
	__asm __volatile(" " : : : "memory");
#endif
}