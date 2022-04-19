#ifndef _VOS_EPOLL_H
#define _VOS_EPOLL_H
#include "staros/so_init.h"

#ifdef __cplusplus
extern "C" {
#endif


#define SO_EPOLLIN        0x001
#define SO_EPOLLPRI       0x002
#define SO_EPOLLOUT       0x004
#define SO_EPOLLERR       0x008
#define SO_EPOLLHUP       0x010
#define SO_EPOLLRDNORM    0x040
#define SO_EPOLLRDBAND    0x080
#define SO_EPOLLWRNORM    0x100
#define SO_EPOLLWRBAND    0x200
#define SO_EPOLLMSG       0x400

#define SO_EPOLLRDHUP     0x2000

#define SO_EPOLLEXCLUSIVE 0x10000000
#define SO_EPOLLONESHOT   0x40000000
#define SO_EPOLLET        0x80000000

#define SO_EPOLL_CTL_ADD  1
#define SO_EPOLL_CTL_DEL  2
#define SO_EPOLL_CTL_MOD  3


typedef union vos_epoll_data {
	void *ptr;
	int fd;
	uint32_t u32;
	uint64_t u64;
} vos_epoll_data_t;

struct vos_epoll_event {
	uint32_t events;      /* epoll event */
	vos_epoll_data_t data;      /* User data variable */
};


SO_EXPORT int vos_epoll_create(int size);
SO_EXPORT int vos_epoll_ctl(int epfd, int op, int fd, struct vos_epoll_event *event);
SO_EXPORT int vos_epoll_wait(int epfd, struct vos_epoll_event *events, int maxevents, int timeout);

#ifdef __cplusplus
}
#endif

#endif

