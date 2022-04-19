/***********************************************************************
 * Copyright (C) 2016-2018, Nanjing StarOS Technology Co., Ltd 
**********************************************************************/
#include "fstackutils/CFSStreamBase.h"
namespace fsutils
{
CFSStreamBase::CFSStreamBase()
    :m_pMbSendBuf(NULL)
    ,m_pMbRcvBuf(NULL)
    ,m_bConnected(false)
    ,m_bIsClient(false)
    ,m_dwSendBufMaxLen(1024 * 32)
    ,m_dwRcvBufMaxLen(1024 * 32)
    ,m_bisRcvoverbuffer(false)
    ,m_bisSendoverbuffer(false)
{
    CAW_INFO_TRACE("CFSStreamBase::CFSStreamBase");
}

CFSStreamBase::~CFSStreamBase()
{
    CAW_INFO_TRACE("CFSStreamBase::~CFSStreamBase");

    if (m_pConnector.Get())
    {
        m_pConnector->CancelConnect();
        m_pConnector=NULL;
    }
    if (m_pTransport.Get() != NULL)
    {
        m_pTransport->Disconnect(CAW_OK);
        m_pTransport = NULL;
    }

    if (m_pMbSendBuf) {
        m_pMbSendBuf->DestroyChained();
        m_pMbSendBuf = NULL;
    }
    if (m_pMbRcvBuf) {
        m_pMbRcvBuf->DestroyChained();
        m_pMbRcvBuf = NULL;
    }

}

void CFSStreamBase::SetSendBufferSize(WORD32 bufferSize)
{
    m_dwSendBufMaxLen = bufferSize;
}
void CFSStreamBase::SetRcvBufferSize(WORD32 bufferSize)
{
    m_dwRcvBufMaxLen = bufferSize;
    if (m_dwRcvBufMaxLen>1024 * 32)
    {
        m_dwRcvBufMaxLen=1024 * 32;
    }
}


CAWResult CFSStreamBase::Connect(const CAWInetAddr &peerAddr, CAWInetAddr *aAddrLocal)
{
    CAW_INFO_TRACE("CTCPSessionBase::Connect,");

    if(m_pConnector.Get())
    {
        CAW_WARNING_TRACE("CFSStreamBase::Connect, always have a connector");
        m_pConnector->CancelConnect();
        m_pConnector = NULL;
    }
    
    if (m_pTransport.Get())
    {
        m_pTransport->Disconnect(CAW_OK);
        m_pTransport = NULL;
    }

    IFSConnectionManager::CType typeConnector = IFSConnectionManager::CTYPE_TCP;
    CAWResult ret = IFSConnectionManager::Instance()->CreateConnectionClient(typeConnector, m_pConnector.ParaOut());
    CAW_ASSERTE_RETURN(CAW_SUCCEEDED(ret), ret);


    m_pConnector->AsycConnect(this,peerAddr, NULL, aAddrLocal);
    m_bIsClient = true;

    return CAW_OK;
}

void CFSStreamBase::OnConnectIndication(
                        CAWResult aReason,
                        IFSTransport *aTrpt,
                        IFSAcceptorConnectorId *aRequestId)
{
    CAW_ASSERTE(m_pTransport.Get()==NULL);
    CAW_ASSERTE(m_pConnector.Get() == aRequestId);
    CAW_INFO_TRACE("CFSStreamBase::OnConnectIndication aReason="<<aReason);

    if(CAW_SUCCEEDED(aReason))
    {
        CAWAutoPtr<IFSTransport> pTransport(aTrpt);
        if (CAW_FAILED(SetTransportHandle(pTransport)))
        {
            CAW_INFO_TRACE("CFSStreamBase::SetTransportHandle failure");
        }
        OnConnected(aReason);
    }
    else
    {
        OnPeerDisconnect(aReason);
    }

}

void CFSStreamBase::OnReceive(
            CAWMessageBlock &aData,
            IFSTransport *aTrptId,
            CFSTransportParameter *aPara)
{
    CAW_ASSERTE(aTrptId == m_pTransport.Get());
    size_t datasize = aData.GetChainedLength();
    if (datasize == 0)
    {
        return;
    }

    if (m_pMbRcvBuf==NULL)
    {
        m_pMbRcvBuf=aData.DuplicateChained();
    }
    else 
    {
        if (m_pMbRcvBuf->GetChainedLength() > m_dwRcvBufMaxLen)
        {
            m_bisRcvoverbuffer=true;
            return;
        }

        m_pMbRcvBuf->Append(aData.DuplicateChained());
    }
    
    datasize = m_pMbRcvBuf->GetChainedLength();
    CAW_INFO_TRACE("CFSStreamBase::OnReceive, datasize="<<datasize);
    std::allocator<char> allocChar;
    char *pBuf = allocChar.allocate(datasize, NULL);
    if (!pBuf)
        return;
    size_t realsize=0;
    m_pMbRcvBuf->Read(pBuf, datasize, &realsize, FALSE);

    size_t remainsize=realsize;
    char *preaddata = pBuf;
    size_t totalsize=0;
    while(remainsize>0)
    {
        size_t wLength = HandleMessage(preaddata, remainsize);
        if (wLength>remainsize)
        {
            CAW_ASSERTE(0);

            break;
        }
        if (wLength == 0)
        {
            break;
        }
        preaddata +=wLength;
        remainsize-=wLength;
        totalsize +=wLength;
    }

    CAWMessageBlock *pTmp2 = m_pMbRcvBuf->Disjoint(totalsize);
    m_pMbRcvBuf->DestroyChained();
    m_pMbRcvBuf = pTmp2;
    
    allocChar.deallocate(pBuf, datasize);
}

CAWResult CFSStreamBase::SendMessage(const char *data, size_t datasize)
{
    CAWMessageBlock mbOn(
                datasize, 
                data, 
                CAWMessageBlock::DONT_DELETE, 
                datasize);
    return SendMessage(mbOn);
}

CAWResult CFSStreamBase::SendMessage(CAWMessageBlock &aData)
{
    CAWResult rv;

    CAW_ASSERTE_RETURN(m_pTransport, CAW_ERROR_NOT_INITIALIZED);
    if (m_pTransport == NULL)
    {
        return CAW_ERROR_NOT_INITIALIZED;
    }

    if (!m_pMbSendBuf) {
		m_pMbSendBuf = aData.DuplicateChained();
    }
    else {
        if (m_pMbSendBuf != &aData) {
            m_pMbSendBuf->Append(aData.DuplicateChained());
        }
    }
	size_t dwTotal = aData.GetChainedLength();
	m_pTransport->SendData(*m_pMbSendBuf);
	aData.AdvanceChainedReadPtr(dwTotal);
	m_pMbSendBuf = m_pMbSendBuf->ReclaimGarbage();
    return CAW_OK;
}


void CFSStreamBase::OnSend(
        IFSTransport *aTrptId,
        CFSTransportParameter *aPara)

{
    CAW_ASSERTE(m_pTransport == aTrptId);

    if (m_pMbSendBuf) {
        m_pTransport->SendData(*m_pMbSendBuf);
        m_pMbSendBuf = m_pMbSendBuf->ReclaimGarbage();
    }
}

void CFSStreamBase::OnDisconnect(CAWResult aReason, IFSTransport *aTrptId)
{
    CAW_INFO_TRACE("CFSStreamBase::OnDisconnect, aReason="<<aReason);
    m_pTransport = NULL;
    m_bConnected = false;

    OnPeerDisconnect(aReason);
}

CAWResult CFSStreamBase::SetTransportHandle(CAWAutoPtr<IFSTransport> &pTransport)
{
    CAW_ASSERTE_RETURN(pTransport.Get(), CAW_ERROR_FAILURE);
    if (m_pTransport.Get() != NULL)
    {
        m_pTransport->Disconnect(CAW_OK);
        m_pTransport=NULL;
    }
    m_pTransport = pTransport;

    if (CAW_FAILED(m_pTransport->OpenWithSink(this))) 
    {
        CAW_ERROR_TRACE("CFSStreamBase::SetTransportHandle, Transport Open Error!");
        return CAW_ERROR_FAILURE;
    }

    pTransport->GetOption(CAW_OPT_TRANSPORT_PEER_ADDR, &m_addrPeer);
    pTransport->GetOption(CAW_OPT_TRANSPORT_LOCAL_ADDR, &m_addrLocal);
    m_bConnected = true;
    //CAW_INFO_TRACE("CFSStreamBase::SetTransportHandle, peer="<<m_addrPeer.GetIpDisplayName()<<",local="<<m_addrLocal.GetIpDisplayName());
    return CAW_OK;
}

CAWResult CFSStreamBase::Close(CAWResult reason)
{
    if (m_pTransport.Get())
    {
        m_pTransport->Disconnect(reason);
        m_pTransport = NULL;
    }
    
    if(m_pConnector.Get())
    {
        m_pConnector->CancelConnect();
        m_pConnector = NULL;
    }

    return CAW_OK;
}

}//namespace fsutils