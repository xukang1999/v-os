/***********************************************************************
 * Copyright (C) 2016-2018, Nanjing StarOS Technology Co., Ltd 
**********************************************************************/

#include "CFSAcceptorUdp.h"
#include "CFSTransportUdp.h"
namespace fsutils
{
class CFSPairInetAddr
{
public:
    CFSPairInetAddr()
        : m_dwIpSrc(0)
        , m_dwIpDst(0)
        , m_wPortSrc(0)
        , m_wPortDst(0)
    {
    }

    CFSPairInetAddr(const CAWInetAddr &aSrc, const CAWInetAddr &aDst)
        : m_dwIpSrc(((sockaddr_in *)aSrc.GetPtr())->sin_addr.s_addr)
        , m_dwIpDst(((sockaddr_in *)aDst.GetPtr())->sin_addr.s_addr)
        , m_wPortSrc(((sockaddr_in *)aSrc.GetPtr())->sin_port)
        , m_wPortDst(((sockaddr_in *)aDst.GetPtr())->sin_port)
    {
    }

    DWORD GetHashValue() const 
    {
        // this hash function is copied from linux kernel
        // source code whose flie name is "net/ipv4/Tcp_ipv4.c".
        int h = ((m_dwIpSrc ^ m_wPortSrc) ^ (m_dwIpDst ^ m_wPortDst));
        h ^= h>>16;
        h ^= h>>8;
        return h;
    }

    bool operator == (const CFSPairInetAddr &aRight) const 
    {
        return m_dwIpSrc == aRight.m_dwIpSrc && 
                m_dwIpDst == aRight.m_dwIpDst && 
                m_wPortSrc == aRight.m_wPortSrc && 
                m_wPortDst == aRight.m_wPortDst;
    }

public:
    DWORD m_dwIpSrc;
    DWORD m_dwIpDst;
    WORD m_wPortSrc;
    WORD m_wPortDst;
};

struct IFSnetAddrHash
{
    size_t operator()(const CFSPairInetAddr &addr) const
    {
        return addr.GetHashValue();
    }
};

typedef CAWHashMapT<CFSPairInetAddr, CAWAutoPtr<CFSTransportUdp>,IFSnetAddrHash> FSUdpTransportsType;


static FSUdpTransportsType *s_pfsUdpTransports;

// Don't make the hash_map static,
// because CAWAutoPtr<CAWTransportUdp> may access thread info 
// which will be deleted in ~CAWThreadManager.
class CFSUdpTransportHashMap : public CAWCleanUpBase
{
public:
    CFSUdpTransportHashMap() : m_Transports(4096)
    {
        CAW_ASSERTE(!s_pfsUdpTransports);
        s_pfsUdpTransports = &m_Transports;
    }

