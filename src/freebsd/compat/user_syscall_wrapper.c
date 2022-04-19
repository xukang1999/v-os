#include <sys/param.h>
#include <sys/limits.h>
#include <sys/uio.h>
#include <sys/proc.h>
#include <sys/syscallsubr.h>
#include <sys/module.h>
#include <sys/param.h>
#include <sys/malloc.h>
#include <sys/socketvar.h>
#include <sys/event.h>
#include <sys/kernel.h>
#include <sys/refcount.h>
#include <sys/sysctl.h>
#include <sys/pcpu.h>
#include <sys/select.h>
#include <sys/poll.h>
#include <sys/event.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/sctp.h>
#include <sys/ttycom.h>
#include <sys/filio.h>
#include <sys/sysproto.h>
#include <sys/fcntl.h>
#include <net/route.h>
#include <net/route/route_ctl.h>

#include <net/if.h>
#include <sys/sockio.h>

#include <machine/stdarg.h>

#include "user_syscall_wrapper.h"

/* POSIX-LIKE api begin */

int vos_sock_fcntl(int s, int cmd, int val)
{
    return ff_fcntl(s, cmd, val);
}
int vos_sock_sysctl(const int *name, u_int namelen, void *oldp, size_t *oldlenp,const void *newp, size_t newlen)
{
    return ff_sysctl(name,namelen,oldp,oldlenp,newp,newlen);
}
int vos_sock_ioctl(int s, long cmd, void *argp)
{
    return ff_ioctl(s,cmd,argp);
}
int vos_sock_socket(int domain, int type, int protocol)
{
    return ff_socket(domain,type,protocol);
}
int vos_sock_setsockopt(int s, int level, int optname, const void *optval,socklen_t optlen)
{
    return ff_setsockopt(s,level,optname,optval, optlen);
}
int vos_sock_getsockopt(int s, int level, int optname, void *optval,socklen_t *optlen)
{
    return ff_getsockopt(s,level, optname, optval, optlen);
}
int vos_sock_listen(int s, int backlog)
{
    return ff_listen(s,backlog);
}
int vos_sock_bind(int s, const struct vos_sock_sockaddr *addr, socklen_t addrlen)
{
    return ff_bind(s, (struct linux_sockaddr*)addr, addrlen);
}
int vos_sock_accept(int s, struct vos_sock_sockaddr *addr, socklen_t *addrlen)
{
    return ff_accept(s,(struct linux_sockaddr*)addr,addrlen);
}
int vos_sock_connect(int s, const struct vos_sock_sockaddr *name, socklen_t namelen)
{
    return ff_connect(s, (struct linux_sockaddr*)name,namelen);
}
int vos_sock_close(int fd)
{
    return ff_close(fd);
}
int vos_sock_shutdown(int s, int how)
{
    return ff_shutdown(s,how);
}

int vos_sock_getpeername(int s, struct vos_sock_sockaddr *name,
    socklen_t *namelen)
{
    return ff_getpeername(s, (struct linux_sockaddr*)name, namelen);
}
int vos_sock_getsockname(int s, struct vos_sock_sockaddr *name,
    socklen_t *namelen)
{
    return ff_getsockname(s, (struct linux_sockaddr*)name, namelen);
}

ssize_t vos_sock_read(int d, void *buf, size_t nbytes)
{
    return ff_read(d, buf, nbytes);
}
ssize_t vos_sock_readv(int fd, const struct iovec *iov, int iovcnt)
{
    return ff_readv(fd, iov, iovcnt);
}
ssize_t vos_sock_write(int fd, const void *buf, size_t nbytes)
{
    return ff_write(fd, buf, nbytes);
}
ssize_t vos_sock_writev(int fd, const struct iovec *iov, int iovcnt)
{
    return ff_writev(fd, iov, iovcnt);
}
ssize_t vos_sock_send(int s, const void *buf, size_t len, int flags)
{
    return ff_send(s, buf, len, flags);
}
ssize_t vos_sock_sendto(int s, const void *buf, size_t len, int flags,
    const struct vos_sock_sockaddr *to, socklen_t tolen)
{
    return ff_sendto(s, buf, len, flags, (struct linux_sockaddr*)to, tolen);
}
ssize_t vos_sock_sendmsg(int s, const struct msghdr *msg, int flags)
{
    return ff_sendmsg(s, msg, flags);
}

ssize_t vos_sock_recv(int s, void *buf, size_t len, int flags)
{
    return ff_recv(s, buf, len, flags);
}
ssize_t vos_sock_recvfrom(int s, void *buf, size_t len, int flags,
    struct vos_sock_sockaddr *from, socklen_t *fromlen)
{
    return ff_recvfrom(s, buf, len, flags, (struct linux_sockaddr*)from, fromlen);
}
ssize_t vos_sock_recvmsg(int s, struct msghdr *msg, int flags)
{
    return ff_recvmsg(s, msg, flags);
}

int vos_sock_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
    struct timeval *timeout)
{
    return ff_select(nfds, readfds, writefds, exceptfds, timeout);
}

int vos_sock_poll(struct pollfd fds[], nfds_t nfds, int timeout)
{
    return ff_poll(fds, nfds, timeout);
}

int
vos_sock_socketpair(int domain, int type, int protocol,int rsv[2])
{
    return ff_socketpair(domain, type, protocol, rsv);
}

int vos_sock_gettimeofday(struct timeval *tv, struct timezone *tz)
{
    return ff_gettimeofday(tv, tz);
}

int vos_sock_fdisused(int fd)
{
    return ff_fdisused(fd);
}

int vos_sock_getmaxfd(void)
{
    return ff_getmaxfd();
}

