/***********************************************************************
 * Copyright (C) 2016-2018, Nanjing StarOS Technology Co., Ltd 
**********************************************************************/
#include "CFSTransportUdp.h"
#include "CFSAcceptorUdp.h"
namespace fsutils
{
CFSTransportUdp::CFSTransportUdp(IPSReactor *pReactor, const CAWInetAddr &aAddrSend, CFSAcceptorUdp *pAcceptor)
    : CFSTransportBase(pReactor)
    , m_pAcceptor(pAcceptor)
    , m_AddrSend(aAddrSend)
{
    CAW_INFO_TRACE_THIS("CFSTransportUdp::CFSTransportUdp");
}

CFSTransportUdp::~CFSTransportUdp()
{
    CAW_INFO_TRACE_THIS("CFSTransportUdp::~CFSTransportUdp");
    Close_t(CAW_OK);
}

CAWResult CFSTransportUdp::Open_t()
{
    CAWResult rv = CAW_OK;
    // Don't RegisterHandler if it is created by Acceptor because Acceptor has registered it.
    if (m_pAcceptor) 
        return rv;

    DWORD dwRcv = 65535, dwSnd = 65535;
    rv = SetOption(CAW_OPT_TRANSPORT_RCV_BUF_LEN, &dwRcv);
    CAW_ASSERTE(CAW_SUCCEEDED(rv));
    rv = SetOption(CAW_OPT_TRANSPORT_SND_BUF_LEN, &dwSnd);
    CAW_ASSERTE(CAW_SUCCEEDED(rv));

    rv = m_pReactor->RegisterHandler(this, IAWEventHandler::READ_MASK);
    if (CAW_FAILED(rv) && rv != CAW_ERROR_FOUND) {
        CAW_ERROR_TRACE_THIS("CFSTransportUdp::Open_t, RegisterHandler(READ_MASK) failed! rv=" << rv);
        return rv;
    }
    else {
        return CAW_OK;
    }
}

int CFSTransportUdp::GetHandle() const
{
    return m_SocketUdp.GetHandle();
}

int CFSTransportUdp::OnInput(int aFd)
{
    return OnEpollInput(aFd);
}

int CFSTransportUdp::OnEpollInput(int aFd)
{
    static char szBuf[16 * 1024];
    for(;;)
    {
        int nRecv = m_SocketUdp.Recv(szBuf, sizeof(szBuf));
        if (nRecv <= 0) {
            int nErr = errno;
            if (nErr == VOS_EWOULDBLOCK)
            {
                return 0;
            }
            else
            {
                CAW_WARNING_TRACE_THIS("CFSTransportUdp::OnInput, Recv() failed!"
                    " nRecv=" << nRecv <<
                    " m_pAcceptor=" << m_pAcceptor <<
                    " err=" << nErr);
            }

            CAW_ASSERTE(!m_pAcceptor);
                return -1;
        }

        OnReceiveCallback(szBuf, nRecv);
    }
    return 0;
}

CFSSocketUdp& CFSTransportUdp::GetPeer()
{
    return m_SocketUdp;
}

CAWResult CFSTransportUdp::Close_t(CAWResult aReason)
{
    // Don't close socket if it is created by Acceptor.
    CAW_UNUSED_ARG(aReason);
    if (m_SocketUdp.GetHandle() != -1) {
        if (!m_pAcceptor) {
            m_SocketUdp.Close();
        }
        else {
            m_pAcceptor->RemoveTransport(m_AddrSend, this);
            m_SocketUdp.SetHandle(-1);
        }
    }
    return CAW_OK;
}

CAWResult CFSTransportUdp::Disconnect_t(CAWResult aReason)
{
    CAW_UNUSED_ARG(aReason);
    if (m_SocketUdp.GetHandle() != -1) {
        if (!m_pAcceptor) {
            m_pReactor->RemoveHandler(this, IAWEventHandler::READ_MASK);
            m_SocketUdp.Close();
        }
        else {
            m_pAcceptor->RemoveTransport(m_AddrSend, this);
            m_SocketUdp.SetHandle(-1);
        }
    }
    return CAW_OK;  
}


CAWResult CFSTransportUdp::SendData(CAWMessageBlock &aData, CFSTransportParameter *aPara)
{
    bool bhight = false;
    //CAW_ASSERTE(aData.GetChainedLength() <= CAWConnectionManager::UDP_SEND_MAX_LEN);
    if (m_SocketUdp.GetHandle() == -1) {
        CAW_WARNING_TRACE_THIS("CFSTransportUdp::SendData, socket is invalid.");
        return CAW_ERROR_NOT_INITIALIZED;
    }

    while(aData.GetChainedLength()>0)
    {
        static iovec_ace iov[CAW_IOV_MAX];
        size_t dwFill = aData.FillIov(iov, CAW_IOV_MAX);
        CAW_ASSERTE_RETURN(dwFill > 0, CAW_OK);

        // ensure iovec are filled with all chained <CAWMessageBlock>s.
        CAW_ASSERTE(dwFill < CAW_IOV_MAX);

        // SendVTo() will failed if after connect() in Win98.
        int nSend;
        if (m_pAcceptor)
            nSend = m_SocketUdp.SendVTo((const iovec*)iov, dwFill, m_AddrSend);
        else 
            nSend = m_SocketUdp.SendV((const iovec*)iov, dwFill);

        if (nSend<0)
        {
            if ((errno == VOS_EWOULDBLOCK) || (errno == VOS_EAGAIN)) {
                if (bhight)
                {
                    continue;
                }
                else
                {
                    CAW_WARNING_TRACE_THIS("CFSTransportUdp::SendData, sendv() EWOULDBLOCK!"
                    " nSend=" << nSend <<
                    " send_len=" << aData.GetChainedLength() <<
                    " dwFill=" << dwFill <<
                    " addr=" << m_AddrSend.GetIpDisplayName() <<
                    " err=" << errno);
                    
                    CAW_ASSERTE(nSend <= 0);
                    return CAW_ERROR_PARTIAL_DATA;
                }
            }
            else
            {
                CAW_ERROR_TRACE_THIS("CFSTransportUdp::SendData, sendv() failed!"
                    " nSend=" << nSend <<
                    " send_len=" << aData.GetChainedLength() <<
                    " dwFill=" << dwFill <<
                    " addr=" << m_AddrSend.GetIpDisplayName() <<
                    " err=" << errno);

                return CAW_ERROR_NETWORK_SOCKET_ERROR;
            }
        }
        else
        {
            if (aPara)
            {
                aPara->m_dwHaveSent += nSend;
            }
            aData.AdvanceChainedReadPtr(nSend);
        }
    }
    return CAW_OK;
}

CAWResult CFSTransportUdp::GetOption(DWORD aCommand, LPVOID aArg)
{
    CAW_ASSERTE_RETURN(aArg, CAW_ERROR_INVALID_ARG);
    switch (aCommand) {
    case CAW_OPT_TRANSPORT_FIO_NREAD:
        if (m_SocketUdp.Control(FIONREAD, aArg) == -1) {
            CAW_WARNING_TRACE_THIS("CFSTransportUdp::GetOption, (CAW_OPT_TRANSPORT_FIO_NREAD) failed! err=" << errno);
            return CAW_ERROR_NETWORK_SOCKET_ERROR;
        }
        return CAW_OK;

    case CAW_OPT_TRANSPORT_FD:
        *(static_cast<int*>(aArg)) = m_SocketUdp.GetHandle();
        return CAW_OK;

    case CAW_OPT_TRANSPORT_LOCAL_ADDR:
        if (m_SocketUdp.GetLocalAddr(*(static_cast<CAWInetAddr*>(aArg))) == -1) {
            CAW_WARNING_TRACE_THIS("CFSTransportUdp::GetOption, (CAW_OPT_TRANSPORT_LOCAL_ADDR) failed!"
            " fd=" << m_SocketUdp.GetHandle() << " err=" << errno);
            return CAW_ERROR_NETWORK_SOCKET_ERROR;
        }
        else
            return CAW_OK;

    case CAW_OPT_TRANSPORT_PEER_ADDR:
        *(static_cast<CAWInetAddr*>(aArg)) = m_AddrSend;
        return CAW_OK;

    case CAW_OPT_TRANSPORT_TRAN_TYPE:
        *(static_cast<IFSConnectionManager::CType*>(aArg)) = IFSConnectionManager::CTYPE_UDP;
        return CAW_OK;

    case CAW_OPT_TRANSPORT_RCV_BUF_LEN: {
        int nLen = sizeof(DWORD);
        if (m_SocketUdp.GetOption(SOL_SOCKET, SO_SNDBUF, aArg, &nLen) == -1) {
            CAW_ERROR_TRACE_THIS("CFSTransportUdp::GetOption, GetOption(SO_SNDBUF) failed!"
            " fd=" << m_SocketUdp.GetHandle() << " err=" << errno);
            return CAW_ERROR_NETWORK_SOCKET_ERROR;
        }
        else
            return CAW_OK;
        }

    case CAW_OPT_TRANSPORT_SND_BUF_LEN: {
        int nLen = sizeof(DWORD);
        if (m_SocketUdp.GetOption(SOL_SOCKET, SO_RCVBUF, aArg, &nLen) == -1) {
            CAW_ERROR_TRACE_THIS("CFSTransportUdp::GetOption, GetOption(SO_RCVBUF) failed!"
            " fd=" << m_SocketUdp.GetHandle() << " err=" << errno);
            return CAW_ERROR_NETWORK_SOCKET_ERROR;
        }
        else
            return CAW_OK;
        }

    case CAW_OPT_TRANSPORT_SOCK_ALIVE: {
            if (m_SocketUdp.GetHandle() == -1) {
                *static_cast<BOOL*>(aArg) = FALSE;
                return CAW_ERROR_NOT_INITIALIZED;
            }
            else {
                *static_cast<BOOL*>(aArg) = TRUE;
                return CAW_OK;
            }
        }

    default:
        CAW_WARNING_TRACE_THIS("CFSTransportUdp::GetOption,"
        " unknow aCommand=" << aCommand << 
        " aArg=" << aArg);
    return CAW_ERROR_INVALID_ARG;
    }
}

CAWResult CFSTransportUdp::SetOption(DWORD aCommand, LPVOID aArg)
{
    CAW_ASSERTE_RETURN(aArg, CAW_ERROR_INVALID_ARG);

    switch (aCommand) {
    case CAW_OPT_TRANSPORT_RCV_BUF_LEN:
        if (m_SocketUdp.SetOption(SOL_SOCKET, SO_SNDBUF, aArg, sizeof(DWORD)) == -1) {
            CAW_ERROR_TRACE_THIS("CFSTransportUdp::SetOption, SetOption(SO_SNDBUF) failed! err=" << errno);
            return CAW_ERROR_NETWORK_SOCKET_ERROR;
        }
        else
            return CAW_OK;

    case CAW_OPT_TRANSPORT_SND_BUF_LEN:
        if (m_SocketUdp.SetOption(SOL_SOCKET, SO_RCVBUF, aArg, sizeof(DWORD)) == -1) {
            CAW_ERROR_TRACE_THIS("CFSTransportUdp::SetOption, SetOption(SO_RCVBUF) failed! err=" << errno);
            return CAW_ERROR_NETWORK_SOCKET_ERROR;
        }
        else
            return CAW_OK;

    default:
        CAW_WARNING_TRACE_THIS("CFSTransportUdp::SetOption,"
        " unknow aCommand=" << aCommand << 
        " aArg=" << aArg);
    return CAW_ERROR_INVALID_ARG;
    }
}

int CFSTransportUdp::GetTransportHandle() const
{
    return m_SocketUdp.GetHandle();
}
}//namespace fsutils
