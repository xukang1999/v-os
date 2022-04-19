/***********************************************************************
 * Copyright (C) 2016-2018, Nanjing StarOS Technology Co., Ltd 
**********************************************************************/

#include "CFSTransportBase.h"
#include "fstackutils/CFSSocket.h"
namespace fsutils
{
CFSTransportBase::CFSTransportBase(IPSReactor *pReactor)
    : m_pSink(NULL)
    , m_pReactor(pReactor)
{
    CAW_ASSERTE(m_pReactor);
    CAW_INFO_TRACE_THIS("CFSTransportBase::CFSTransportBase");
}

CFSTransportBase::~CFSTransportBase()
{
    CAW_INFO_TRACE_THIS("CFSTransportBase::~CFSTransportBase");
}

DWORD CFSTransportBase::AddReference()
{
    CAW_INFO_TRACE("CFSTransportBase::AddReference");
    return CAWReferenceControlSingleThreadTimerDelete::AddReference();
}

DWORD CFSTransportBase::ReleaseReference()
{
    CAW_INFO_TRACE("CFSTransportBase::ReleaseReference");
    return CAWReferenceControlSingleThreadTimerDelete::ReleaseReference();
}

CAWResult CFSTransportBase::OpenWithSink(IFSTransportSink *aSink)
{
    CAW_ASSERTE_RETURN(aSink, CAW_ERROR_INVALID_ARG);

    // we allow the upper layer invokes this function many times.
    if (m_pSink) {
        m_pSink = aSink;
        return CAW_OK;
    }
    else {
        m_pSink = aSink;
    }

    CAWResult rv = Open_t();
    if (CAW_FAILED(rv)) {
        Close_t(CAW_OK);
        m_pSink = NULL;
    }
    return rv;
}

IFSTransportSink* CFSTransportBase::GetSink()
{
    return m_pSink;
}

CAWResult CFSTransportBase::Disconnect(CAWResult aReason)
{
    CAW_INFO_TRACE_THIS("CFSTransportBase::Disconnect");

    CAWResult rv = Disconnect_t(aReason);
    m_pSink = NULL;
    return rv;
}

int CFSTransportBase::OnClose(int aFd, MASK aMask)
{
    
    CAW_INFO_TRACE_THIS("CFSTransportBase::OnClose aFd="<<aFd);
    if (m_pSink==NULL)
    {
        return 0;
    }
    CAW_UNUSED_ARG(aFd);
    CAW_UNUSED_ARG(aMask);
    Close_t(CAW_OK);
    IFSTransportSink *pTmp = m_pSink;
    m_pSink = NULL;

    CAW_ASSERTE(pTmp);


    if (pTmp)
    {
        //printf("CFSTransportBase::OnClose, ptmp=%p, this=%p\n", pTmp, this);
        pTmp->OnDisconnect(CAW_ERROR_NETWORK_SOCKET_ERROR, this);
    }

    return 0;
}
}//namespace fsutils