int
vos_sock_sctp_sendmsg(int s, void *msg, size_t len, struct vos_sock_sockaddr *to,
	     socklen_t tolen, uint32_t ppid, uint32_t flags,
	     uint16_t stream_no, uint32_t timetolive, uint32_t context)
{
    return ff_sctp_sendmsg(s,msg,len,(struct linux_sockaddr*)to,tolen,ppid,flags,stream_no,timetolive,context);
}
#define LINUX_MSG_NOTIFICATION      0x00008000
#define FREEBSD_MSG_NOTIFICATION    0x00002000
#define LINUX_MSG_EOR               0x00000080
#define FREEBSD_MSG_EOR             0x00000008
#define FREEBSD_SCTP_ASSOC_CHANGE                       0x0001
#define FREEBSD_SCTP_PEER_ADDR_CHANGE                   0x0002
#define FREEBSD_SCTP_REMOTE_ERROR                       0x0003
#define FREEBSD_SCTP_SEND_FAILED                        0x0004
#define FREEBSD_SCTP_SHUTDOWN_EVENT                     0x0005
#define FREEBSD_SCTP_ADAPTATION_INDICATION              0x0006

#define LINUX_SCTP_ASSOC_CHANGE                       SCTP_ASSOC_CHANGE
#define LINUX_SCTP_PEER_ADDR_CHANGE                   SCTP_PEER_ADDR_CHANGE
#define LINUX_SCTP_REMOTE_ERROR                       SCTP_REMOTE_ERROR
#define LINUX_SCTP_SEND_FAILED                        SCTP_SEND_FAILED
#define LINUX_SCTP_SHUTDOWN_EVENT                     SCTP_SHUTDOWN_EVENT
#define LINUX_SCTP_ADAPTATION_INDICATION              SCTP_ADAPTATION_INDICATION

#define FREEBSD_SCTP_COMM_UP            0x0001
#define FREEBSD_SCTP_COMM_LOST          0x0002
#define FREEBSD_SCTP_RESTART            0x0003
#define FREEBSD_SCTP_SHUTDOWN_COMP      0x0004
#define FREEBSD_SCTP_CANT_STR_ASSOC     0x0005

#define LINUX_SCTP_COMM_UP            SCTP_COMM_UP
#define LINUX_SCTP_COMM_LOST          SCTP_COMM_LOST
#define LINUX_SCTP_RESTART            SCTP_RESTART
#define LINUX_SCTP_SHUTDOWN_COMP      SCTP_SHUTDOWN_COMP
#define LINUX_SCTP_CANT_STR_ASSOC     SCTP_CANT_STR_ASSOC

int vos_sock_sctp_recvmsg(int s, void *msg, size_t len, struct vos_sock_sockaddr *from,
		 socklen_t *fromlen, struct vos_sctp_sndrcvinfo*sinfo,
		 int *msg_flags)
 {
    int flags=0;
    if (msg_flags)
    {
        flags=*msg_flags;
    }
    struct linux_sockaddr fromaddr;
    int ret = ff_sctp_recvmsg(s,msg,len,(struct linux_sockaddr*)&fromaddr,fromlen, (struct sctp_sndrcvinfo* )sinfo,&flags);

    if (from)
    {
        memcpy(from,&fromaddr,*fromlen);
    }

    if (ret>0)
    {
        if (flags & FREEBSD_MSG_NOTIFICATION)
        {
            if (msg_flags)
            {
                *msg_flags|=LINUX_MSG_NOTIFICATION;
            }

            union sctp_notification *not =
                (union sctp_notification *)msg;

            switch(not->sn_header.sn_type) {
            case FREEBSD_SCTP_ASSOC_CHANGE:
            {
                not->sn_header.sn_type=LINUX_SCTP_ASSOC_CHANGE;
                if (not->sn_assoc_change.sac_state == FREEBSD_SCTP_COMM_UP) {
                    not->sn_assoc_change.sac_state=LINUX_SCTP_COMM_UP;
                } else if (not->sn_assoc_change.sac_state == FREEBSD_SCTP_SHUTDOWN_COMP){
                    not->sn_assoc_change.sac_state=LINUX_SCTP_SHUTDOWN_COMP;
                } else if (not->sn_assoc_change.sac_state == FREEBSD_SCTP_COMM_LOST){
                    not->sn_assoc_change.sac_state=LINUX_SCTP_COMM_LOST;
                }
                else
                {
                }
            }
                break;
            case FREEBSD_SCTP_SHUTDOWN_EVENT:
                not->sn_header.sn_type=LINUX_SCTP_SHUTDOWN_EVENT;
                break;

            case FREEBSD_SCTP_SEND_FAILED:
                not->sn_header.sn_type=LINUX_SCTP_SEND_FAILED;
                break;

            case FREEBSD_SCTP_PEER_ADDR_CHANGE:
                not->sn_header.sn_type=LINUX_SCTP_PEER_ADDR_CHANGE;
                break;

            case FREEBSD_SCTP_REMOTE_ERROR:
                not->sn_header.sn_type=LINUX_SCTP_REMOTE_ERROR;
                break;

            default :
                break;
            }


        }
        if (flags & FREEBSD_MSG_EOR)
        {
            if (msg_flags)
            {
                *msg_flags|=LINUX_MSG_EOR;
            }

        }
    }

    return ret;
 }

int vos_sock_getnamefo(const struct vos_sock_sockaddr *sockaddr, socklen_t addrlen, 
                     char *host, socklen_t hostlen,
                     char *serv, socklen_t servlen, int flags)
{
    return ff_getnamefo((struct linux_sockaddr *)sockaddr, 
        addrlen, 
        host,
        hostlen, 
        serv,
        servlen,
        flags);
}
