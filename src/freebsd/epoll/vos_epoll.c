#include <sys/errno.h>
#include "staros/staros.h"
#include "compat/syscall_wrapper.h"
int
vos_epoll_create(int size __attribute__((__unused__)))
{
    return ff_kqueue();
}
#define kchanges 2
int
vos_epoll_ctl(int epfd, int op, int fd, struct vos_epoll_event *event)
{
    /*
     * Since kqueue uses EVFILT_READ and EVFILT_WRITE filters to
     * handle read/write events, so we need two kevents.
     */
    struct kevent kev[kchanges];
    int flags = 0;
    int read_flags, write_flags;

    if ((!event && op != SO_EPOLL_CTL_DEL) ||
        (op != SO_EPOLL_CTL_ADD &&
         op != SO_EPOLL_CTL_MOD &&
         op != SO_EPOLL_CTL_DEL)) {
        return -1;
    }

    /*
     * EPOLL_CTL_DEL doesn't need to care for event->events.
     */
    if (op == SO_EPOLL_CTL_DEL) {
        EV_SET(&kev[0], fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
        EV_SET(&kev[1], fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);

        return ff_kevent(epfd, kev, kchanges, NULL, 0, NULL);
    }

    /*
     * FIXME:
     *
     * Kqueue doesn't have edge-triggered mode that exactly
     * same with epoll, the most similar way is setting EV_CLEAR
     * or EV_DISPATCH flag, but there are still some differences.
     *
     * EV_CLEAR:after the event is retrieved by the user,
     *    its state is reset.
     * EV_DISPATCH: disable the event source immediately
     *    after delivery of an event.
     *
     * Here we use EV_CLEAR temporarily.
     *
     */
    if (event->events & SO_EPOLLET) {
        flags |= EV_CLEAR;
    }

    if (event->events & SO_EPOLLONESHOT) {
        flags |= EV_ONESHOT;
    }

    if (op == SO_EPOLL_CTL_ADD) {
        flags |= EV_ADD;
    }

    read_flags = write_flags = flags | EV_DISABLE;

    if (event->events & SO_EPOLLIN) {
        read_flags &= ~EV_DISABLE;
        read_flags |= EV_ENABLE;
    }

    if (event->events & SO_EPOLLOUT) {
        write_flags &= ~EV_DISABLE;
        write_flags |= EV_ENABLE;
    }

    // Fix #124: set user data
    EV_SET(&kev[0], fd, EVFILT_READ, read_flags, 0, 0, event->data.ptr);
    EV_SET(&kev[1], fd, EVFILT_WRITE, write_flags, 0, 0, event->data.ptr);

    return ff_kevent(epfd, kev, kchanges, NULL, 0, NULL);
}

static void 
vos_event_to_epoll(void **ev, struct kevent *kev)
{
    unsigned int event_one = 0;
    struct vos_epoll_event**ppev = (struct vos_epoll_event**)ev;

    if (kev->filter == EVFILT_READ) {
        if (kev->data || !(kev->flags & EV_EOF)) {
            event_one |= SO_EPOLLIN;
        }
    } else if (kev->filter == EVFILT_WRITE) {
        event_one |= SO_EPOLLOUT;
    }

    if (kev->flags & EV_ERROR) {
        event_one |= SO_EPOLLERR;
    }

    if (kev->flags & EV_EOF) {
        event_one |= SO_EPOLLHUP;

        if (kev->fflags) {
            event_one |= SO_EPOLLERR;
        }

        if (kev->filter == EVFILT_READ) {
            event_one |= SO_EPOLLIN;
        } else if (kev->filter == EVFILT_WRITE) {
            event_one |= SO_EPOLLERR;
        }
    }

    (*ppev)->events   = event_one;
    // Fix #124: get user data
    if (kev->udata != NULL)
        (*ppev)->data.ptr  = kev->udata;
    else
        (*ppev)->data.fd = kev->ident;
    (*ppev)++;
}

int 
vos_epoll_wait(int epfd, struct vos_epoll_event *events, int maxevents, int timeout)
{
    int i, ret;
    if (!events || maxevents < 1) {
        return -1;
    }
    if (timeout == -1)
    {
        return ff_kevent_do_each(epfd, NULL, 0, events, maxevents, NULL, vos_event_to_epoll);
    }
    else
    {
        struct timespec short_wait;
        short_wait.tv_sec = timeout / 1000;
        short_wait.tv_nsec = (timeout % 1000) * 1000 * 1000;
        return ff_kevent_do_each(epfd, NULL, 0, events, maxevents, &short_wait, vos_event_to_epoll);
    }
}

