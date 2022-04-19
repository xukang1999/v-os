#ifndef _VOS_KEVENT_H
#define _VOS_KEVENT_H
#include "staros/so_init.h"
#ifdef __cplusplus
extern "C" {
#endif


#define VOS_EVFILT_READ        (-1)
#define VOS_EVFILT_WRITE       (-2)
#define VOS_EVFILT_AIO         (-3)    /* attached to aio requests */
#define VOS_EVFILT_VNODE       (-4)    /* attached to vnodes */
#define VOS_EVFILT_PROC        (-5)    /* attached to struct proc */
#define VOS_EVFILT_SIGNAL      (-6)    /* attached to struct proc */
#define VOS_EVFILT_TIMER       (-7)    /* timers */
#define VOS_EVFILT_PROCDESC    (-8)    /* attached to process descriptors */
#define VOS_EVFILT_FS          (-9)    /* filesystem events */
#define VOS_EVFILT_LIO         (-10)    /* attached to lio requests */
#define VOS_EVFILT_USER        (-11)    /* User events */
#define VOS_EVFILT_SENDFILE    (-12)    /* attached to sendfile requests */
#define VOS_EVFILT_EMPTY       (-13)    /* empty send socket buf */
#define VOS_EVFILT_SYSCOUNT    13

#define VOS_EV_SET(kevp_, a, b, c, d, e, f) do {	\
    struct kevent *kevp = (kevp_);      \
    (kevp)->ident = (a);            \
    (kevp)->filter = (b);           \
    (kevp)->flags = (c);            \
    (kevp)->fflags = (d);           \
    (kevp)->data = (e);         \
    (kevp)->udata = (f);            \
    (kevp)->ext[0] = 0;         \
    (kevp)->ext[1] = 0;         \
    (kevp)->ext[2] = 0;         \
    (kevp)->ext[3] = 0;         \
} while(0)

struct so_kevent {
    uint64_t ident;      /* identifier for this event */
    short filter;           /* filter for event */
    unsigned short flags;   /* action flags for kqueue */
    unsigned int fflags;    /* filter flag value */
	int64_t data;         /* filter data value */
    void *udata;            /* opaque user data identifier */
	uint64_t ext[4];      /* extensions */
};

SO_EXPORT int vos_kqueue(void);
SO_EXPORT int vos_kevent(int kq, const struct so_kevent *changelist, int nchanges, 
    struct so_kevent *eventlist, int nevents, const struct timespec *timeout);
SO_EXPORT int vos_kevent_do_each(int kq, const struct so_kevent *changelist, int nchanges, 
    void *eventlist, int nevents, const struct timespec *timeout, 
    void (*do_each)(void **, struct so_kevent *));

#ifdef __cplusplus
}
#endif
#endif

