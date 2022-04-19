#include "ports/port_memory.h"
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

void* port_malloc(size_t sz)
{
	char* p = new char[sz];
	return p;
}
void port_free(void* ptr)
{
	if (ptr)
	{
		char* p = (char*)ptr;
		delete[] p;
	}
}
void* port_realloc(void* pointer, size_t size)
{
	if (pointer)
	{
		char* p = (char*)pointer;
		delete[] p;
	}
	return port_malloc(size);
}
void* port_calloc(size_t num, size_t size)
{
	char* p = new char[num*size];
	return p;
}