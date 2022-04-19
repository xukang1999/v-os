#ifndef PORT_MEMORY__H
#define PORT_MEMORY__H
#include "staros/so_init.h"
#ifdef __cplusplus
extern "C" {
#endif
SO_EXPORT void port_memory_barrier(void);

SO_EXPORT void* port_malloc(size_t sz);
SO_EXPORT void port_free(void* ptr);
SO_EXPORT void* port_realloc(void* pointer, size_t size);
SO_EXPORT void* port_calloc(size_t num, size_t size);

#ifdef __cplusplus
}
#endif
#endif