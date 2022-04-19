#include "ports/port_init.h"
#include <errno.h>
void port_set_errno(int error)
{
    errno = error;
}
int port_get_errno()
{
    return errno;
}
#if 0
void port_set_errno2(int error)
{
    switch (error) {
        case VOS_EPERM:       errno = EPERM; break;
        case VOS_ENOENT:      errno = ENOENT; break;
        case VOS_ESRCH:       errno = ESRCH; break;
        case VOS_EINTR:       errno = EINTR; break;
        case VOS_EIO:         errno = EIO; break;
        case VOS_ENXIO:       errno = ENXIO; break;
        case VOS_E2BIG:       errno = E2BIG; break;
        case VOS_ENOEXEC:     errno = ENOEXEC; break;
        case VOS_EBADF:       errno = EBADF; break;
        case VOS_ECHILD:      errno = ECHILD; break;
        case VOS_EDEADLK:     errno = EDEADLK; break;
        case VOS_ENOMEM:      errno = ENOMEM; break;
        case VOS_EACCES:      errno = EACCES; break;
        case VOS_EFAULT:      errno = EFAULT; break;
        case VOS_EBUSY:       errno = EBUSY; break;
        case VOS_EEXIST:      errno = EEXIST; break;
        case VOS_EXDEV:       errno = EXDEV; break;
        case VOS_ENODEV:      errno = ENODEV; break;
        case VOS_ENOTDIR:     errno = ENOTDIR; break;
        case VOS_EISDIR:      errno = EISDIR; break;
        case VOS_EINVAL:      errno = EINVAL; break;
        case VOS_ENFILE:      errno = ENFILE; break;
        case VOS_EMFILE:      errno = EMFILE; break;
        case VOS_ENOTTY:      errno = ENOTTY; break;
        case VOS_ETXTBSY:     errno = ETXTBSY; break;
        case VOS_EFBIG:       errno = EFBIG; break;
        case VOS_ENOSPC:      errno = ENOSPC; break;
        case VOS_ESPIPE:      errno = ESPIPE; break;
        case VOS_EROFS:       errno = EROFS; break;
        case VOS_EMLINK:      errno = EMLINK; break;
        case VOS_EPIPE:       errno = EPIPE; break;
        case VOS_EDOM:        errno = EDOM; break;
        case VOS_ERANGE:      errno = ERANGE; break;
    
        /*case VOS_EAGAIN:       same as EWOULDBLOCK */
        case VOS_EWOULDBLOCK:     errno = EWOULDBLOCK; break;
    
        case VOS_EINPROGRESS:     errno = EINPROGRESS; break;
        case VOS_EALREADY:        errno = EALREADY; break;
        case VOS_ENOTSOCK:        errno = ENOTSOCK; break;
        case VOS_EDESTADDRREQ:    errno = EDESTADDRREQ; break;
        case VOS_EMSGSIZE:        errno = EMSGSIZE; break;
        case VOS_EPROTOTYPE:      errno = EPROTOTYPE; break;
        case VOS_ENOPROTOOPT:     errno = ENOPROTOOPT; break;
        case VOS_EPROTONOSUPPORT: errno = EPROTONOSUPPORT; break;

        /* case VOS_EOPNOTSUPP:   same as ENOTSUP */
        case VOS_ENOTSUP:         errno = ENOTSUP; break;

        case VOS_EAFNOSUPPORT:    errno = EAFNOSUPPORT; break;
        case VOS_EADDRINUSE:      errno = EADDRINUSE; break;
        case VOS_EADDRNOTAVAIL:   errno = EADDRNOTAVAIL; break;
        case VOS_ENETDOWN:        errno = ENETDOWN; break;
        case VOS_ENETUNREACH:     errno = ENETUNREACH; break;
        case VOS_ENETRESET:       errno = ENETRESET; break;
        case VOS_ECONNABORTED:    errno = ECONNABORTED; break;
        case VOS_ECONNRESET:      errno = ECONNRESET; break;
        case VOS_ENOBUFS:         errno = ENOBUFS; break;
        case VOS_EISCONN:         errno = EISCONN; break;
        case VOS_ENOTCONN:        errno = ENOTCONN; break;
        case VOS_ETIMEDOUT:       errno = ETIMEDOUT; break;
        case VOS_ECONNREFUSED:    errno = ECONNREFUSED; break;
        case VOS_ELOOP:           errno = ELOOP; break;
        case VOS_ENAMETOOLONG:    errno = ENAMETOOLONG; break;
        case VOS_EHOSTUNREACH:    errno = EHOSTUNREACH; break;
        case VOS_ENOTEMPTY:       errno = ENOTEMPTY; break;
        case VOS_ENOLCK:      errno = ENOLCK; break;
        case VOS_ENOSYS:      errno = ENOSYS; break;
        case VOS_EIDRM:       errno = EIDRM; break;
        case VOS_ENOMSG:      errno = ENOMSG; break;
        case VOS_EOVERFLOW:   errno = EOVERFLOW; break;
        case VOS_ECANCELED:   errno = ECANCELED; break;
        case VOS_EILSEQ:      errno = EILSEQ; break;
        case VOS_EBADMSG:     errno = EBADMSG; break;
        case VOS_ENOLINK:     errno = ENOLINK; break;
        case VOS_EPROTO:      errno = EPROTO; break;
        default:              errno = error; break;
    }
}

#endif