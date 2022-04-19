/***********************************************************************
 * Copyright (C) 2016-2018, Nanjing StarOS Technology Co., Ltd 
**********************************************************************/

#include "CFSReactorEpoll.h"
namespace fsutils
{
CFSReactorEpoll::CFSReactorEpoll()
     :m_nEpollFd(-1)
{
    //printf("CFSReactorEpoll::CFSReactorEpoll");
}

CFSReactorEpoll::~CFSReactorEpoll()
{
    //printf("CFSReactorEpoll::~CFSReactorEpoll");
    if (m_nEpollFd>0)
    {
        vos_sock_close(m_nEpollFd);
        m_nEpollFd=-1;
    }
}

IAWReactor::ReactorType CFSReactorEpoll::GetReactorType()

{
    return REACTOR_TYPE_EPOLL;
}

CAWResult CFSReactorEpoll::Open()
{
    if (m_nEpollFd>0)
    {
        return CAW_OK;
    }
        /*The MAX_EVENTS is not the maximum size of the backing store*/
    CAWResult rv = CAW_ERROR_FAILURE;
    CAW_INFO_TRACE_THIS("CFSReactorEpoll::Open");
    m_nEpollFd = vos_epoll_create(0);
    if (m_nEpollFd < 0) {
        CAW_ERROR_TRACE("epoll_create,failure");
        goto fail;
    }
    CAW_INFO_TRACE_THIS("fs_epoll_create fd="<<m_nEpollFd);

    memset(m_pEpollEvents, 0, sizeof(m_pEpollEvents));
    
    
    rv = CFSReactorBase::Open();
    if (CAW_FAILED(rv))
    {
        CAW_ERROR_TRACE("CAWReactorBase open error");
        goto fail;
    }

    rv = m_Notify.Open(this);
    if (CAW_FAILED(rv))
    {
        CAW_ERROR_TRACE("m_Notify open error");
        goto fail;
    }

    CAWStopFlag::SetStartFlag();
    CAW_INFO_TRACE_THIS("CFSReactorEpoll::Open()");
    return CAW_OK;

fail:
    Close();
    CAW_ASSERTE(CAW_FAILED(rv));
    CAW_ERROR_TRACE_THIS("CFSReactorEpoll::Open, failed!"" rv=" << rv);
    return rv;
}

CAWResult CFSReactorEpoll::NotifyHandler(IPSEventHandler *aEh, IPSEventHandler::MASK aMask)
{
    return m_Notify.Notify(aEh,aMask);
}

CAWResult CFSReactorEpoll::RunEventLoop()
{
    if (m_nEpollFd == -1)
    {
        CAW_ASSERTE(0);
        return CAW_ERROR_FAILURE;
    }
    m_Est.EnsureSingleThread();
    while (!CAWStopFlag::IsFlagStopped()) {
        CAWTimeValue tvTimeout(CAWTimeValue::s_tvMax());
        if (m_pTimerQueue) {
            // process timer prior to wait event.
            m_pTimerQueue->CheckExpire(&tvTimeout);
        }

        int nfds = 0;
        long aToWaitTimeOutMillisec = tvTimeout.GetTotalInMsec();
        //printf("vos_epoll_wait aToWaitTimeOutMillisec=%lld\n", aToWaitTimeOutMillisec);
        if (aToWaitTimeOutMillisec == -1)
        {
            nfds = vos_epoll_wait(m_nEpollFd, m_pEpollEvents, MAX_EVENTS, -1);
        }
        else if (aToWaitTimeOutMillisec > 0)
        {
            nfds = vos_epoll_wait(m_nEpollFd, m_pEpollEvents, MAX_EVENTS, aToWaitTimeOutMillisec);
        }
        else if (aToWaitTimeOutMillisec == 0)
        {
            nfds = vos_epoll_wait(m_nEpollFd, m_pEpollEvents, MAX_EVENTS, aToWaitTimeOutMillisec);
        }
        //printf("vos_epoll_wait nfds=%d\n", nfds);
        if (nfds == 0)
        {
            ProcessHandleEvent(
                -1,
                IPSEventHandler::EVENTQUEUE_MASK,
                CAW_OK,
                FALSE);
            continue;
        }
        else if ((nfds == -1) && (errno == EINTR))
        {
            //no file descriptor became ready during the requested timeout  milliseconds
            return CAW_OK;
        }
        else if (nfds == -1)
        {
            CAW_ERROR_TRACE("CFSReactorEpoll::RunEventLoop, epoll_wait, failure=" << errno);
            return CAW_ERROR_FAILURE;
        }
        for (int n = 0; n < nfds; ++n)
        {
            CAWResult rvError = CAW_OK;
            IAWEventHandler::MASK maskSig = IAWEventHandler::NULL_MASK;
            int fdSig = m_pEpollEvents[n].data.fd;

            if (CAW_BIT_ENABLED(m_pEpollEvents[n].events, SO_EPOLLIN))
            {
                //maskSig = IAWEventHandler::READ_MASK | IAWEventHandler::ACCEPT_MASK
                //CAW_INFO_TRACE("CFSReactorEpoll::RunEventLoop, epoll_wait, EPOLLIN, fd="<<fdSig);
                maskSig |= IAWEventHandler::READ_MASK | IAWEventHandler::ACCEPT_MASK | IAWEventHandler::CONNECT_MASK;
            }
            if (CAW_BIT_ENABLED(m_pEpollEvents[n].events, SO_EPOLLOUT))
            {
                //CAW_INFO_TRACE("CFSReactorEpoll::RunEventLoop, epoll_wait, EPOLLOUT, fd="<<fdSig);
                maskSig |= IAWEventHandler::WRITE_MASK | IAWEventHandler::CONNECT_MASK;
                if (IsConnected(fdSig) == false)
                {
                    CAW_WARNING_TRACE("CFSReactorEpoll::RunEventLoop,"
                        " handle is closed."
                        " fd=" << fdSig);
                    rvError = CAW_ERROR_NETWORK_SOCKET_CLOSE;
                    CAW_SET_BITS(maskSig, IAWEventHandler::CLOSE_MASK);
                }
            }
            else
            {
                //CAW_INFO_TRACE("CFSReactorEpoll::RunEventLoop, epoll_wait, events="<<m_pEpollEvents[n].events<<", fd="<<fdSig);
            }
            ProcessHandleEvent(fdSig, maskSig, rvError, FALSE);
        }

        ProcessHandleEvent(
            -1,
            IPSEventHandler::EVENTQUEUE_MASK,
            CAW_OK,
            FALSE);

    }

    /*ShouldNotReachHere();*/
    CAW_ASSERTE(0);
    return CAW_OK;
}


bool CFSReactorEpoll::IsConnected(int fd)
{
    int error = 0;  
    socklen_t len = sizeof(int);  
    if (( 0 == vos_sock_getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len) ))
    {  
        if( 0 == error )
        {  
            return true;  
        }  
        else
        {  
            return false;
        } 
    }
    return false;
}


