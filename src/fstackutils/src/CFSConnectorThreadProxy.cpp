/***********************************************************************
 * Copyright (C) 2016-2018, Nanjing StarOS Technology Co., Ltd 
**********************************************************************/

#include "CFSConnectorThreadProxy.h"
#include "CFSTransportThreadProxy.h"

namespace fsutils
{
CFSConnectorThreadProxy::
CFSConnectorThreadProxy(IFSConnectionManager::CType aType, 
    IPSReactor *aThreadNetwork,
                CAWThread *aThreadUser)
    : CFSAcceptorConnectorSinkThreadProxyT<CFSConnectorThreadProxy>(this)
    , m_pThreadUser(aThreadUser)
    , m_pThreadNetwork(aThreadNetwork)
    , m_Type(aType)
{
    if (!m_pThreadNetwork) {
        //m_pThreadNetwork = CAWThreadManager::Instance()->GetThread(CAWThreadManager::TT_NETWORK);
        CAW_ASSERTE(m_pThreadNetwork);
    }
    if (!m_pThreadUser) {
        //m_pThreadUser = CAWThreadManager::Instance()->GetThread(CAWThreadManager::TT_CURRENT);
        CAW_ASSERTE(m_pThreadUser);
    }
}

CFSConnectorThreadProxy::~CFSConnectorThreadProxy()
{
    // the current thread is network thread due to <m_pConActual>.
    //	CAW_ASSERTE(CAWThreadManager::IsEqualCurrentThread(m_pThreadNetwork->GetThreadId()));
}

DWORD CFSConnectorThreadProxy::AddReference()
{
    return CAWReferenceControlMutilThread::AddReference();
}

DWORD CFSConnectorThreadProxy::ReleaseReference()
{
    return CAWReferenceControlMutilThread::ReleaseReference();
}

void CFSConnectorThreadProxy::OnReferenceDestory()
{
    // this assert helps to debug that the upper layer 
    // didn't call CancelConnect() before.
    CAW_ASSERTE(CAWStopFlag::m_bStoppedFlag.Value());

    // only the user thread can delete this due to <m_pConActual>,
    // so we have to post delete event to the network thread.
    if (CAWThreadManager::IsEqualCurrentThread(m_pThreadUser->GetThreadId())) {
    //CAW_INFO_TRACE_THIS("CAWTransportThreadProxy::OnReferenceDestory,"
    //" post delete event to the network thread");
    CAWEventDeleteT<CFSConnectorThreadProxy> *pEventDelete;
    pEventDelete = new CAWEventDeleteT<CFSConnectorThreadProxy>(this);
    if (pEventDelete)
        pEventDelete->Launch(m_pThreadNetwork);
    }
    else {
        delete this;
    }
}

BOOL CFSConnectorThreadProxy::IsConnector()
{
	return TRUE;
}

void CFSConnectorThreadProxy::SetStopFlag()
{
    CFSAcceptorConnectorSinkThreadProxyT<CFSConnectorThreadProxy>::ResetSink(NULL);
    CAWStopFlag::SetStopFlag();
}

void CFSConnectorThreadProxy::
AsycConnect(IFSAcceptorConnectorSink *aSink,  
            const CAWInetAddr &aAddrPeer, 
            CAWTimeValue *aTimeout,
            CAWInetAddr *aAddrLocal)
{
    CAW_ASSERTE(CAWThreadManager::IsEqualCurrentThread(m_pThreadUser->GetThreadId()));
    CAW_ASSERTE(CAWStopFlag::IsFlagStopped());

    // Don't check m_pConActual due to its operations are all in network thread.
    //	CAW_ASSERTE(!m_pConActual);

    CAW_INFO_TRACE_THIS("CFSConnectorThreadProxy::AsycConnect,"
        " aSink=" << aSink <<
        " addr=" << aAddrPeer.GetIpDisplayName() << 
        " sec=" << (aTimeout ? aTimeout->GetSec() : -1) << 
        " usec=" << (aTimeout ? aTimeout->GetUsec() : -1));

    CAW_ASSERTE(!CFSAcceptorConnectorSinkThreadProxyT<CFSConnectorThreadProxy>::m_pSinkActual);
    CAW_ASSERTE(aSink);
    CFSAcceptorConnectorSinkThreadProxyT<CFSConnectorThreadProxy>::ResetSink(aSink);

    CAWResult rv = CAW_ERROR_OUT_OF_MEMORY;
    CFSEventAsycConnect *pEvent = new CFSEventAsycConnect(this, this, aAddrPeer, aTimeout, aAddrLocal);
    if (pEvent) {
        rv = m_pThreadNetwork->PostEvent(pEvent);
    }

    if (CAW_SUCCEEDED(rv))
        CAWStopFlag::SetStartFlag();

    //OnConnecteIndication(FAILED) if CAW_FAILED(rv)
    CAW_ASSERTE(CAW_SUCCEEDED(rv));
}

void CFSConnectorThreadProxy::CancelConnect()
{
    CAW_ASSERTE(CAWThreadManager::IsEqualCurrentThread(m_pThreadUser->GetThreadId()));

    if (CAWStopFlag::IsFlagStopped())
        return;

    // invoke CFSConnectorThreadProxy::SetStopFlag() in order to ResetSink(NULL).
    //CAWStopFlag::SetStopFlag();
    CFSConnectorThreadProxy::SetStopFlag();

    CAW_INFO_TRACE_THIS("CFSConnectorThreadProxy::CancelConnect,");

    CFSEventCancelConnect *pEvent = new CFSEventCancelConnect(this);
    if (pEvent)
    {
        m_pThreadNetwork->PostEvent(pEvent);
    }
}


//////////////////////////////////////////////////////////////////////
// class CFSEventAsycConnect
//////////////////////////////////////////////////////////////////////

CFSEventAsycConnect::
CFSEventAsycConnect(CFSConnectorThreadProxy *aConnectorThreadProxy, 
                        IFSAcceptorConnectorSink *aSink, 
                        const CAWInetAddr &aAddrPeer, 
                        CAWTimeValue *aTimeout,
                        CAWInetAddr *aAddrLocal)
    : m_pOwnerThreadProxy(aConnectorThreadProxy)
    , m_pSink(aSink)
    , m_addrPeer(aAddrPeer)
    , m_pParaTimeout(NULL)
    , m_pParaAddrLocal(NULL)
{
    CAW_ASSERTE(m_pOwnerThreadProxy);
    if (aTimeout) {
        m_tvTimeout = *aTimeout;
        m_pParaTimeout = &m_tvTimeout;
    }
    if (aAddrLocal) {
        m_addrLocal = *aAddrLocal;
        m_pParaAddrLocal = &m_addrLocal;
    }
}

CFSEventAsycConnect::~CFSEventAsycConnect()
{
}

CAWResult CFSEventAsycConnect::OnEventFire()
{

    // we must new actual connector in the network thread.
    // <m_pConActual> may not be NULL due to OnConnectIndicatoin don't assign it to NULL.
    //	CAW_ASSERTE(!m_pOwnerThreadProxy->m_pConActual);

    IFSConnectionManager::CType type = m_pOwnerThreadProxy->m_Type;

    CAWResult rv = IFSConnectionManager::Instance()->CreateConnectionClient(type, 
        m_pOwnerThreadProxy->m_pConActual.ParaOut());
    if (CAW_FAILED(rv)) {
        CAW_ERROR_TRACE_THIS("CFSEventAsycConnect::OnEventFire,"
        	" can't create connector in the network thread. rv=" << rv);
        CAW_ASSERTE(FALSE);
        return rv;
    }

    if (m_pOwnerThreadProxy->m_pConActual) {
        m_pOwnerThreadProxy->m_pConActual->AsycConnect(m_pSink, m_addrPeer, m_pParaTimeout, m_pParaAddrLocal);
        return CAW_OK;
    }
    else
        return CAW_ERROR_NULL_POINTER;
}


//////////////////////////////////////////////////////////////////////
// class CEventCancelConnect
//////////////////////////////////////////////////////////////////////

CFSEventCancelConnect::
CFSEventCancelConnect(CFSConnectorThreadProxy *aConnectorThreadProxy)
    : m_pOwnerThreadProxy(aConnectorThreadProxy)
{
    CAW_ASSERTE(m_pOwnerThreadProxy);
}

CFSEventCancelConnect::~CFSEventCancelConnect()
{
}

CAWResult CFSEventCancelConnect::OnEventFire()
{

    CAW_ASSERTE(m_pOwnerThreadProxy->m_pConActual);
    if (m_pOwnerThreadProxy->m_pConActual) {
        m_pOwnerThreadProxy->m_pConActual->CancelConnect();
        m_pOwnerThreadProxy->m_pConActual = NULL;
        return CAW_OK;
    }
    else
        return CAW_ERROR_NULL_POINTER;
}
}//namespace fsutils
