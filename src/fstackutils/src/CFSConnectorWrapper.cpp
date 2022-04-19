/***********************************************************************
 * Copyright (C) 2016-2018, Nanjing StarOS Technology Co., Ltd 
**********************************************************************/

#include "fstackutils/CFSInterface.h"
#include "CFSConnectorWrapper.h"
#include "CFSTransportBase.h"
#include "CFSConnectorTcpT.h"
#include "CFSConnectorUdpT.h"
#include "CFSTransportTcp.h"
#include "CFSTransportUdp.h"
#include "CFSAcceptorUdp.h"
using namespace starbase;
namespace fsutils
{
CFSConnectorWrapper::CFSConnectorWrapper(IPSReactor *pNetworkThread)
    : m_pReactor(pNetworkThread)
    , m_pSink(NULL)
    , m_pConnector(NULL)
    , m_bClosed(TRUE)
{
    if (m_pReactor==NULL)
    {
        CAW_ASSERTE(0);
    }
}

CFSConnectorWrapper::~CFSConnectorWrapper()
{
    Close_i();
    if (m_pConnector)
    {
        delete m_pConnector;
        m_pConnector=NULL;
    }
}

DWORD CFSConnectorWrapper::AddReference()
{
    return CAWReferenceControlSingleThread::AddReference();
}

DWORD CFSConnectorWrapper::ReleaseReference()
{
    return CAWReferenceControlSingleThread::ReleaseReference();
}

CAWResult CFSConnectorWrapper::Init(IFSConnectionManager::CType aType)
{
    CAW_ASSERTE_RETURN(!m_pConnector, CAW_ERROR_ALREADY_INITIALIZED);

    //CAW_ASSERTE(!m_pReactor);
    //m_pReactor = CAWThreadManager::Instance()->GetThreadReactor(CAWThreadManager::TT_NETWORK);
    CAW_ASSERTE_RETURN(m_pReactor, CAW_ERROR_NOT_INITIALIZED);

    switch(aType) {
        case IFSConnectionManager::CTYPE_TCP:
            m_pConnector = new 
            CFSConnectorTcpT<CFSConnectorWrapper, CFSTransportTcp, CFSSocketTcp>
            (m_pReactor, *this);
            break;

        case IFSConnectionManager::CTYPE_UDP:
            m_pConnector = new
            CFSConnectorUdpT<CFSConnectorWrapper, CFSTransportUdp, CFSSocketUdp>
            (m_pReactor, *this);
            break;
        default:
            CAW_ERROR_TRACE("CFSConnectorWrapper::Init, error type=" << aType);
            Close_i();
            return CAW_ERROR_INVALID_ARG;
    }

    if (!m_pConnector) {
        Close_i();
        return CAW_ERROR_OUT_OF_MEMORY;
    }
    else {
        return CAW_OK;
    }
}

void CFSConnectorWrapper::
AsycConnect(IFSAcceptorConnectorSink* aSink, 
                const CAWInetAddr& aAddrPeer, 
                CAWTimeValue* aTimeout, 
                CAWInetAddr *aAddrLocal)
{
    CAW_ASSERTE(m_pConnector);

    CAW_ASSERTE(!m_pSink);
    m_pSink = aSink;
    CAW_ASSERTE(m_pSink);

    CAW_ASSERTE(m_bClosed);
    m_bClosed = FALSE;
    CAWInetAddr localtemp;
    if (aAddrLocal)
    {
        localtemp = *aAddrLocal;
    }

    CAW_INFO_TRACE_THIS("CFSConnectorWrapper::AsycConnect,"
        " addr=" << aAddrPeer.GetIpDisplayName() <<
        ",localaddr="<<localtemp.GetIpDisplayName()<<
        " m_pConnector=" << m_pConnector);

    int nRet = -1;
    if (m_pConnector && m_pSink)
        nRet = m_pConnector->Connect(aAddrPeer, aAddrLocal);
    if (nRet == -1) 
    {
        CAW_WARNING_TRACE("CFSConnectorWrapper::AsycConnect, connect failed."
            " addr=" << aAddrPeer.GetIpDisplayName() <<
            " err=" << errno);
        m_pReactor->ScheduleTimer(
            this, 
            reinterpret_cast<LPVOID>(CAW_ERROR_NETWORK_CONNECT_ERROR), 
            CAWTimeValue(0, 0), 
            1);
        return;
    }

    if (aTimeout) 
    {
        m_pReactor->ScheduleTimer(
            this, 
            reinterpret_cast<LPVOID>(CAW_ERROR_NETWORK_CONNECT_TIMEOUT), 
            *aTimeout, 
            1);
    }
}

int CFSConnectorWrapper::
OnConnectIndication(CAWResult aReason, IFSTransport *aTrpt, IFSConnectorInternal *aId)
{
    CAW_ASSERTE(m_pConnector);
    CAW_ASSERTE(m_pSink);
    CAW_ASSERTE(aId == m_pConnector);

    CAW_INFO_TRACE_THIS("CFSConnectorWrapper::OnConnectIndication,"
        " aReason=" << aReason <<
        " aTrpt=" << aTrpt <<
        " aId=" << aId);

    CAWAutoPtr<IFSTransport> pTransport(aTrpt);
    if (CAW_FAILED(aReason)) 
    {
        Close_i();
        m_pSink->OnConnectIndication(aReason, NULL, this);
    }
    else 
    {
        Close_i();
        m_pSink->OnConnectIndication(aReason, aTrpt, this);
    }
    return 0;
}

void CFSConnectorWrapper::CancelConnect()
    {
    Close_i();
    m_pSink = NULL;
}

void CFSConnectorWrapper::Close_i()
{
    if (m_bClosed)
    {
        return;
    }
    m_bClosed = TRUE;

    // Don't cleanup resources due to connect again.
    if (m_pConnector) 
    {
        m_pConnector->Close();
        //delete m_pConnector;
        //m_pConnector = NULL;
    }
    if (m_pReactor)
    {
        m_pReactor->CancelTimer(this);
        //m_pReactor = NULL;
    }
    // can't empty m_pSink because callback follows Close_i()
    //m_pSink = NULL;
}

void CFSConnectorWrapper::OnTimeout(const CAWTimeValue &aCurTime, LPVOID aArg)
{
    CAW_ASSERTE(m_pSink);

    CAW_UNUSED_ARG(aCurTime);

    CAWResult rvReason = reinterpret_cast<CAWResult>(aArg);
    if (rvReason == CAW_ERROR_NETWORK_CONNECT_ERROR) {
        CAW_INFO_TRACE_THIS("CFSConnectorWrapper::OnTimeout, connect failed.");
    }
    else if (rvReason == CAW_ERROR_NETWORK_CONNECT_TIMEOUT) {
        CAW_INFO_TRACE_THIS("CFSConnectorWrapper::OnTimeout, connect timeout.");
    }
    else {
        CAW_ERROR_TRACE("CFSConnectorWrapper::OnTimeout, unkown nReason=" << rvReason);
        CAW_ASSERTE(FALSE);
        return;
    }

    Close_i();
    m_pSink->OnConnectIndication(rvReason, NULL, this);
}

BOOL CFSConnectorWrapper::IsConnector()
{
    return TRUE;
}
}//namespace fsutils