CAWResult CFSReactorEpoll::StopEventLoop()
{
    CAWStopFlag::m_bStoppedFlag = TRUE;
    m_Notify.Notify(&m_Notify, IAWEventHandler::NULL_MASK);
    return CAW_OK;
}

CAWResult CFSReactorEpoll::Close()
{
    m_Notify.Close();
    return CFSReactorBase::Close();
}

CAWResult CFSReactorEpoll::OnHandleRegister(int aFd, IPSEventHandler::MASK aMask, IPSEventHandler *aEh)
{
    if (m_nEpollFd == -1)
    {
        CAW_ASSERTE(0);
        return CAW_ERROR_FAILURE;
    }

    struct vos_epoll_event ev;
    if (aEh == NULL)
    {
        return CAW_ERROR_FAILURE;
    }

    ::memset(&ev, 0, sizeof(ev));

    int nVal = vos_sock_fcntl(aFd, VOS_F_GETFL, 0);
    if (nVal == -1)
    {
        CAW_ERROR_TRACE("CFSReactorEpoll::OnHandleRegister, fcntl error");
        return CAW_ERROR_FAILURE;
    }
    nVal |= VOS_O_NONBLOCK;
    if (vos_sock_fcntl(aFd, VOS_F_SETFL, nVal) == -1)
    {
        CAW_ERROR_TRACE("CFSReactorEpoll::OnHandleRegister, fcntl set noblock error");
        return CAW_ERROR_FAILURE;
    }

    int on = 1;
    vos_sock_ioctl(aFd, FIONBIO, &on);

    ev.data.fd = aFd;
        // READ, ACCEPT, and CONNECT flag will place the handle in the read set.
    if (CAW_BIT_ENABLED(aMask, IAWEventHandler::READ_MASK) || 
        CAW_BIT_ENABLED(aMask, IAWEventHandler::ACCEPT_MASK))
    {
        ev.events |= SO_EPOLLIN | SO_EPOLLET;
    }
    // WRITE and CONNECT flag will place the handle in the write set.
    if (CAW_BIT_ENABLED(aMask, IAWEventHandler::WRITE_MASK) || 
        CAW_BIT_ENABLED(aMask, IAWEventHandler::CONNECT_MASK))
    {
        ev.events |= SO_EPOLLOUT | SO_EPOLLET | SO_EPOLLERR | SO_EPOLLHUP;
    }


    if (vos_epoll_ctl(m_nEpollFd, SO_EPOLL_CTL_ADD, aFd, &ev) < 0)
    {
        CAW_ERROR_TRACE("CFSReactorEpoll::OnHandleRegister, failure, m_nEpollFd="<<m_nEpollFd<<",nHandle="<<aFd);
        return CAW_ERROR_FAILURE;
    }
    CAW_INFO_TRACE_THIS("CFSReactorEpoll::OnHandleRegister, fs_epoll_ctl ok!!!!, fd="<<aFd);

    return CAW_OK;
}

void CFSReactorEpoll::OnHandleRemoved(int  aFd)
{
    if (m_nEpollFd == -1)
    {
        CAW_ASSERTE(0);
        return;
    }
    struct vos_epoll_event ev;
     ::memset(&ev, 0, sizeof(ev));
     
    CAW_INFO_TRACE_THIS("CFSReactorEpoll::OnHandleRemoved, fs_epoll_ctl handle="<<aFd);

    ev.data.fd = aFd;
    
    if (vos_epoll_ctl(m_nEpollFd, SO_EPOLL_CTL_DEL, aFd, &ev) < 0)
    {
        CAW_ERROR_TRACE("CFSReactorEpoll::OnHandleRemoved, failure");
    }
}

}
