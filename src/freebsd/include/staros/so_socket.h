/**********************************************************************************
 * Copyright (C) 2013-2015, Nanjing WFNEX Technology Co., Ltd. All rights reserved.
***********************************************************************************/
#ifndef __SO_SOCKET_H__
#define __SO_SOCKET_H__
#include "staros/so_init.h"
#ifdef __cplusplus
extern "C" {
#endif

#define VOS_SOCK_MAX_IFNAME_LEN					32
#define VOS_SCTP_ALIGN_RESV_PAD					92
/*sockaddr地址*/
struct vos_sock_sockaddr {
    short sa_family;
    char sa_data[14];
};

struct vos_sctp_sndrcvinfo {
	uint16_t sinfo_stream;
	uint16_t sinfo_ssn;
	uint16_t sinfo_flags;
	uint32_t sinfo_ppid;
	uint32_t sinfo_context;
	uint32_t sinfo_timetolive;
	uint32_t sinfo_tsn;
	uint32_t sinfo_cumtsn;
	uint32_t sinfo_assoc_id;
	uint16_t sinfo_keynumber;
	uint16_t sinfo_keynumber_valid;
	uint8_t __reserve_pad[VOS_SCTP_ALIGN_RESV_PAD];
};


/* POSIX-LIKE api begin */
SO_EXPORT int vos_sock_fcntl(int s, int cmd, int val);
SO_EXPORT int vos_sock_sysctl(const int *name, u_int namelen, void *oldp, size_t *oldlenp,const void *newp, size_t newlen);
SO_EXPORT int vos_sock_ioctl(int s, long cmd, void *argp);
SO_EXPORT int vos_sock_socket(int domain, int type, int protocol);
SO_EXPORT int vos_sock_setsockopt(int s, int level, int optname, const void *optval,socklen_t optlen);
SO_EXPORT int vos_sock_getsockopt(int s, int level, int optname, void *optval,socklen_t *optlen);
SO_EXPORT int vos_sock_listen(int s, int backlog);
SO_EXPORT int vos_sock_bind(int s, const struct vos_sock_sockaddr *addr, socklen_t addrlen);
SO_EXPORT int vos_sock_accept(int s, struct vos_sock_sockaddr *addr, socklen_t *addrlen);
SO_EXPORT int vos_sock_connect(int s, const struct vos_sock_sockaddr *name, socklen_t namelen);
SO_EXPORT int vos_sock_close(int fd);
SO_EXPORT int vos_sock_shutdown(int s, int how);
SO_EXPORT int vos_sock_getpeername(int s, struct vos_sock_sockaddr *name,socklen_t *namelen);
SO_EXPORT int vos_sock_getsockname(int s, struct vos_sock_sockaddr *name,socklen_t *namelen);
SO_EXPORT ssize_t vos_sock_read(int d, void *buf, size_t nbytes);
SO_EXPORT ssize_t vos_sock_readv(int fd, const struct iovec *iov, int iovcnt);
SO_EXPORT ssize_t vos_sock_write(int fd, const void *buf, size_t nbytes);
SO_EXPORT ssize_t vos_sock_writev(int fd, const struct iovec *iov, int iovcnt);
SO_EXPORT ssize_t vos_sock_send(int s, const void *buf, size_t len, int flags);
SO_EXPORT ssize_t vos_sock_sendto(int s, const void *buf, size_t len, int flags,const struct vos_sock_sockaddr *to, socklen_t tolen);
SO_EXPORT ssize_t vos_sock_sendmsg(int s, const struct msghdr *msg, int flags);
SO_EXPORT ssize_t vos_sock_recv(int s, void *buf, size_t len, int flags);
SO_EXPORT ssize_t vos_sock_recvfrom(int s, void *buf, size_t len, int flags,struct vos_sock_sockaddr *from, socklen_t *fromlen);
SO_EXPORT ssize_t vos_sock_recvmsg(int s, struct msghdr *msg, int flags);
SO_EXPORT int vos_sock_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,struct timeval *timeout);
SO_EXPORT int vos_sock_poll(struct pollfd fds[], nfds_t nfds, int timeout);
SO_EXPORT int vos_sock_gettimeofday(struct timeval *tv, struct timezone *tz);			
SO_EXPORT int vos_sock_fdisused(int fd);
SO_EXPORT int vos_sock_getmaxfd(void);
SO_EXPORT int vos_sock_socketpair(int domain, int type, int protocol, int rsv[2]);
/*sctp */
SO_EXPORT int
vos_sock_sctp_sendmsg(int s, void *msg, size_t len, struct vos_sock_sockaddr *to,
	     socklen_t tolen, uint32_t ppid, uint32_t flags,
	     uint16_t stream_no, uint32_t timetolive, uint32_t context);
SO_EXPORT int vos_sock_sctp_recvmsg(int s, void *msg, size_t len, struct vos_sock_sockaddr *from,
		 socklen_t *fromlen, struct vos_sctp_sndrcvinfo *sinfo,
		 int *msg_flags);
		 
SO_EXPORT int vos_sock_getnamefo(const struct vos_sock_sockaddr *sockaddr, socklen_t addrlen, 
                     char *host, socklen_t,
                     char *serv, socklen_t, int flags);

					 
#ifdef __cplusplus
}
#endif
#endif