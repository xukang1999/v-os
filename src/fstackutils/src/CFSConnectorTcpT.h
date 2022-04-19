/***********************************************************************
 * Copyright (C) 2016-2018, Nanjing StarOS Technology Co., Ltd 
**********************************************************************/

#ifndef CFSCONNECTORTCPT_H
#define CFSCONNECTORTCPT_H
#include "fstackutils/CFSSocket.h"
#include "ipstacktp/CPSReactorInterface.h"
using namespace ipstacktp;
namespace fsutils
{
template <class UpperType, class UpTrptType, class UpSockType>
class CAW_OS_EXPORT CFSConnectorTcpT 
    : public IPSEventHandler
    , public IFSConnectorInternal
{
public:
    CFSConnectorTcpT(IPSReactor *aReactor, UpperType &aUpper)
        : m_pReactor(aReactor)
        , m_Upper(aUpper)
        , m_bResolving(FALSE)
    {
        CAW_INFO_TRACE_THIS("CFSConnectorTcpT::CFSConnectorTcpT");
    }

    virtual ~CFSConnectorTcpT()
    {
        CAW_INFO_TRACE_THIS("CFSConnectorTcpT::~CFSConnectorTcpT");
        Close();
    }

    // interface ACmConnectorInternal
    virtual int Connect(const CAWInetAddr &aAddr, CAWInetAddr *aAddrLocal = NULL)
    {
        int nRet = 0;
        const CAWInetAddr *pAddrConnect = &aAddr;
        if (aAddrLocal)
            m_addrLocal = *aAddrLocal;
        CAW_INFO_TRACE_THIS("CFSConnectorTcpT::Connect,m_addrLocal="<<m_addrLocal.GetIpDisplayName()<<",peer="<<aAddr.GetIpDisplayName());

        m_pTransport = new UpTrptType(m_pReactor);
        if (!m_pTransport.Get())
        {
            CAW_ERROR_TRACE_THIS("CFSConnectorTcpT::Connect create UpTrptType error");
            return -1;
        }

        nRet = Connect_i(m_pTransport.Get(), *pAddrConnect);
        if (nRet == 0) 
        {
            // it rarely happens. we have to OnConnectIndication(CAW_OK) to upper layer.
            CAW_WARNING_TRACE_THIS("CFSConnectorTcpT::Connect, connect return 0.");
            nRet = m_pReactor->NotifyHandler(this, IAWEventHandler::WRITE_MASK);
        }
        else if (nRet == 1)
            nRet = 0;
        return nRet;
        }

    virtual int Close()
    {
        CAW_INFO_TRACE_THIS("CFSConnectorTcpT::Close");

        if (m_pTransport.Get()) 
        {
            m_pReactor->RemoveHandler(this, IAWEventHandler::CONNECT_MASK);
            //delete m_pTransport;
            m_pTransport = NULL;
        }
        return 0;
    }

    /// interface IAWEventHandler
    virtual int GetHandle() const 
    {
        CAW_ASSERTE_RETURN(m_pTransport.Get(), -1);
        return m_pTransport->GetHandle();
    }

    /// OnOutput() indicating Connect successful,
    /// OnClose() indicating Connect failed.
    //  virtual int OnInput(CAW_HANDLE aFd = CAW_INVALID_HANDLE);
    virtual int OnOutput(int aFd = -1)
    {
        //printf("CFSConnectorTcpT OnOutput\n");
        CAW_INFO_TRACE_THIS("CFSConnectorTcpT::OnOutput");
        //CAW_ASSERTE(m_pTransport.Get());
        //CAW_ASSERTE(aFd == m_pTransport->GetHandle());

        //UpTrptType* pTrans = m_pTransport;
        m_Upper.OnConnectIndication(CAW_OK, m_pTransport.Get(), this);
        //m_pTransport = NULL;
        
        return 0;
    }

    virtual int OnClose(int aFd, MASK aMask)
    {
        CAW_INFO_TRACE_THIS("CFSConnectorTcpT::OnClose");
        //CAW_ASSERTE(m_pTransport);
        //CAW_ASSERTE(aFd == m_pTransport->GetHandle());
        //CAW_ASSERTE(aMask == IAWEventHandler::CONNECT_MASK);

        Close();
        m_Upper.OnConnectIndication(CAW_ERROR_NETWORK_SOCKET_ERROR, NULL, this);
        return 0;
    }


private:
    int Connect_i(UpTrptType *aTrpt, const CAWInetAddr &aAddr)
    {

        int nRet;
        UpSockType &sockPeer = aTrpt->GetPeer();
        CAW_ASSERTE(sockPeer.GetHandle() == -1);
        
        if (m_addrLocal == CAWInetAddr::s_InetAddrAny())
            nRet = sockPeer.Open(FALSE);
        else
            nRet = sockPeer.Open(FALSE, m_addrLocal);
        if (nRet == -1) 
        {
            CAW_ERROR_TRACE_THIS("CAWConnectorTcpT::Connect_i, Open() failed!"
            " laddr=" << m_addrLocal.GetIpDisplayName() <<
            " lport=" << m_addrLocal.GetPort() << 
            " err=" << errno);
            return -1;
        }
        if (sockPeer.Enable(CFSIPCBase::NON_BLOCK) == -1) 
        {
            CAW_ERROR_TRACE_THIS("CAWConnectorTcpT::Connect_i, Enable(NON_BLOCK) failed! err=" << errno);
            return -1;
        }
        
        CAW_INFO_TRACE_THIS("CAWConnectorTcpT::Connect_i,"
                    " addr=" << aAddr.GetIpDisplayName() << 
                    " port=" << aAddr.GetPort() << 
                    " laddr=" << m_addrLocal.GetIpDisplayName() <<
                    " lport=" << m_addrLocal.GetPort() << 
                    " fd=" << sockPeer.GetHandle() << 
                    " aTrpt=" << aTrpt);


        /// we regiester CONNECT_MASK prior to connect() to avoid lossing OnConnect()
        CAWResult rv = m_pReactor->RegisterHandler(this, IPSEventHandler::CONNECT_MASK);
        if (CAW_FAILED(rv))
            return -1;

        nRet = vos_sock_connect((int)sockPeer.GetHandle(), 
                    reinterpret_cast<const struct vos_sock_sockaddr *>(aAddr.GetPtr()), 
                    aAddr.GetSize());
        if (nRet == -1 && errno == VOS_EINPROGRESS)
            errno = VOS_EWOULDBLOCK;
        if (nRet == -1) 
        {
            if (errno == VOS_EWOULDBLOCK)
                return 1;
            else 
            {
                CAW_ERROR_TRACE_THIS("CFSConnectorTcpT::Connect_i, connect() failed!"
                        " addr=" << aAddr.GetIpDisplayName() <<
                        " err=" << errno<<
                        " ewouldblock="<< VOS_EWOULDBLOCK);
                return -1;
            }
        }
        else
            return 0;
    }

    IPSReactor *m_pReactor;
    UpperType &m_Upper;
    CAWAutoPtr<UpTrptType> m_pTransport;
    CAWInetAddr m_addrUnResolved;
    CAWInetAddr m_addrLocal;
    BOOL m_bResolving;
};
}//namespace fsutils
#endif // !CFSCONNECTORTCPT_H

