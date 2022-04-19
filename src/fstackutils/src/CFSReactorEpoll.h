/***********************************************************************
 * Copyright (C) 2016-2018, Nanjing StarOS Technology Co., Ltd 
**********************************************************************/

#ifndef CFSREACTOREPOLL_H
#define CFSREACTOREPOLL_H
#include "CFSReactorBase.h"
#include "CFSReactorNotifyPipe.h"
#include "staros/staros.h"

namespace fsutils
{
static const int MAX_EVENTS = 1024;

class CFSReactorEpoll : public CFSReactorBase  
{
public:
    CFSReactorEpoll();
    virtual ~CFSReactorEpoll();

    virtual IAWReactor::ReactorType GetReactorType();

    // interface IAWReactor
    virtual CAWResult Open();

    virtual CAWResult NotifyHandler(IPSEventHandler *aEh, IPSEventHandler::MASK aMask);

    virtual CAWResult RunEventLoop();

    virtual CAWResult StopEventLoop();

    virtual CAWResult Close();
    bool IsConnected(int fd);

protected:
    virtual CAWResult OnHandleRegister(int aFd, 
                        IPSEventHandler::MASK aMask, 
        IPSEventHandler *aEh);
    virtual void OnHandleRemoved(int aFd);

private:
    int                     m_nEpollFd;
    CFSReactorNotifyPipe    m_Notify;
    struct vos_epoll_event      m_pEpollEvents[MAX_EVENTS];
};
}//namespace fsutils
#endif // !CFSREACTOREPOLL_H

