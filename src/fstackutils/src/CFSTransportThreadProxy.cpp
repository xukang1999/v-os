/***********************************************************************
 * Copyright (C) 2016-2018, Nanjing StarOS Technology Co., Ltd 
**********************************************************************/
#include "CFSTransportThreadProxy.h"

namespace fsutils
{
CFSTransportThreadProxy::
CFSTransportThreadProxy(IFSTransport *aActual, 
    IPSReactor *aThreadNetwork,
                            CAWThread *aThreadUser,
                            IFSConnectionManager::CType aType)
    : m_pTransportActual(aActual)
    , m_pSinkActual(NULL)
    , m_pThreadUser(aThreadUser)
    , m_pThreadNetwork(aThreadNetwork)
    , m_Type(aType)
    , m_bIsBufferFull(FALSE)
    , m_bNeedOnSend(FALSE)
    , m_bPartialDataLastTime(FALSE)
{
    CAW_ASSERTE(m_pTransportActual);
    CAW_ASSERTE(m_pThreadUser);
    CAW_ASSERTE(m_pThreadNetwork);
    // we set start flag here to allow upper layer not invoke OpenWithSink.
    CAWStopFlag::SetStartFlag();
    CAWStopFlag::m_Est.Reset2ThreadId(m_pThreadUser->GetThreadId());
    m_isOnRecieve=FALSE;
    //printf("CFSTransportThreadProxy::CFSTransportThreadProxy %p\n", this);

    CAW_INFO_TRACE_THIS("CFSTransportThreadProxy::CFSTransportThreadProxy");
}

CFSTransportThreadProxy::~CFSTransportThreadProxy()
{
    // the current thread is network thread due to <m_pTransportActual>.
    if (m_pTransportActual) {
        m_pTransportActual->Disconnect(CAW_OK);
        m_pTransportActual = NULL;
    }
    //printf("CFSTransportThreadProxy::~CFSTransportThreadProxy %p\n", this);
    CAW_INFO_TRACE_THIS("CFSTransportThreadProxy::~CFSTransportThreadProxy");
}

DWORD CFSTransportThreadProxy::AddReference()
{
    //DWORD refcount = CAWReferenceControlMutilThread::GetReference();

    //printf("CFSTransportThreadProxy::AddReference() refcount=%d, m_isOnRecieve=%d, %p\n", refcount, m_isOnRecieve, this);

    return CAWReferenceControlMutilThread::AddReference();
}

DWORD CFSTransportThreadProxy::ReleaseReference()
{
    //CAW_INFO_TRACE_THIS("CFSTransportThreadProxy::ReleaseReference");
    DWORD refcount = CAWReferenceControlMutilThread::GetReference();
    if (refcount == 1)
    {
        CAWStopFlag::SetStopFlag();
    }
    //printf("CFSTransportThreadProxy::ReleaseReference() refcount=%d, m_isOnRecieve=%d, %p\n", refcount, m_isOnRecieve, this);
    return CAWReferenceControlMutilThread::ReleaseReference();
}
DWORD CFSTransportThreadProxy::GetRefCount()
{
    return  CAWReferenceControlMutilThread::GetReference();
}

void CFSTransportThreadProxy::OnReferenceDestory()
{
    // this assert helps to debug that the upper layer 
    // didn't call Disconnect() before.
    CAWStopFlag::SetStopFlag();
    //CAW_INFO_TRACE("CFSTransportThreadProxy::OnReferenceDestory, m_bStoppedFlag="<<CAWStopFlag::m_bStoppedFlag);
    CAW_ASSERTE(CAWStopFlag::m_bStoppedFlag.Value());

    m_TimerReference0.Schedule(this, CAWTimeValue::s_tvZero(), 1);
}

void CFSTransportThreadProxy::OnTimer(CAWTimerWrapperID* aId)
{
    CAWResult rv;
    CAW_ASSERTE(aId == &m_TimerReference0);

    // we must Cancel() timer in same thread as the Schedule().
    rv = m_TimerReference0.Cancel();
    CAW_INFO_TRACE_THIS("CFSTransportThreadProxy::OnTimer rv="<<rv);
    //printf("CFSTransportThreadProxy::OnTimer %p\n", this);

    CAW_ASSERTE(rv == CAW_ERROR_NOT_FOUND);

    // only the network thread can delete this due to <m_pTransportActual>,
    // so we have to post delete event to the network thread.
    if (CAWThreadManager::IsEqualCurrentThread(m_pThreadUser->GetThreadId())) {
        CAWEventDeleteT<CFSTransportThreadProxy> *pEventDelete;
        pEventDelete = new CAWEventDeleteT<CFSTransportThreadProxy>(this);
        if (pEventDelete) {
            rv = pEventDelete->Launch(m_pThreadNetwork);
        }
    }
    else {
        delete this;
    }
}

CAWResult CFSTransportThreadProxy::OpenWithSink(IFSTransportSink *aSink)
{
    CAW_ASSERTE_RETURN(aSink, CAW_ERROR_INVALID_ARG);
    CAW_ASSERTE(CAWThreadManager::IsEqualCurrentThread(m_pThreadUser->GetThreadId()));
    CAW_INFO_TRACE_THIS("CFSTransportThreadProxy::OpenWithSink");
    // we allow the upper layer invokes this function many times.
    CAW_ASSERTE(m_pSinkActual != aSink);
    m_pSinkActual = aSink;

    return CAW_OK;
}

IFSTransportSink* CFSTransportThreadProxy::GetSink()
{
    return m_pSinkActual;
}

CAWResult CFSTransportThreadProxy::SendData(CAWMessageBlock &aData, CFSTransportParameter *aPara)
{
    //printf("CFSTransportThreadProxy::SendData, %p\n", this);
    if (CAWStopFlag::IsFlagStopped())
        return CAW_OK;

    size_t dwTotal = aData.GetChainedLength();

    if (m_bIsBufferFull && !m_bPartialDataLastTime) {
        m_bNeedOnSend = TRUE;
        CAW_ERROR_TRACE_THIS("CFSTransportThreadProxy::SendData m_bIsBufferFull=true");
        return CAW_ERROR_PARTIAL_DATA;
    }

    CAWResult rv=CAW_ERROR_FAILURE;
    CFSEventSendData* pEvent = new CFSEventSendData(this, aData, aPara);
    rv = m_pThreadNetwork->PostEvent(pEvent);

    if (CAW_SUCCEEDED(rv)) {
        rv = aData.AdvanceChainedReadPtr(dwTotal);
        CAW_ASSERTE(CAW_SUCCEEDED(rv));
        if (aPara)
            aPara->m_dwHaveSent = dwTotal;
    }
    else {
        CAW_ERROR_TRACE_THIS("CFSTransportThreadProxy::SendData,"
            " PostEvent() failed!");
        CAW_ASSERTE(rv != CAW_ERROR_PARTIAL_DATA);
    }
    return rv;
}

CAWResult CFSTransportThreadProxy::SetOption(DWORD aCommand, LPVOID aArg)
{
    // this function should be invoked in the different threads.
    if (m_pTransportActual)
        return m_pTransportActual->SetOption(aCommand, aArg);
    else
        return CAW_ERROR_NULL_POINTER;
}

CAWResult CFSTransportThreadProxy::GetOption(DWORD aCommand, LPVOID aArg)
{
    // this function should be invoked in the different threads.
    if (!m_pTransportActual)
        return CAW_ERROR_NULL_POINTER;

    switch(aCommand)
    {
        case CAW_OPT_TRANSPORT_TRAN_TYPE: {
            DWORD dwTransType;
            CAWResult rv = m_pTransportActual->GetOption(aCommand, &dwTransType);
            if (CAW_SUCCEEDED(rv)) {
                IFSConnectionManager::CType *pType = static_cast<IFSConnectionManager::CType*>(aArg);
                CAW_ASSERTE_RETURN(pType, CAW_ERROR_INVALID_ARG);

                *pType = dwTransType;
            }
            return rv; 
        }

        case CAW_OPT_LOWER_TRANSPORT: 
            *(static_cast<IFSTransport**>(aArg)) = m_pTransportActual.Get();
            return CAW_OK;

        default:
            return m_pTransportActual->GetOption(aCommand, aArg);
    }
}

int CFSTransportThreadProxy::GetTransportHandle() const
{
    if (m_pTransportActual.Get())
    {
        return m_pTransportActual->GetTransportHandle();
    }
    else 
    {
        CAW_ASSERTE(0);
        return -1;
    }
}


CAWResult CFSTransportThreadProxy::Disconnect(CAWResult aReason)
{
    CAW_ASSERTE(CAWThreadManager::IsEqualCurrentThread(m_pThreadUser->GetThreadId()));
    // allow upper layer not invoke OpenWithSink before.
    CAW_INFO_TRACE_THIS("CFSTransportThreadProxy::Disconnect");
    if (CAWStopFlag::IsFlagStopped())
        return CAW_OK;
    CAWStopFlag::SetStopFlag();
    m_pSinkActual = NULL;

    //CAW_INFO_TRACE_THIS("CFSTransportThreadProxy::Disconnect,"
    //	" aReason=" << aReason << 
    //	" tran=" << static_cast<IAWTransport*>(this));

    CFSEventDisconnect *pEvent = new CFSEventDisconnect(this, aReason);
    if (pEvent == NULL)
    {
        return CAW_ERROR_FAILURE;
    }
    return m_pThreadNetwork->PostEvent(pEvent);
}

void CFSTransportThreadProxy::
OnReceive(CAWMessageBlock &aData, IFSTransport *aTrptId, 
         CFSTransportParameter *aPara)
{
    //printf("CFSTransportThreadProxy::OnReceive, this=%p\n", this);

    CAW_INFO_TRACE_THIS("CFSTransportThreadProxy::OnReceive, aDatasize="<<aData.GetChainedLength());
    if (CAWStopFlag::IsFlagStopped())
    {
        //printf("CFSTransportThreadProxy::OnReceive IsFlagStopped true");
        return;
    }

    CAW_ASSERTE(aTrptId == m_pTransportActual.Get());
    
    CFSEventOnReceive *pEvent2 = new CFSEventOnReceive(this, aData, this, aPara);
    if (pEvent2)
    {
        m_pThreadUser->GetEventQueue()->PostEvent(pEvent2);
    }
}

CAWResult CFSTransportThreadProxy::
Send_i(CAWMessageBlock *aData, CFSTransportParameter* aPara, BOOL aIsDuplicated)
{
    CAW_INFO_TRACE_THIS("CFSTransportThreadProxy::Send_i, aDatasize="<<aData->GetChainedLength());

    CAWResult rv = CAW_OK;
    CAWResult rvSend = CAW_OK;
    CAW_ASSERTE_RETURN(m_pTransportActual, CAW_ERROR_NULL_POINTER);
    // need not aData->DuplicateChained() if empty.
    if (m_SendBuffer.empty() && aData) {
        //printf("CFSTransportThreadProxy::Send_i m_pTransportActual->SendData,m_pTransportActual=%p\n", m_pTransportActual.Get());
        rv = m_pTransportActual->SendData(
                *aData, 
                aPara);
        if (CAW_SUCCEEDED(rv) ||
            CAW_BIT_ENABLED(m_Type, IFSConnectionManager::CTYPE_UDP)) 
        {
            // delete <aData> to avoid memery leak.
            if (aIsDuplicated)
                aData->DestroyChained();
            return rv;
        }
        CAW_ASSERTE(aData->GetChainedLength() > 0);
        // if failed, fall to next to buffer into the list.
        rvSend = rv;
    }
    //printf("CFSTransportThreadProxy::Send_i m_pTransportActual->SendData end\n");

    // UDP should not be buffered.
    CAW_ASSERTE(CAW_BIT_DISABLED(m_Type, IFSConnectionManager::CTYPE_UDP));

    if (aData) {
        //printf("CFSTransportThreadProxy::Send_i CItem itNew\n");

        CItem itNew(
            aIsDuplicated ? aData : aData->DuplicateChained(), 
            aPara);
        m_SendBuffer.push_back(itNew);
        // assign itNew.m_pMbSend to NULL to avoid being destroyed
        itNew.m_pMbSend = NULL;

        if (!aIsDuplicated) {
            // It is OK if SendData() a half when m_SendBuffer.empty().
            size_t dwTotal = aData->GetChainedLength();
            rv = aData->AdvanceChainedReadPtr(dwTotal);
            CAW_ASSERTE(CAW_SUCCEEDED(rv));
            if (aPara)
                aPara->m_dwHaveSent = dwTotal;
        }

        // mainly check for SendData() failed when m_SendBuffer.empty()
        if (CAW_FAILED(rvSend))
        goto fail;
    }

    while (!m_SendBuffer.empty()) {

        CFSTransportThreadProxy::CItem &itTop = m_SendBuffer.front();
        rvSend = m_pTransportActual->SendData(
        *(itTop.m_pMbSend), 
        itTop.m_pParaTransportParameter);

        if (CAW_FAILED(rvSend)) {
            CAW_ASSERTE(itTop.m_pMbSend->GetChainedLength() > 0);
            goto fail;
        }
        else {
            CAW_ASSERTE(itTop.m_pMbSend->GetChainedLength() == 0);
            m_SendBuffer.pop_front();
        }
    }
    m_bIsBufferFull = FALSE;
    m_bPartialDataLastTime = FALSE;
    //printf("CFSTransportThreadProxy::Send_i while m_SendBuffer.empty end\n");

    return CAW_OK;

fail:
    CAW_ASSERTE(CAW_FAILED(rvSend));
    if (rvSend != CAW_ERROR_PARTIAL_DATA) {
        CAW_ERROR_TRACE_THIS("CFSTransportThreadProxy::Send_i,"
        " SendData() failed. rvSend=" << rvSend);
        m_bPartialDataLastTime = FALSE;
    }
    else
    {
        m_bPartialDataLastTime = TRUE;
    }
    m_bIsBufferFull = TRUE;
    // return OK because we have buffered the data.
    return CAW_OK;
}

void CFSTransportThreadProxy::
OnSend(IFSTransport *aTrptId, CFSTransportParameter *aPara)
{
    CAW_INFO_TRACE_THIS("CFSTransportThreadProxy::OnSend");

    CAW_ASSERTE(aTrptId == m_pTransportActual.Get());
    if (CAWStopFlag::IsFlagStopped())
        return;

    if (CAW_BIT_ENABLED(m_Type, IFSConnectionManager::CTYPE_UDP))
        return;

    Send_i(NULL, NULL, FALSE);

    // Do not need OnSend to upper layer if buffer full.
    if (m_bIsBufferFull) {
        CAW_ASSERTE(!m_SendBuffer.empty());
        return;
    }

    CFSEventOnSend *pEvent = new CFSEventOnSend(this, this, aPara);
    if (pEvent)
    {
        m_pThreadUser->GetEventQueue()->PostEvent(pEvent);
    }
}

void CFSTransportThreadProxy::
OnDisconnect(CAWResult aReason, IFSTransport *aTrptId)
{
    if (CAWStopFlag::IsFlagStopped())
        return;

    if(aTrptId != m_pTransportActual.Get())
    {
        CAW_ERROR_TRACE_THIS("CFSTransportThreadProxy::OnDisconnect input = " << aTrptId << " holder = " << m_pTransportActual.Get());
    }
    CAW_ASSERTE(aTrptId == m_pTransportActual.Get());

    if (m_pTransportActual) {
        m_pTransportActual->Disconnect(aReason);
    }

    CFSEventOnDisconnect *pEvent = new CFSEventOnDisconnect(this, aReason, this);
    if (pEvent)
    {
        if (m_pThreadUser)
        {
            IAWEventQueue *pqueue = m_pThreadUser->GetEventQueue();
            if (pqueue)
            {
                pqueue->PostEvent(pEvent);
            }
            else 
            {
                delete pEvent;
            }
        }
        else 
        {
            delete pEvent;
        }
    }
}


//////////////////////////////////////////////////////////////////////
// class CFSEventSendData
//////////////////////////////////////////////////////////////////////

CFSEventSendData::
CFSEventSendData(CFSTransportThreadProxy *aThreadProxy, 
       CAWMessageBlock &aData, 
       CFSTransportParameter *aPara)
    : m_pOwnerThreadProxy(aThreadProxy)
    , m_pParaTransportParameter(NULL)
{
    CAW_ASSERTE(m_pOwnerThreadProxy);
    m_pMessageBlock = aData.DuplicateChained();
    CAW_ASSERTE(m_pMessageBlock);
    if (aPara) {
        m_TransportParameter = *aPara;
        m_pParaTransportParameter = &m_TransportParameter;
    }
    CAW_INFO_TRACE_THIS("CFSEventSendData::CFSEventSendData");
}

CFSEventSendData::~CFSEventSendData()
{
    if (m_pMessageBlock) {
        m_pMessageBlock->DestroyChained();
        m_pMessageBlock = NULL;
    }
    CAW_INFO_TRACE_THIS("CFSEventSendData::~CFSEventSendData");
}

CAWResult CFSEventSendData::OnEventFire()
{
    CAW_INFO_TRACE_THIS("CFSEventSendData::OnEventFire");

    CAWResult rv = m_pOwnerThreadProxy->Send_i(
                m_pMessageBlock, 
                m_pParaTransportParameter, 
                TRUE);
    //yijian fix memory leak
    m_pMessageBlock = NULL;
    return rv;
}


//////////////////////////////////////////////////////////////////////
// class CFSTransportThreadProxy::CItem
//////////////////////////////////////////////////////////////////////

CFSTransportThreadProxy::CItem::
CItem(CAWMessageBlock *aMb, CFSTransportParameter *aPara)
    : m_pMbSend(aMb)
    , m_pParaTransportParameter(NULL)
{
    CAW_ASSERTE(m_pMbSend);
    if (aPara) {
        m_TransportParameter = *aPara;
        m_pParaTransportParameter = &m_TransportParameter;
    }
}

CFSTransportThreadProxy::CItem::~CItem()
{
    if (m_pMbSend)
    {
        m_pMbSend->DestroyChained();
        m_pMbSend=NULL;
    }
}

//////////////////////////////////////////////////////////////////////
// class CFSEventDisconnect
//////////////////////////////////////////////////////////////////////

CFSEventDisconnect::
CFSEventDisconnect(CFSTransportThreadProxy *aThreadProxy, 
        CAWResult aReason)
    : m_pOwnerThreadProxy(aThreadProxy)
    , m_Reason(aReason)
{
    CAW_ASSERTE(m_pOwnerThreadProxy);
    CAW_INFO_TRACE_THIS("CFSEventDisconnect::CFSEventDisconnect");
}

CFSEventDisconnect::~CFSEventDisconnect()
{
    CAW_INFO_TRACE_THIS("CFSEventDisconnect::~CFSEventDisconnect");
}

CAWResult CFSEventDisconnect::OnEventFire()
{
    CAW_INFO_TRACE_THIS("CFSEventDisconnect::OnEventFire");

    CAWResult rv = CAW_ERROR_NULL_POINTER;
    //m_pOwnerThreadProxy->Send_i(NULL, NULL, FALSE);
    if (m_pOwnerThreadProxy->m_pTransportActual) {
        rv = m_pOwnerThreadProxy->m_pTransportActual->Disconnect(m_Reason);
    }
    return rv;
}

CFSEventOpenWithSink::
CFSEventOpenWithSink(CFSTransportThreadProxy *aThreadProxy)
    : m_pOwnerThreadProxy(aThreadProxy)
{
    CAW_ASSERTE(m_pOwnerThreadProxy);
    CAW_INFO_TRACE_THIS("CFSEventOpenWithSink::CFSEventOpenWithSink");
}

CFSEventOpenWithSink::~CFSEventOpenWithSink()
{
    CAW_INFO_TRACE_THIS("CFSEventOpenWithSink::~CFSEventOpenWithSink");
}

CAWResult CFSEventOpenWithSink::OnEventFire()
{
    CAWResult rv = CAW_OK;
    CAW_INFO_TRACE_THIS("CFSEventOpenWithSink::OnEventFire");

    if (m_pOwnerThreadProxy->m_pTransportActual) {
        rv = m_pOwnerThreadProxy->m_pTransportActual->OpenWithSink(m_pOwnerThreadProxy.Get());
    }
    return rv;
}


//////////////////////////////////////////////////////////////////////
// class CFSEventOnReceive
//////////////////////////////////////////////////////////////////////

CFSEventOnReceive::
CFSEventOnReceive(CFSTransportThreadProxy *aThreadProxy, 
        CAWMessageBlock &aData, 
        IFSTransport *aTrptId, 
        CFSTransportParameter *aPara)
    : m_pOwnerThreadProxy(aThreadProxy)
    , m_pData(aData.DuplicateChained())
    , m_pTrptId(aTrptId)
    , m_pParaTransportParameter(NULL)
{
    CAW_ASSERTE(m_pData);
    CAW_ASSERTE(m_pOwnerThreadProxy);
    if (aPara) {
        m_TransportParameter = *aPara;
        m_pParaTransportParameter = &m_TransportParameter;
    }
    m_pOwnerThreadProxy->m_isOnRecieve=TRUE;
    CAW_INFO_TRACE_THIS("CFSEventOnReceive::CFSEventOnReceive");
    //printf("CFSEventOnReceive::CFSEventOnReceive %p, refounc=%d\n", m_pOwnerThreadProxy.Get(), m_pOwnerThreadProxy->GetRefCount());

}

CFSEventOnReceive::~CFSEventOnReceive()
{
    m_pOwnerThreadProxy->m_isOnRecieve=FALSE;

    CAW_ASSERTE(m_pData);

    if (m_pData) {
        m_pData->DestroyChained();
        m_pData = NULL;
    }
    //printf("CFSEventOnReceive::~CFSEventOnReceive %p, refounc=%d\n", m_pOwnerThreadProxy.Get(), m_pOwnerThreadProxy->GetRefCount());

    CAW_INFO_TRACE_THIS("CFSEventOnReceive::~CFSEventOnReceive");
}

CAWResult CFSEventOnReceive::OnEventFire()
{
    CAW_INFO_TRACE_THIS("CFSEventOnReceive::OnEventFire");
    CAW_ASSERTE(CAWThreadManager::IsEqualCurrentThread(m_pOwnerThreadProxy->m_pThreadUser->GetThreadId()));

    if (m_pOwnerThreadProxy->CAWStopFlag::IsFlagStopped()) {
        CAW_INFO_TRACE_THIS("CFSEventOnReceive::OnEventFire is stop");
        return CAW_OK;
    }
    if (m_pOwnerThreadProxy->m_pSinkActual && m_pData) {
        CAW_INFO_TRACE_THIS("CFSEventOnReceive::OnEventFire, m_pData="<<m_pData->GetChainedLength());
        m_pOwnerThreadProxy->m_pSinkActual->OnReceive(
            *m_pData, 
            m_pTrptId, 
            m_pParaTransportParameter);
        return CAW_OK;
    }
    else 
    {
        if (m_pOwnerThreadProxy->m_pSinkActual==NULL)
        {
            CAW_INFO_TRACE_THIS("CFSEventOnReceive::OnEventFire, m_pSinkActual is NULL, proxy="<<m_pOwnerThreadProxy);
        }
        else if (m_pData==NULL)
        {
            CAW_INFO_TRACE_THIS("CFSEventOnReceive::OnEventFire, m_Data is NULL");
        }
    }
    return CAW_ERROR_NULL_POINTER;
}

//////////////////////////////////////////////////////////////////////
// class CFSEventOnSend
//////////////////////////////////////////////////////////////////////

CFSEventOnSend::
CFSEventOnSend(CFSTransportThreadProxy *aThreadProxy, 
        IFSTransport *aTrptId, 
        CFSTransportParameter *aPara)
    : m_pOwnerThreadProxy(aThreadProxy)
    , m_pTrptId(aTrptId)
    , m_pParaTransportParameter(NULL)
{
    CAW_ASSERTE(m_pOwnerThreadProxy);
    if (aPara) {
        m_TransportParameter = *aPara;
        m_pParaTransportParameter = &m_TransportParameter;
    }
    CAW_INFO_TRACE_THIS("CFSEventOnSend::CFSEventOnSend");

}

CFSEventOnSend::~CFSEventOnSend()
{
    CAW_INFO_TRACE_THIS("CFSEventOnSend::~CFSEventOnSend");
}

CAWResult CFSEventOnSend::OnEventFire()
{
    CAW_INFO_TRACE_THIS("CFSEventOnSend::OnEventFire");

    CAW_ASSERTE(CAWThreadManager::IsEqualCurrentThread(m_pOwnerThreadProxy->m_pThreadUser->GetThreadId()));

    if (m_pOwnerThreadProxy->CAWStopFlag::IsFlagStopped()) {
        return CAW_OK;
    }

    if (m_pOwnerThreadProxy->m_pSinkActual) {
        if (!m_pOwnerThreadProxy->m_bNeedOnSend) {
            return CAW_OK;
        }
        m_pOwnerThreadProxy->m_bNeedOnSend = FALSE;

        m_pOwnerThreadProxy->m_pSinkActual->OnSend(
            m_pTrptId, 
            m_pParaTransportParameter);
        return CAW_OK;
    }
    return CAW_ERROR_NULL_POINTER;
}

//////////////////////////////////////////////////////////////////////
// class CFSEventOnDisconnect
//////////////////////////////////////////////////////////////////////

CFSEventOnDisconnect::
CFSEventOnDisconnect(CFSTransportThreadProxy *aThreadProxy, 
                                    CAWResult aReason,
                                    IFSTransport *aTrptId)
    : m_pOwnerThreadProxy(aThreadProxy)
    , m_Reason(aReason)
    , m_pTrptId(aTrptId)
{
    CAW_ASSERTE(m_pOwnerThreadProxy);
    CAW_INFO_TRACE_THIS("CFSEventDisconnect::CFSEventOnDisconnect");

}

CFSEventOnDisconnect::~CFSEventOnDisconnect()
{
    CAW_INFO_TRACE_THIS("CFSEventDisconnect::~CFSEventOnDisconnect");
}

CAWResult CFSEventOnDisconnect::OnEventFire()
{
    CAW_INFO_TRACE_THIS("CFSEventDisconnect::OnEventFire");

    CAW_ASSERTE(CAWThreadManager::IsEqualCurrentThread(m_pOwnerThreadProxy->m_pThreadUser->GetThreadId()));

    if (m_pOwnerThreadProxy->CAWStopFlag::IsFlagStopped()) {
        CAW_WARNING_TRACE_THIS("CFSEventOnDisconnect::OnEventFire, stopped."
        " m_pOwnerThreadProxy=" << m_pOwnerThreadProxy.Get());
        return CAW_OK;
    }
    m_pOwnerThreadProxy->CAWStopFlag::SetStopFlag();

    if (m_pOwnerThreadProxy->m_pSinkActual) {
        m_pOwnerThreadProxy->m_pSinkActual->OnDisconnect(
                                            m_Reason,
                                            m_pTrptId);
                                            return CAW_OK;
    }
    return CAW_ERROR_NULL_POINTER;
}

}//namespace fsutils
