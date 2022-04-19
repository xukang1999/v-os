/***********************************************************************
 * Copyright (C) 2016-2018, Nanjing StarOS Technology Co., Ltd 
**********************************************************************/

#include "fstackutils/CFSInterface.h"
#include "CFSReactorNotifyPipe.h"
#include "CFSReactorBase.h"
#include "staros/staros.h"
#include "fstackutils/CFSSocket.h"
namespace fsutils
{
CFSReactorNotifyPipe::CFSReactorNotifyPipe()
    : m_pReactor(NULL)
{
}

CFSReactorNotifyPipe::~CFSReactorNotifyPipe()
{
    Close();
}

CAWResult CFSReactorNotifyPipe::Open(CFSReactorBase *aReactor)
{
    CAWResult rv = CAW_OK;
    //CAWIPCBase ipcNonblock;

    CAW_ASSERTE(!m_pReactor);
    m_pReactor = aReactor;
    CAW_ASSERTE_RETURN(m_pReactor, CAW_ERROR_INVALID_ARG);

    rv = m_PipeNotify.Open();
    if (CAW_FAILED(rv)) 
    {
        return rv;
    }

    int readfd = m_PipeNotify.GetReadHandle();
    int ret = vos_sock_fcntl(readfd, VOS_F_SETFL, 1);
    if (ret < 0)
    {
        printf("set noblock failure\n");
    }

    CAW_INFO_TRACE("CFSReactorNotifyPipe::Open");

    rv = m_pReactor->RegisterHandler(this, IAWEventHandler::READ_MASK);
    if (CAW_FAILED(rv)) 
    {
        Close();
        return rv;
    }
    else
    {
        return rv;
    }

}

int CFSReactorNotifyPipe::GetHandle() const 
{
    return m_PipeNotify.GetReadHandle();
}

int CFSReactorNotifyPipe::OnInput(int aFd)
{
    CAW_INFO_TRACE("CFSReactorNotifyPipe::OnInput fd="<<aFd);
    CAW_ASSERTE(aFd == m_PipeNotify.GetReadHandle());

    CBuffer bfNew;
    int nRecv = vos_sock_recv(
        (CAW_SOCKET)m_PipeNotify.GetReadHandle(), 
        (char*)&bfNew, sizeof(bfNew), 0);

    if (nRecv < (int)sizeof(bfNew)) {
        CAW_ERROR_TRACE("CFSReactorNotifyPipe::OnInput,"
        " nRecv=" << nRecv <<
        " fd=" << m_PipeNotify.GetReadHandle() << 
        " err=" << errno);

        return 0;
    }

    if (bfNew.m_Fd == m_PipeNotify.GetReadHandle())
        return 0;

    CAW_ASSERTE(m_pReactor);
    if (m_pReactor)
        m_pReactor->ProcessHandleEvent(bfNew.m_Fd, bfNew.m_Mask, CAW_OK, TRUE);
    return 0;
}

CAWResult CFSReactorNotifyPipe::
Notify(IPSEventHandler *aEh, IPSEventHandler::MASK aMask)
{

    CAW_INFO_TRACE("CFSReactorNotifyPipe::Notify, WriteHandle ="<<m_PipeNotify.GetWriteHandle());
    // this function can be invoked in the different thread.
    if (m_PipeNotify.GetWriteHandle() < 0) {
        CAW_WARNING_TRACE("CFSReactorNotifyPipe::Notify, WriteHandle INVALID.");
        return CAW_ERROR_NOT_INITIALIZED;
    }

    int fdNew = -1;
    if (aEh) {
        fdNew = aEh->GetHandle();
        CAW_ASSERTE(fdNew != -1);
    }

    CBuffer bfNew(fdNew, aMask);
    int nSend = vos_sock_send(
    (CAW_SOCKET)m_PipeNotify.GetWriteHandle(), 
    (char*)&bfNew, sizeof(bfNew), 0);
    CAW_INFO_TRACE("CFSReactorNotifyPipe::Notify, ps_send ="<<nSend);

    if (nSend < (int)sizeof(bfNew)) {
        CAW_ERROR_TRACE("CFSReactorNotifyPipe::Notify,"
        " nSend=" << nSend <<
        " fd=" << m_PipeNotify.GetWriteHandle() <<
        " err=" << errno);
        return CAW_ERROR_UNEXPECTED;
    }
    return CAW_OK;
}

CAWResult CFSReactorNotifyPipe::Close()
{
    if (m_pReactor) {
        m_pReactor->RemoveHandler(this);
        m_pReactor = NULL;
    }
    return m_PipeNotify.Close();
}
}//namespace fsutils