    virtual ~CFSUdpTransportHashMap()
    {
        CAW_ASSERTE(s_pfsUdpTransports);
        s_pfsUdpTransports = NULL;
    }

public:
    FSUdpTransportsType m_Transports;
};


CFSAcceptorUdp::CFSAcceptorUdp(IPSReactor *pNetworkThread)
    :CFSAcceptorBase(pNetworkThread)
{
    if (!s_pfsUdpTransports) {
        new CFSUdpTransportHashMap();
    }
}

CFSAcceptorUdp::~CFSAcceptorUdp()
{
    StopListen(CAW_OK);
}

CAWResult CFSAcceptorUdp::StartListen(IFSAcceptorConnectorSink *aSink, const CAWInetAddr &aAddrListen)
{
    CAWResult rv = CAW_ERROR_NETWORK_SOCKET_ERROR;
    CAW_ASSERTE_RETURN(m_Socket.GetHandle() == -1, CAW_ERROR_ALREADY_INITIALIZED);

    CAW_ASSERTE(!m_pSink);
    CAW_ASSERTE_RETURN(aSink, CAW_ERROR_INVALID_ARG);
    m_pSink = aSink;

    DWORD dwRcv = 262144, dwSnd = 262144;
    int nOption;
    int nRet = m_Socket.Open(aAddrListen);
    if (nRet == -1) {
        CAW_ERROR_TRACE_THIS("CFSAcceptorUdp::StartListen, Open() failed!"
        " addr=" << aAddrListen.GetIpDisplayName() << 
        " err=" << errno);
        rv = CAW_ERROR_NETWORK_SOCKET_ERROR;
        goto fail;
    }

    nOption = m_Socket.SetOption(SOL_SOCKET, SO_SNDBUF,  &dwSnd, sizeof(DWORD));
    CAW_ASSERTE(nOption == 0);
    nOption = m_Socket.SetOption(SOL_SOCKET, SO_RCVBUF,  &dwRcv, sizeof(DWORD));
    CAW_ASSERTE(nOption == 0);

    rv = m_pReactor->RegisterHandler(this, IAWEventHandler::READ_MASK);
    if (CAW_FAILED(rv))
        goto fail;

    // aAddrListen may be "0.0.0.0".
    m_AddrLocol = aAddrListen;
    CAWStopFlag::SetStartFlag();
    return CAW_OK;

fail:
    CAW_ASSERTE(CAW_FAILED(rv));
    StopListen(rv);
    return rv;
}

CAWResult CFSAcceptorUdp::StopListen(CAWResult )
{
    if (CAWStopFlag::IsFlagStopped())
        return CAW_OK;

    // Don't clear because s_pfsUdpTransports is singleton.
    if (s_pfsUdpTransports) {
        FSUdpTransportsType::iterator iter = s_pfsUdpTransports->begin();
        while (iter != s_pfsUdpTransports->end()) {
            const CFSPairInetAddr &pairAddr = (*iter).first;
            DWORD dwIpDst = ((sockaddr_in *)m_AddrLocol.GetPtr())->sin_addr.s_addr;
            WORD wPortDst = ((sockaddr_in *)m_AddrLocol.GetPtr())->sin_port;
            if (pairAddr.m_dwIpDst == dwIpDst && pairAddr.m_wPortDst == wPortDst) {
                FSUdpTransportsType::iterator iterTmp = iter;
                ++iterTmp;
                s_pfsUdpTransports->erase(iter);
                iter = iterTmp;
            }
            else {
                ++iter;
            }
        }
    }

    if (m_Socket.GetHandle() != -1) {
        m_pReactor->RemoveHandler(this, IAWEventHandler::READ_MASK);
        m_Socket.Close();
    }
    m_pSink = NULL;
    CAWStopFlag::SetStopFlag();
    return CAW_OK;

}

int CFSAcceptorUdp::GetHandle() const
{
    return m_Socket.GetHandle();
}

int CFSAcceptorUdp::OnInput(int aFd)
{
    return OnEpollInput(aFd);
}

int CFSAcceptorUdp::OnClose(int aFd, MASK aMask)
{
    CAW_ERROR_TRACE_THIS("CFSAcceptorUdp::OnClose, it's impossible!"
        " aFd=" << aFd <<
        " aMask=" << aMask);
    CAW_ASSERTE(FALSE);
    return 0;
}

CAWResult CFSAcceptorUdp::RemoveTransport(const CAWInetAddr &aAddr, CFSTransportUdp *aTrpt)
{
    CAW_UNUSED_ARG(aTrpt);
    if (CAWStopFlag::IsFlagStopped()) {
        CAW_WARNING_TRACE("CFSAcceptorUdp::RemoveTransport, the acceptor is stopped.");
        return CAW_OK;
    }

    CAW_INFO_TRACE_THIS("CFSAcceptorUdp::RemoveTransport,"
        " src_ip=" << aAddr.GetIpDisplayName()<<
        " dst_ip=" << m_AddrLocol.GetIpDisplayName());

    CFSPairInetAddr addrPair(aAddr, m_AddrLocol);
#ifdef CAW_DEBUG
    FSUdpTransportsType::size_type nErase = 
#endif // CAW_DEBUG
    s_pfsUdpTransports->erase(addrPair);
    CAW_ASSERTE(nErase == 1);
    return CAW_OK;
}

int CFSAcceptorUdp::OnEpollInput(int fd)
{
    static char szBuf[1024*16];
    CAWInetAddr addrRecv;

    for(;;)
    {
        int nRecv = m_Socket.RecvFrom(szBuf, sizeof(szBuf), addrRecv);
        if ((nRecv == -1) 
            && ((errno == VOS_EAGAIN) ||(errno == VOS_EWOULDBLOCK)))
        {
            break;
        }
        else if (nRecv < 0)
        {
            CAW_ERROR_TRACE("CFSAcceptorUdp::OnInput,error");
            break;
        }
        CFSTransportUdp *pTrans = NULL;
        CFSPairInetAddr addrPair(addrRecv, m_AddrLocol);
        FSUdpTransportsType::iterator iter = s_pfsUdpTransports->find(addrPair);
        if (iter == s_pfsUdpTransports->end()) {
            // create UDP transport with network reactor in the network thread.
            pTrans = new CFSTransportUdp(m_pReactor, addrRecv, this);
            if (!pTrans)
                break;

            pTrans->GetPeer().SetHandle(m_Socket.GetHandle());

            // it will do AddRefenceControl() twice.
            FSUdpTransportsType::value_type nodeNew(addrPair, pTrans);
            s_pfsUdpTransports->insert(nodeNew);

            CAW_ASSERTE(m_pSink);
            if (m_pSink)
                m_pSink->OnConnectIndication(CAW_OK, pTrans, this);
        }
        else {
            pTrans = (*iter).second.Get();
        }


        CAW_ASSERTE(pTrans);
        pTrans->OnReceiveCallback(szBuf, nRecv);
    }
    return 0;
}
}//namespace fsutils

