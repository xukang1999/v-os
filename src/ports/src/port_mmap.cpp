
#include "ports/port_mmap.h"
#ifdef _WIN32
#include <Windows.h>
#include <io.h>
#define MS_ASYNC            1
#define MS_SYNC             2
#define MS_INVALIDATE       4

#ifndef FILE_MAP_EXECUTE
# define FILE_MAP_EXECUTE   0x0020
#endif

#else
#if defined(__APPLE__) && defined(__MACH__)
    #include <sys/mman.h>
#else
    #include <sys/mman.h>
#endif
#endif
#ifdef _WIN32
static int _mmap_error(DWORD err, int deferr) {
    if (0 == err)
        return deferr;
    return err;
}

static DWORD _mmap_prot_page(int prot) {
    DWORD protect = 0;

    if (VOS_MAP_PROT_NONE == prot)
        return protect;

    if (prot & VOS_MAP_PROT_EXEC)
        protect = (prot & VOS_MAP_PROT_WRITE) ? PAGE_EXECUTE_READWRITE : PAGE_EXECUTE_READ;
    else
        protect = (prot & VOS_MAP_PROT_WRITE) ? PAGE_READWRITE : PAGE_READONLY;

    return protect;
}

static DWORD _mmap_prot_file(int prot) {
    DWORD desiredAccess = 0;

    if (VOS_MAP_PROT_NONE == prot)
        return desiredAccess;

    if (prot & VOS_MAP_PROT_READ)
        desiredAccess |= FILE_MAP_READ;

    if (prot & VOS_MAP_PROT_WRITE)
        desiredAccess |= FILE_MAP_WRITE;

    if (prot & VOS_MAP_PROT_EXEC)
        desiredAccess |= FILE_MAP_EXECUTE;

    return desiredAccess;
}
#endif

void *port_mmap(void *addr, size_t len, int prot, int flags, int fildes, off_t off) {
#ifdef _WIN32
    HANDLE fm, h;
    void * map = VOS_MAP_FAILED;

    DWORD protect = _mmap_prot_page(prot);
    DWORD desiredAccess = _mmap_prot_file(prot);

    DWORD dwFileOffsetHigh = 0;
    DWORD dwFileOffsetLow = (DWORD)off;

    DWORD dwMaxSizeHigh = 0;
    DWORD dwMaxSizeLow = (DWORD)(off + len);

    errno = 0;

    if (!len
        /* Unsupported flag combinations */
        || (flags & VOS_MAP_FIXED)
        /* Usupported protection combinations */
        || (VOS_MAP_PROT_EXEC == prot)) {
        errno = EINVAL;
        return VOS_MAP_FAILED;
    }
    if (fildes != -1)
    {
        h = !(flags & VOS_MAP_ANONYMOUS) ? (HANDLE)_get_osfhandle(fildes) : INVALID_HANDLE_VALUE;

        if ((INVALID_HANDLE_VALUE == h) && !(flags & VOS_MAP_ANONYMOUS)) {
            errno = EBADF;
            return VOS_MAP_FAILED;
        }
    }
    else
    {
        h = INVALID_HANDLE_VALUE;
    }

    fm = CreateFileMapping(h, NULL, protect, dwMaxSizeHigh, dwMaxSizeLow, NULL);

    if (!fm) {
        errno = _mmap_error(GetLastError(), EPERM);
        return VOS_MAP_FAILED;
    }

    map = MapViewOfFile(fm, desiredAccess, dwFileOffsetHigh, dwFileOffsetLow, len);

    CloseHandle(fm);

    if (!map) {
        errno = _mmap_error(GetLastError(), EPERM);
        return VOS_MAP_FAILED;
    }

    return map;
#else
    int host_prot;
    int host_flags;
    host_prot = 0;
    if ((prot & VOS_MAP_PROT_READ) == VOS_MAP_PROT_READ)   host_prot |= PROT_READ;
    if ((prot & VOS_MAP_PROT_WRITE) == VOS_MAP_PROT_WRITE) host_prot |= PROT_WRITE;

    host_flags = 0;
    if ((flags & VOS_MAP_SHARED) == VOS_MAP_SHARED)   host_flags |= MAP_SHARED;
    if ((flags & VOS_MAP_PRIVATE) == VOS_MAP_PRIVATE) host_flags |= MAP_PRIVATE;
    if ((flags & VOS_MAP_ANON) == VOS_MAP_ANON)       host_flags |= MAP_ANON;
    void* ret = mmap(addr, len, host_prot, host_flags, fildes, off);
    return ret;
#endif
}

int port_munmap(void *addr, size_t len) {
#ifdef _WIN32
    if (!UnmapViewOfFile(addr)) {
        errno = _mmap_error(GetLastError(), EPERM);
        return -1;
    }
    return 0;
#else
    munmap(addr, len);
#endif
}

int port_msync(void *addr, size_t len, int flags) {
#ifdef _WIN32
    if (!FlushViewOfFile(addr, len)) {
        errno = _mmap_error(GetLastError(), EPERM);
        return -1;
    }
    return 0;
#else
    return msync(addr, len, flags);
#endif
}

int port_mprotect(void *addr, size_t len, int prot) {
#ifdef _WIN32
    DWORD newProtect = _mmap_prot_page(prot);
    DWORD oldProtect = 0;

    if (!VirtualProtect(addr, len, newProtect, &oldProtect)) {
        errno = _mmap_error(GetLastError(), EPERM);
        return -1;
    }
    return 0;
#else
    return mprotect(addr, len, prot);
#endif
}

int port_mlock(const void *addr, size_t len) {
#ifdef _WIN32
    if (!VirtualLock((LPVOID)addr, len)) {
        errno = _mmap_error(GetLastError(), EPERM);
        return -1;
    }
    return 0;
#else
    return mlock(addr, len);
#endif
}

int port_munlock(const void *addr, size_t len) {
#ifdef _WIN32
    if (!VirtualUnlock((LPVOID)addr, len)) {
        errno = _mmap_error(GetLastError(), EPERM);
        return -1;
    }
    return 0;
#else
    return munlock(addr, len);
#endif
}