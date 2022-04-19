#ifndef PORT__MMAP_H__
#define PORT__MMAP_H__
#include <sys/types.h>
#include "staros/so_init.h"
#ifdef __cplusplus
extern "C" {
#endif

SO_EXPORT void *port_mmap(void *addr, size_t len, int prot, int flags, int fildes, off_t off);
SO_EXPORT int port_munmap(void *addr, size_t len);
SO_EXPORT int port_msync(void *addr, size_t len, int flags);
SO_EXPORT int port_mprotect(void *addr, size_t len, int prot);
SO_EXPORT int port_mlock(const void *addr, size_t len);
SO_EXPORT int port_munlock(const void *addr, size_t len);

#ifdef __cplusplus
}
#endif

#endif  /* __MMAP_H__ */