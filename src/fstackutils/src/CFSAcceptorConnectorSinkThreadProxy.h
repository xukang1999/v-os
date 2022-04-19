/***********************************************************************
 * Copyright (C) 2016-2018, Nanjing StarOS Technology Co., Ltd 
**********************************************************************/
#ifndef CFSACCEPTORCONNECTORSINKTHREADPROXY_H
#define CFSACCEPTORCONNECTORSINKTHREADPROXY_H

#include "CFSTransportThreadProxy.h"

namespace fsutils
{

template <class ThreadProxyType>
class CFSEventOnConnectIndication : public IAWEvent
{
public:
    CFSEventOnConnectIndication(ThreadProxyType *aConnectorThreadProxy,
        CAWResult aReason,
        IFSTransport *aTrpt,
        IFSAcceptorConnectorId *aRequestId)
        : m_pOwnerThreadProxy(aConnectorThreadProxy)
        , m_Reason(aReason)
        , m_pTrpt(aTrpt)
        , m_pRequestId(aRequestId)
    {
        CAW_ASSERTE(m_pOwnerThreadProxy);
        CAW_INFO_TRACE_THIS("CFSEventOnConnectIndication::CFSEventOnConnectIndication");
    }

    virtual ~CFSEventOnConnectIndication()
    {
        CAW_INFO_TRACE_THIS("CFSEventOnConnectIndication::~CFSEventOnConnectIndication");
    }

    virtual CAWResult OnEventFire()
    {
        CAW_INFO_TRACE_THIS("CFSEventOnConnectIndication::OnEventFire");
        CAW_ASSERTE(CAWThreadManager::IsEqualCurrentThread(m_pOwnerThreadProxy->m_pThreadUser->GetThreadId()));
        if (m_pOwnerThreadProxy->IsFlagStopped()) {
            CAW_WARNING_TRACE_THIS("CFSAcceptorConnectorSinkThreadProxyT::"
                "CEventOnConnectIndication::OnEventFire, stopped."
                " m_pOwnerThreadProxy=" << m_pOwnerThreadProxy.Get());
            if (m_pTrpt)
                m_pTrpt->Disconnect(CAW_ERROR_NOT_INITIALIZED);
            return CAW_OK;
        }

        // SetStopFlag() will ResetSink to NULL, store it before.
        IFSAcceptorConnectorSink* pSink = m_pOwnerThreadProxy->m_pSinkActual;
        CAW_ASSERTE(pSink);

        /// Stop connector to avoid it CannelConnect again.
        /// It should do nothing to the acceptor.
        m_pOwnerThreadProxy->SetStopFlag();

        if (pSink) {
            pSink->OnConnectIndication(
                        m_Reason, 
                        m_pTrpt.ParaIn(), 
                        m_pRequestId);
            return CAW_OK;
        }
        else
            return CAW_ERROR_NULL_POINTER;
    }

private:
    CAWAutoPtr<ThreadProxyType> m_pOwnerThreadProxy;
    CAWResult m_Reason;
    CAWAutoPtr<IFSTransport> m_pTrpt;
    IFSAcceptorConnectorId *m_pRequestId;
};

template <class ThreadProxyType>
class CFSAcceptorConnectorSinkThreadProxyT
    : public IFSAcceptorConnectorSink
{
public:
    CFSAcceptorConnectorSinkThreadProxyT(ThreadProxyType *pThreadProxy)
        : m_pSinkActual(NULL)
        , m_pThreadProxy(pThreadProxy)
    {
        CAW_ASSERTE(m_pThreadProxy);
        CAW_INFO_TRACE_THIS("CFSAcceptorConnectorSinkThreadProxyT::CFSAcceptorConnectorSinkThreadProxyT");
    }

    virtual ~CFSAcceptorConnectorSinkThreadProxyT()
    {
        CAW_INFO_TRACE_THIS("CFSAcceptorConnectorSinkThreadProxyT::~CFSAcceptorConnectorSinkThreadProxyT");
    }

    void ResetSink(IFSAcceptorConnectorSink *aSink)
    {
        CAW_ASSERTE(CAWThreadManager::IsEqualCurrentThread(m_pThreadProxy->m_pThreadUser->GetThreadId()));
        m_pSinkActual = aSink;
    }

    // interface IAWAcceptorConnectorSink
    virtual void OnConnectIndication(
                            CAWResult aReason,
                            IFSTransport *aTrpt,
                            IFSAcceptorConnectorId *aRequestId)
    {
        CAW_INFO_TRACE_THIS("CFSAcceptorConnectorSinkThreadProxyT::OnConnectIndication");

        CFSTransportThreadProxy *pTransportThreadProxy = NULL;
        if (CAW_SUCCEEDED(aReason)) {
            pTransportThreadProxy = new CFSTransportThreadProxy(
                                            aTrpt, 
                                            m_pThreadProxy->m_pThreadNetwork, 
                                            m_pThreadProxy->m_pThreadUser,
                                            m_pThreadProxy->m_Type);
            if (!pTransportThreadProxy)
                aReason = CAW_ERROR_OUT_OF_MEMORY;
            else 
                //aReason = CAW_OK;
                aReason = aTrpt->OpenWithSink(pTransportThreadProxy);

            if (CAW_FAILED(aReason)) {
                delete pTransportThreadProxy;
                pTransportThreadProxy = NULL;

                if (!m_pThreadProxy->IsConnector()) {
                    CAW_WARNING_TRACE("CFSAcceptorConnectorSinkThreadProxyT::OnConnectIndication,"
                    " It's acceptor, don't callback.");
                    return;
                }
            }
        }

#ifdef CAW_DEBUG
        if (!pTransportThreadProxy) {
            CAW_ASSERTE(!aTrpt);
            CAW_ASSERTE(CAW_FAILED(aReason));
            //CAW_ASSERTE(m_pThreadProxy->IsConnector());
        }
#endif // CAW_DEBUG

        CFSEventOnConnectIndication<ThreadProxyType> *pEvent = new CFSEventOnConnectIndication<ThreadProxyType>(
                                                                                m_pThreadProxy, 
                                                                                aReason, 
                                                                                pTransportThreadProxy, 
                                                                                m_pThreadProxy);
        if (m_pThreadProxy)
        {
            if (m_pThreadProxy->m_pThreadUser)
            {
                if (m_pThreadProxy->m_pThreadUser->GetEventQueue())
                {
                    m_pThreadProxy->m_pThreadUser->GetEventQueue()->PostEvent(pEvent);
                }
                else 
                {
                    delete pEvent;
                    CAW_ERROR_TRACE("CFSAcceptorConnectorSinkThreadProxyT no m_pThreadProxy->m_pThreadUser->GetEventQueue()");
                }
            }
            else 
            {
                delete pEvent;
                CAW_ERROR_TRACE("CFSAcceptorConnectorSinkThreadProxyT no m_pThreadProxy->m_pThreadUser");
            }
        }
        else 
        {
            delete pEvent;
            CAW_ERROR_TRACE("CFSAcceptorConnectorSinkThreadProxyT no m_pThreadProxy");
        }
    }

protected:
    IFSAcceptorConnectorSink *m_pSinkActual;
    ThreadProxyType *m_pThreadProxy;
};
}//namespace fsutils
#endif // !CFSACCEPTORCONNECTORSINKTHREADPROXY_H

