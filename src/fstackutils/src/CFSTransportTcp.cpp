/***********************************************************************
 * Copyright (C) 2016-2018, Nanjing StarOS Technology Co., Ltd 
**********************************************************************/

#include "CFSTransportTcp.h"

namespace fsutils
{
CFSTransportTcp::CFSTransportTcp(IPSReactor *pReactor)
    : CFSTransportBase(pReactor)
    , m_bNeedOnSend(FALSE)
{
    CAW_INFO_TRACE_THIS("CFSTransportTcp::CFSTransportTcp");  
}

CFSTransportTcp::~CFSTransportTcp()
{
    CAW_INFO_TRACE_THIS("CFSTransportTcp::~CFSTransportTcp");
    m_TimerRereceive.Cancel();
    if (m_SocketTcp.GetHandle() != -1) {
        m_SocketTcp.Close(CAW_OK);
    }
}

CAWResult CFSTransportTcp::Open_t()
{
    CAWResult rv = CAW_ERROR_NETWORK_SOCKET_ERROR;
    CAW_INFO_TRACE_THIS("CFSTransportTcp::Open_t");

    DWORD dwRcv = 65535, dwSnd = 65535;
    rv = SetOption(CAW_OPT_TRANSPORT_RCV_BUF_LEN, &dwRcv);
    //	CAW_ASSERTE(CAW_SUCCEEDED(rv));
    rv = SetOption(CAW_OPT_TRANSPORT_SND_BUF_LEN, &dwSnd);
    //	CAW_ASSERTE(CAW_SUCCEEDED(rv));


    m_SocketTcp.GetLocalAddr(m_LocalAddr);
    m_SocketTcp.GetRemoteAddr(m_RemoteAddr);

    if (m_SocketTcp.GetHandle() == -1)
    {
        CAW_ASSERTE(0);
        return CAW_ERROR_FAILURE;
    }

    rv = m_pReactor->RegisterHandler(this, IAWEventHandler::READ_MASK | IAWEventHandler::WRITE_MASK);

    if (CAW_FAILED(rv) && rv != CAW_ERROR_FOUND) {
        CAW_ERROR_TRACE_THIS("CFSTransportTcp::Open_t, RegisterHandler(READ_MASK|WRITE_MASK) failed!");
        return rv;
    }
    else
        return CAW_OK;
    }

int CFSTransportTcp::GetHandle() const
{
    return m_SocketTcp.GetHandle();
}

static char szBuf[16*1024];

//static char szBuf[9];

int CFSTransportTcp::OnInput(int aFd)
{
    return OnEpollInput(aFd);
}

int CFSTransportTcp::OnEpollInput(int afd)
{
    CAW_INFO_TRACE("CFSTransportTcp::OnEpollInput");
    for(;;)
    {
        int nRecv = Recv_i(szBuf, sizeof(szBuf));
        if (nRecv <= 0)
            return nRecv;

        CAWMessageBlock mbOn(
            nRecv, 
            szBuf, 
            CAWMessageBlock::DONT_DELETE | CAWMessageBlock::WRITE_LOCKED, 
            nRecv);
        CAW_INFO_TRACE_THIS("CFSTransportTcp::OnEpollInput afd="<<afd<<",nRecv="<<nRecv);

        CAW_ASSERTE(m_pSink);
        //printf("CFSTransportTcp::OnEpollInput, m_pSink=%p\n", m_pSink);
        if (m_pSink)
            m_pSink->OnReceive(mbOn, this);
    }

    return 0;
}


void CFSTransportTcp::OnTimer(CAWTimerWrapperID *aId)
{
    CAW_UNUSED_ARG(aId);
    int nRecv = Recv_i(szBuf, sizeof(szBuf), FALSE);
    if (nRecv <= 0)
        return;

    CAWMessageBlock mbOn(
            nRecv,
            szBuf,
            CAWMessageBlock::DONT_DELETE | CAWMessageBlock::WRITE_LOCKED,
            nRecv);

    CAW_ASSERTE(m_pSink);
    if (m_pSink)
        m_pSink->OnReceive(mbOn, this);
}

int CFSTransportTcp::OnOutput(int fd)
{
    CAW_INFO_TRACE_THIS("CFSTransportTcp::OnOutput, m_bNeedOnSend="<<m_bNeedOnSend);

    //	CAW_INFO_LOG(("CFSTransportTcp::OnOutput"));
    if (!m_bNeedOnSend) 
        return 0;
    m_bNeedOnSend = FALSE;
    CAW_ASSERTE(m_pSink);
    if (m_pSink) 
        m_pSink->OnSend(this);
    return 0;
}

CFSSocketTcp& CFSTransportTcp::GetPeer()
{
    return m_SocketTcp;
}

CAWResult CFSTransportTcp::Close_t(CAWResult aReason)
{
    m_TimerRereceive.Cancel();
    if (m_SocketTcp.GetHandle() != -1) {
        //m_pReactor->RemoveHandler(this);
        m_SocketTcp.Close(aReason);
    }
    return CAW_OK;
}

CAWResult CFSTransportTcp::Disconnect_t(CAWResult aReason)
{
    m_TimerRereceive.Cancel();
    m_pReactor->RemoveHandler(this, IAWEventHandler::READ_MASK | IAWEventHandler::WRITE_MASK);
    return CAW_OK;
}


CAWResult CFSTransportTcp::
SendData(CAWMessageBlock &aData, CFSTransportParameter *aPara)
{
    //printf("CFSTransportTcp::SendData,size=%d\n", aData.GetChainedLength());
    CAW_INFO_TRACE_THIS("CFSTransportTcp::SendData aData="<<aData.GetChainedLength());

    if (m_SocketTcp.GetHandle() == -1) {
        CAW_WARNING_TRACE_THIS("CFSTransportTcp::SendData, socket is invalid.");
        return CAW_ERROR_NOT_INITIALIZED;
    }

    while(aData.GetChainedLength()>0)
    {
        iovec_ace iov[CAW_IOV_MAX];
        size_t dwFill = aData.FillIov(iov, CAW_IOV_MAX);
        CAW_ASSERTE_RETURN(dwFill > 0, CAW_OK);

        int nSend = m_SocketTcp.SendV((const iovec*)iov, (DWORD)dwFill);
        //printf("while CFSTransportTcp::SendData,size=%d, nSend=%d, this=%p\n", aData.GetChainedLength(), nSend, this);
        CAW_INFO_TRACE_THIS("CFSTransportTcp::SendData, nSend="<<nSend);

        if (nSend < 0) {
            if ((errno == VOS_EWOULDBLOCK) || (errno == VOS_EAGAIN)) {
                        m_bNeedOnSend = TRUE;
                        return CAW_ERROR_PARTIAL_DATA; 
                }
                else {
                    CAW_WARNING_TRACE_THIS("CFSTransportTcp::SendData, sendv() failed! err=" << errno << 
                    " fd=" << m_SocketTcp.GetHandle() << 
                    " addr1=" << m_LocalAddr.GetIpDisplayName() << 
                    " addr2=" << m_RemoteAddr.GetIpDisplayName());
                    // Don't NotifyHandler avoid blocking due to otify pipe overflow.
                    return CAW_ERROR_NETWORK_SOCKET_ERROR;
            }
        }
        else
        {
            if (aPara)
            {
                aPara->m_dwHaveSent+=nSend;
            }
            aData.AdvanceChainedReadPtr(nSend);
        }

        //printf("CFSTransportTcp::SendData,aData=%d, this=%p\n", aData.GetChainedLength(), this);
#if 0
        if (aData.GetChainedLength()) {
#ifdef CAW_USE_REACTOR_SELECT
            m_pReactor->RegisterHandler(this, IAWEventHandler::READ_MASK | IAWEventHandler::WRITE_MASK);
#endif // CAW_USE_REACTOR_SELECT
            m_bNeedOnSend = TRUE;
            return CAW_ERROR_PARTIAL_DATA;
        }
        else
            return CAW_OK;
#endif
    }

    return CAW_OK;
}

CAWResult CFSTransportTcp::GetOption(DWORD aCommand, LPVOID aArg)
{
    switch (aCommand) {
        case CAW_OPT_TRANSPORT_FIO_NREAD:
            if (m_SocketTcp.Control(FIONREAD, aArg) == -1) {
                CAW_WARNING_TRACE_THIS("CFSTransportTcp::GetOption, (CAW_OPT_TRANSPORT_FIO_NREAD) failed! err=" << errno);
                return CAW_ERROR_NETWORK_SOCKET_ERROR;
            }
            return CAW_OK;

        case CAW_OPT_TRANSPORT_FD:
            *(static_cast<int *>(aArg)) = m_SocketTcp.GetHandle();
            return CAW_OK;

        case CAW_OPT_TRANSPORT_LOCAL_ADDR:
            if (m_SocketTcp.GetLocalAddr(*(static_cast<CAWInetAddr*>(aArg))) == -1) {
                CAW_WARNING_TRACE_THIS("CFSTransportTcp::GetOption, (CAW_OPT_TRANSPORT_LOCAL_ADDR) failed! err=" << errno);
                return CAW_ERROR_NETWORK_SOCKET_ERROR;
            }
            else
            return CAW_OK;

        case CAW_OPT_TRANSPORT_PEER_ADDR:
            if (m_SocketTcp.GetRemoteAddr(*(static_cast<CAWInetAddr*>(aArg))) == -1) {
                CAW_WARNING_TRACE_THIS("CFSTransportTcp::GetOption, (CAW_OPT_TRANSPORT_PEER_ADDR) failed! err=" << errno);
                return CAW_ERROR_NETWORK_SOCKET_ERROR;
            }
            else
            return CAW_OK;

        case CAW_OPT_TRANSPORT_TRAN_TYPE:
            *(static_cast<IFSConnectionManager::CType*>(aArg)) = IFSConnectionManager::CTYPE_TCP;
            return CAW_OK;

        case CAW_OPT_TRANSPORT_RCV_BUF_LEN: {
            int nLen = sizeof(DWORD);
            if (m_SocketTcp.GetOption(SOL_SOCKET, SO_SNDBUF, aArg, &nLen) == -1) {
            //			CAW_ERROR_TRACE_THIS("CFSTransportTcp::GetOption, GetOption(SO_SNDBUF) failed! err=" << errno);
            return CAW_ERROR_NETWORK_SOCKET_ERROR;
            }
            else
            return CAW_OK;
            }

        case CAW_OPT_TRANSPORT_SND_BUF_LEN: {
            int nLen = sizeof(DWORD);
            if (m_SocketTcp.GetOption(SOL_SOCKET, SO_RCVBUF, aArg, &nLen) == -1) {
            //			CAW_ERROR_TRACE_THIS("CFSTransportTcp::GetOption, GetOption(SO_RCVBUF) failed! err=" << errno);
            return CAW_ERROR_NETWORK_SOCKET_ERROR;
            }
            else
            return CAW_OK;
            }

        default:
            CAW_WARNING_TRACE_THIS("CFSTransportTcp::GetOption,"
            " unknow aCommand=" << aCommand << 
            " aArg=" << aArg);
            return CAW_ERROR_INVALID_ARG;
        }
    }

CAWResult CFSTransportTcp::SetOption(DWORD aCommand, LPVOID aArg)
{
    CAW_ASSERTE_RETURN(aArg, CAW_ERROR_INVALID_ARG);
    switch (aCommand) {
        case CAW_OPT_TRANSPORT_FD: {
            // we allow user to set TCP socket to CAW_INVALID_HANDLE, 
            // mainly used by CAWConnectorProxyT.
            int hdNew = *(static_cast<int *>(aArg));
            CAW_ASSERTE_RETURN(hdNew == -1, CAW_ERROR_INVALID_ARG);
            m_SocketTcp.SetHandle(hdNew);
            return CAW_OK;
        }

        case CAW_OPT_TRANSPORT_RCV_BUF_LEN:
            if (m_SocketTcp.SetOption(SOL_SOCKET, SO_SNDBUF, aArg, sizeof(DWORD)) == -1) {
                //			CAW_ERROR_TRACE_THIS("CFSTransportTcp::SetOption, SetOption(SO_SNDBUF) failed! err=" << errno);
                return CAW_ERROR_NETWORK_SOCKET_ERROR;
            }
            else
                return CAW_OK;

        case CAW_OPT_TRANSPORT_SND_BUF_LEN:
            if (m_SocketTcp.SetOption(SOL_SOCKET, SO_RCVBUF, aArg, sizeof(DWORD)) == -1) {
            //			CAW_ERROR_TRACE_THIS("CFSTransportTcp::SetOption, SetOption(SO_RCVBUF) failed! err=" << errno);
                return CAW_ERROR_NETWORK_SOCKET_ERROR;
            }
            else
                return CAW_OK;


        default:
            CAW_WARNING_TRACE_THIS("CFSTransportTcp::SetOption,"
            " unknow aCommand=" << aCommand << 
            " aArg=" << aArg);
        return CAW_ERROR_INVALID_ARG;
    }
}


int CFSTransportTcp::GetTransportHandle() const
{
    //CAW_ASSERTE(m_SocketTcp.GetHandle() != CAW_INVALID_HANDLE);
    return m_SocketTcp.GetHandle();
}
}//namespace fsutils

