/***********************************************************************
 * Copyright (C) 2016-2018, Nanjing StarOS Technology Co., Ltd 
**********************************************************************/

#include "CFSAcceptorTcp.h"
#include "CFSTransportTcp.h"
namespace fsutils
{
CFSAcceptorTcp::CFSAcceptorTcp(IPSReactor *pThreadNetwork)
    :CFSAcceptorBase(pThreadNetwork)
{
    CAW_INFO_TRACE("CFSAcceptorTcp::CFSAcceptorTcp\n");
}

CFSAcceptorTcp::~CFSAcceptorTcp()
{
    CAW_INFO_TRACE("CFSAcceptorTcp::~CFSAcceptorTcp\n");
    StopListen(CAW_OK);
}

CAWResult CFSAcceptorTcp::StartListen(IFSAcceptorConnectorSink *aSink, const CAWInetAddr &aAddrListen)
{
    CAWResult rv = CAW_ERROR_NETWORK_SOCKET_ERROR;
    CAW_ASSERTE_RETURN(m_Socket.GetHandle() == -1, CAW_ERROR_ALREADY_INITIALIZED);

    CAW_ASSERTE(!m_pSink);
    CAW_ASSERTE_RETURN(aSink, CAW_ERROR_INVALID_ARG);
    m_pSink = aSink;
    int on = 1;

    int nRet = m_Socket.Open();
    if (nRet == -1) {
        CAW_ERROR_TRACE_THIS("CFSAcceptorTcp::StartListen, Open() failed!"
            " addr=" << aAddrListen.GetIpDisplayName() <<
            " err=" << errno);
        goto fail;
    }
    CAW_INFO_TRACE_THIS("socket fd="<<m_Socket.GetHandle());

    //fs_ioctl((CAW_SOCKET)m_Socket.GetHandle(), FIONBIO, &on);

    nRet = vos_sock_bind((CAW_SOCKET)m_Socket.GetHandle(),(struct vos_sock_sockaddr*)(aAddrListen.GetPtr()), aAddrListen.GetSize());
    if (nRet == -1) {
        CAW_ERROR_TRACE_THIS("CFSAcceptorTcp::StartListen, bind() failed!"
            " addr=" << aAddrListen.GetIpDisplayName() <<
            " err=" << errno);
        rv = CAW_ERROR_NETWORK_SOCKET_ERROR;
        goto fail;
    }
    CAW_INFO_TRACE_THIS("fs_bind fd="<<m_Socket.GetHandle());

    nRet = vos_sock_listen((CAW_SOCKET)m_Socket.GetHandle(), 512/*SOMAXCONN*/);
    if (nRet == -1) {
        CAW_ERROR_TRACE_THIS("CFSAcceptorTcp::StartListen, listen() failed! err=" << errno);
        rv = CAW_ERROR_NETWORK_SOCKET_ERROR;
        goto fail;
    }
    CAW_INFO_TRACE_THIS("fs_bind fd="<<m_Socket.GetHandle());


    rv = m_pReactor->RegisterHandler(this, IPSEventHandler::ACCEPT_MASK);
    if (CAW_FAILED(rv))
        goto fail;

    CAW_INFO_TRACE_THIS("CFSAcceptorTcp::StartListen,"
        " addr=" << aAddrListen.GetIpDisplayName() <<
        " aSink=" << aSink << 
        " fd=" << m_Socket.GetHandle());

    return CAW_OK;

fail:
    CAW_ASSERTE(CAW_FAILED(rv));
    StopListen(rv);
    return rv;
}

CAWResult CFSAcceptorTcp::StopListen(CAWResult aReason)
{
    if (m_Socket.GetHandle() != -1) {
        m_pReactor->RemoveHandler(this, IAWEventHandler::ACCEPT_MASK);
        m_Socket.Close(aReason);
    }
    m_pSink = NULL;
    return CAW_OK;
}

int CFSAcceptorTcp::GetHandle() const 
{
    return m_Socket.GetHandle();
}

int CFSAcceptorTcp::OnInput(int aFd)
{
    return OnEpollInput(aFd);
}

int CFSAcceptorTcp::OnEpollInput(int aFd)
{
    CAW_ASSERTE(aFd == GetHandle());
    CFSTransportTcp *pTrans = NULL;

    for(;;)
    {
        CAWInetAddr addrPeer;
        int nAddrLen = addrPeer.GetSize();
        int sockNew = (int)vos_sock_accept(
            (int)GetHandle(),
            reinterpret_cast<struct vos_sock_sockaddr *>(addrPeer.GetPtr()),
            reinterpret_cast<socklen_t*>(&nAddrLen)
            );

        if ((sockNew == -1) 
            && ((errno == VOS_EAGAIN) ||(errno == VOS_EWOULDBLOCK)))
        {
            break;
        }
        else if(sockNew == -1) {
            if ((errno == VOS_EAGAIN) ||(errno == VOS_EWOULDBLOCK))
            {
                break;
            }
            else
            {
                CAW_ERROR_TRACE_THIS("CFSAcceptorTcp::OnInput, accept() failed! err=" << errno
                    <<", temp="<< VOS_EWOULDBLOCK);
            }
            break;
        }
        //CAWAutoPtr<IAWTransport> pTrans(new CAWTransportTcp(m_pReactorNetwork));

        //IAWTransport *aTrpt
        // create TCP transport with network reactor in the network thread.
        pTrans = new CFSTransportTcp(m_pReactor);

        if (!pTrans)
        {
            return 0;
        }

        pTrans->GetPeer().SetHandle(sockNew);
        if (pTrans->GetPeer().Enable(CFSIPCBase::NON_BLOCK) == -1) {
            CAW_ERROR_TRACE_THIS("CFSAcceptorTcp::OnInput, Enable(NON_BLOCK) failed! err=" << errno);
            goto fail;
        }

        CAW_INFO_TRACE_THIS("CFSAcceptorTcp::OnInput,"
            " addr=" << addrPeer.GetIpDisplayName() <<
            " sockNew=" << sockNew << 
            " pTrans=" << pTrans);

        pTrans->AddReference();
        CAW_ASSERTE(m_pSink);
        if (m_pSink)
        {
            m_pSink->OnConnectIndication(CAW_OK, pTrans, this);
        }
        pTrans->ReleaseReference();
    }
    return 0;
fail:
    if (pTrans)
        delete pTrans;
    return 0;
}


int CFSAcceptorTcp::OnClose(int aFd, MASK aMask)
{
    CAW_ERROR_TRACE_THIS("CFSAcceptorTcp::OnClose, it's impossible!"
        " aFd=" << aFd <<
        " aMask=" << aMask);
    return 0;
}
}//namespace fsutils
