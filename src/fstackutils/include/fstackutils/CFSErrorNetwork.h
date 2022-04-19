/***********************************************************************
 * Copyright (C) 2016-2018, Nanjing StarOS Technology Co., Ltd 
**********************************************************************/
#ifndef CFSERRORNETWORK_H
#define CFSERRORNETWORK_H

#include "CAWError.h"

#define FS_EPERM         1        /* Operation not permitted */
#define FS_ENOENT        2        /* No such file or directory */
#define FS_ESRCH         3        /* No such process */
#define FS_EINTR         4        /* Interrupted system call */
#define FS_EIO           5        /* Input/output error */
#define FS_ENXIO         6        /* Device not configured */
#define FS_E2BIG         7        /* Argument list too long */
#define FS_ENOEXEC       8        /* Exec format error */
#define FS_EBADF         9        /* Bad file descriptor */
#define FS_ECHILD        10        /* No child processes */
#define FS_EDEADLK       11        /* Resource deadlock avoided */
#define FS_ENOMEM        12        /* Cannot allocate memory */
#define FS_EACCES        13        /* Permission denied */
#define FS_EFAULT        14        /* Bad address */
#define FS_ENOTBLK       15        /* Block device required */
#define FS_EBUSY         16        /* Device busy */
#define FS_EEXIST        17        /* File exists */
#define FS_EXDEV         18        /* Cross-device link */
#define FS_ENODEV        19        /* Operation not supported by device */
#define FS_ENOTDIR       20        /* Not a directory */
#define FS_EISDIR        21        /* Is a directory */
#define FS_EINVAL        22        /* Invalid argument */
#define FS_ENFILE        23        /* Too many open files in system */
#define FS_EMFILE        24        /* Too many open files */
#define FS_ENOTTY        25        /* Inappropriate ioctl for device */
#define FS_ETXTBSY       26        /* Text file busy */
#define FS_EFBIG         27        /* File too large */
#define FS_ENOSPC        28        /* No space left on device */
#define FS_ESPIPE        29        /* Illegal seek */
#define FS_EROFS         30        /* Read-only filesystem */
#define FS_EMLINK        31        /* Too many links */
#define FS_EPIPE         32        /* Broken pipe */

/* math software */
#define FS_EDOM          33        /* Numerical argument out of domain */
#define FS_ERANGE        34        /* Result too large */

/* non-blocking and interrupt i/o */
#define FS_EAGAIN        35        /* Resource temporarily unavailable */
#define FS_EWOULDBLOCK   FS_EAGAIN        /* Operation would block */
#define FS_EINPROGRESS   36        /* Operation now in progress */
#define FS_EALREADY      37        /* Operation already in progress */

/* ipc/network software -- argument errors */
#define FS_ENOTSOCK      38        /* Socket operation on non-socket */
#define FS_EDESTADDRREQ  39        /* Destination address required */
#define FS_EMSGSIZE      40        /* Message too long */
#define FS_EPROTOTYPE    41        /* Protocol wrong type for socket */
#define FS_ENOPROTOOPT   42        /* Protocol not available */
#define FS_EPROTONOSUPPORT    43        /* Protocol not supported */
#define FS_ESOCKTNOSUPPORT    44        /* Socket type not supported */
#define FS_EOPNOTSUPP         45        /* Operation not supported */
#define FS_ENOTSUP        FS_EOPNOTSUPP    /* Operation not supported */
#define FS_EPFNOSUPPORT       46        /* Protocol family not supported */
#define FS_EAFNOSUPPORT       47        /* Address family not supported by protocol family */
#define FS_EADDRINUSE         48        /* Address already in use */
#define FS_EADDRNOTAVAIL      49        /* Can't assign requested address */

/* ipc/network software -- operational errors */
#define FS_ENETDOWN       50        /* Network is down */
#define FS_ENETUNREACH    51        /* Network is unreachable */
#define FS_ENETRESET      52        /* Network dropped connection on reset */
#define FS_ECONNABORTED   53        /* Software caused connection abort */
#define FS_ECONNRESET     54        /* Connection reset by peer */
#define FS_ENOBUFS        55        /* No buffer space available */
#define FS_EISCONN        56        /* Socket is already connected */
#define FS_ENOTCONN       57        /* Socket is not connected */
#define FS_ESHUTDOWN      58        /* Can't send after socket shutdown */
#define FS_ETOOMANYREFS   59        /* Too many references: can't splice */
#define FS_ETIMEDOUT      60        /* Operation timed out */
#define FS_ECONNREFUSED   61        /* Connection refused */

#define FS_ELOOP          62        /* Too many levels of symbolic links */
#define FS_ENAMETOOLONG   63        /* File name too long */

/* should be rearranged */
#define FS_EHOSTDOWN      64        /* Host is down */
#define FS_EHOSTUNREACH   65        /* No route to host */
#define FS_ENOTEMPTY      66        /* Directory not empty */

/* quotas & mush */
#define FS_EPROCLIM       67        /* Too many processes */
#define FS_EUSERS         68        /* Too many users */
#define FS_EDQUOT         69        /* Disc quota exceeded */

#define FS_ESTALE         70        /* Stale NFS file handle */
#define FS_EREMOTE        71        /* Too many levels of remote in path */
#define FS_EBADRPC        72        /* RPC struct is bad */
#define FS_ERPCMISMATCH   73        /* RPC version wrong */
#define FS_EPROGUNAVAIL   74        /* RPC prog. not avail */
#define FS_EPROGMISMATCH  75        /* Program version wrong */
#define FS_EPROCUNAVAIL   76        /* Bad procedure for program */

#define FS_ENOLCK         77        /* No locks available */
#define FS_ENOSYS         78        /* Function not implemented */

#define FS_EFTYPE         79        /* Inappropriate file type or format */
#define FS_EAUTH          80        /* Authentication error */
#define FS_ENEEDAUTH      81        /* Need authenticator */
#define FS_EIDRM          82        /* Identifier removed */
#define FS_ENOMSG         83        /* No message of desired type */
#define FS_EOVERFLOW      84        /* Value too large to be stored in data type */
#define FS_ECANCELED      85        /* Operation canceled */
#define FS_EILSEQ         86        /* Illegal byte sequence */
#define FS_ENOATTR        87        /* Attribute not found */

#define FS_EDOOFUS        88        /* Programming error */

#define FS_EBADMSG        89        /* Bad message */
#define FS_EMULTIHOP      90        /* Multihop attempted */
#define FS_ENOLINK        91        /* Link has been severed */
#define FS_EPROTO         92        /* Protocol error */

#define FS_ENOTCAPABLE    93        /* Capabilities insufficient */
#define FS_ECAPMODE       94        /* Not permitted in capability mode */
#define FS_ENOTRECOVERABLE 95        /* State not recoverable */
#define FS_EOWNERDEAD      96        /* Previous owner died */

#endif // CFSERRORNETWORK_H