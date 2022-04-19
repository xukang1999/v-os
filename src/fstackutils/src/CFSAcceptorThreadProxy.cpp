/***********************************************************************
 * Copyright (C) 2016-2018, Nanjing StarOS Technology Co., Ltd 
**********************************************************************/

#include "CFSAcceptorThreadProxy.h"
namespace fsutils
{
CFSAcceptorThreadProxy::
CFSAcceptorThreadProxy(IFSConnectionManager::CType aType,
    IPSReactor *aThreadNetwork,
                                    CAWThread *aThreadUser)
    : CFSAcceptorConnectorSinkThreadProxyT<CFSAcceptorThreadProxy>(this)
    , m_pThreadUser(aThreadUser)
    , m_pThreadNetwork(aThreadNetwork)
    , m_Type(aType)
{
    CAW_INFO_TRACE_THIS("CFSAcceptorThreadProxy::CFSAcceptorThreadProxy");

    if (!m_pThreadNetwork) {
        //m_pThreadNetwork = CAWThreadManager::Instance()->GetThread(CAWThreadManager::TT_NETWORK);
        CAW_ASSERTE(m_pThreadNetwork);
    }
    if (!m_pThreadUser) {
        //m_pThreadUser = CAWThreadManager::Instance()->GetThread(CAWThreadManager::TT_CURRENT);
        CAW_ASSERTE(m_pThreadUser);
    }
}

CFSAcceptorThreadProxy::~CFSAcceptorThreadProxy()
{
    CAW_INFO_TRACE_THIS("CFSAcceptorThreadProxy::~CFSAcceptorThreadProxy");

// the current thread is network thread due to <m_pAcceptorActual>.
//CAW_ASSERTE(CAWThreadManager::IsEqualCurrentThread(m_pThreadNetwork->GetThreadId()));
}

DWORD CFSAcceptorThreadProxy::AddReference()
{
    return CAWReferenceControlMutilThread::AddReference();
}

DWORD CFSAcceptorThreadProxy::ReleaseReference()
{
    return CAWReferenceControlMutilThread::ReleaseReference();
}

void CFSAcceptorThreadProxy::OnReferenceDestory()
{
    // this assert helps to debug that the upper layer 
    // didn't call StopListen() before.
    CAW_ASSERTE(CAWStopFlag::m_bStoppedFlag.Value());

    // only the user thread can delete this due to <m_pAcceptorActual>,
    // so we have to post delete event to the network thread.
    if (CAWThreadManager::IsEqualCurrentThread(m_pThreadUser->GetThreadId())) {
        CAWEventDeleteT<CFSAcceptorThreadProxy> *pEventDelete = new CAWEventDeleteT<CFSAcceptorThreadProxy>(this);
        if (pEventDelete)
        {
            pEventDelete->Launch(m_pThreadNetwork);
        }
    }
    else 
    {
        delete this;
    }
}

BOOL CFSAcceptorThreadProxy::IsConnector()
{
    return FALSE;
}

CAWResult CFSAcceptorThreadProxy::StartListen(IFSAcceptorConnectorSink *aSink, const CAWInetAddr &aAddrListen)
{
    CAW_ASSERTE(CAWThreadManager::IsEqualCurrentThread(m_pThreadUser->GetThreadId()));
    CAW_ASSERTE_RETURN(CAWStopFlag::IsFlagStopped(), CAW_ERROR_ALREADY_INITIALIZED);

    // Don't check m_pAcceptorActual due to its operations are all in network thread.
    //	CAW_ASSERTE(!m_pAcceptorActual);

    CAW_INFO_TRACE_THIS("CFSAcceptorThreadProxy::StartListen,"
        " aSink=" << aSink <<
        " addr=" << aAddrListen.GetIpDisplayName());

    CAW_ASSERTE(!CFSAcceptorConnectorSinkThreadProxyT<CFSAcceptorThreadProxy>::m_pSinkActual);
    CAW_ASSERTE_RETURN(aSink, CAW_ERROR_INVALID_ARG);
    CFSAcceptorConnectorSinkThreadProxyT<CFSAcceptorThreadProxy>::ResetSink(aSink);

    CAWResult rv = CAW_ERROR_OUT_OF_MEMORY;
    CFSEventStartListen *pEvent = new CFSEventStartListen(this, this, aAddrListen);
    if (pEvent) {
    // we must use SendEvent() to get the listen result.
        rv = m_pThreadNetwork->SendEvent(pEvent);
    }

    if (CAW_FAILED(rv)) {
        CAW_WARNING_TRACE_THIS("CFSAcceptorThreadProxy::StartListen, SendEvent() failed.");
        CFSAcceptorConnectorSinkThreadProxyT<CFSAcceptorThreadProxy>::ResetSink(NULL);
    }
    else 
        CAWStopFlag::SetStartFlag();
    return rv;
}

CAWResult CFSAcceptorThreadProxy::StopListen(CAWResult aReason)
{
    CAW_ASSERTE(CAWThreadManager::IsEqualCurrentThread(m_pThreadUser->GetThreadId()));
    if (CAWStopFlag::IsFlagStopped())
    {
        CAW_INFO_TRACE_THIS("CFSAcceptorThreadProxy::StopListen, IsFlagStopped");

        return CAW_OK;
    }
    CAWStopFlag::SetStopFlag();

    CAW_INFO_TRACE_THIS("CFSAcceptorThreadProxy::StopListen, aReason=" << aReason);

    CFSAcceptorConnectorSinkThreadProxyT<CFSAcceptorThreadProxy>::ResetSink(NULL);

    CFSEventStopListen *pEvent = new CFSEventStopListen(this, aReason);
    if (pEvent) {
#if 0
    // TODO : SendEvent() or PostEvent() when network thread is main thread?
    // If use SendEvent(), StartListen() will be invoked 
    // successfully if it is invoked after this function at once.
    // But the UDP transport will not SendData() successfully if 
    // the UDP socket is closed in UDP acceptor.
    if (CAWThreadManager::IsThreadEqual(m_pThreadUser->GetThreadId(), m_pThreadNetwork->GetThreadId()))
    return m_pThreadNetwork->GetEventQueue()->PostEvent(pEvent);
    else
    return m_pThreadNetwork->GetEventQueue()->SendEvent(pEvent);
#else
    // use PostEvent() in spite of network thread is main thread or serparator thread.
        return m_pThreadNetwork->PostEvent(pEvent);
#endif
    }
    else 
        return CAW_ERROR_OUT_OF_MEMORY;
}


//////////////////////////////////////////////////////////////////////
// class CFSEventStartListen
//////////////////////////////////////////////////////////////////////

CFSEventStartListen::
CFSEventStartListen(CFSAcceptorThreadProxy *aThreadProxy, 
                            IFSAcceptorConnectorSink *aSink, 
                            const CAWInetAddr &aAddrListen)
    : m_pOwnerThreadProxy(aThreadProxy)
    , m_pSink(aSink)
    , m_addrListen(aAddrListen)
{
    CAW_INFO_TRACE_THIS("CFSEventStartListen::CFSEventStartListen");
}

CFSEventStartListen::~CFSEventStartListen()
{
    CAW_INFO_TRACE_THIS("CFSEventStartListen::~CFSEventStartListen");

}

CAWResult CFSEventStartListen::OnEventFire()
{
    CAW_INFO_TRACE_THIS("CFSEventStartListen::OnEventFire");

    // we must new actual acceptor in the network thread.
    CAW_ASSERTE(!m_pOwnerThreadProxy->m_pAcceptorActual);
    IFSConnectionManager::CType type = m_pOwnerThreadProxy->m_Type;

    CAWResult rv = IFSConnectionManager::Instance()->CreateConnectionServer(
                                                type, 
                                                m_pOwnerThreadProxy->m_pAcceptorActual.ParaOut());
    if (CAW_FAILED(rv)) {
        CAW_ERROR_TRACE_THIS("CFSEventStartListen::OnEventFire, can't create acceptor in the network thread. rv=" << rv);
        return rv;
    }

    rv = m_pOwnerThreadProxy->m_pAcceptorActual->StartListen(m_pSink, m_addrListen);
    if (CAW_FAILED(rv))
    {
        m_pOwnerThreadProxy->m_pAcceptorActual = NULL;
    }
    return rv;
}


//////////////////////////////////////////////////////////////////////
// class CFSEventStopListen
//////////////////////////////////////////////////////////////////////

CFSEventStopListen::CFSEventStopListen(CFSAcceptorThreadProxy *aConnectorThreadProxy,CAWResult aReason)
    : m_pOwnerThreadProxy(aConnectorThreadProxy)
    , m_Reason(aReason)
{
    CAW_ASSERTE(m_pOwnerThreadProxy);
    CAW_INFO_TRACE_THIS("CFSEventStopListen::CFSEventStopListen");
}

CFSEventStopListen::~CFSEventStopListen()
{
    CAW_INFO_TRACE_THIS("CFSEventStopListen::~CFSEventStopListen");
}

CAWResult CFSEventStopListen::OnEventFire()
{
    CAW_INFO_TRACE_THIS("CFSEventStopListen::OnEventFire");

    CAW_ASSERTE(m_pOwnerThreadProxy->m_pAcceptorActual);
    if (m_pOwnerThreadProxy->m_pAcceptorActual) 
    {
        CAWResult rv = m_pOwnerThreadProxy->m_pAcceptorActual->StopListen(m_Reason);
        m_pOwnerThreadProxy->m_pAcceptorActual = NULL;
        return rv;
    }
    else
    {
        return CAW_ERROR_NULL_POINTER;
    }
}
}//namespace fsutils
