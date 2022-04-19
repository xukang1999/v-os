/***********************************************************************
 * Copyright (C) 2016-2018, Nanjing StarOS Technology Co., Ltd 
**********************************************************************/
#ifndef CFSCONNECTORUDPT_H
#define CFSCONNECTORUDPT_H

#include "fstackutils/CFSInterface.h"
#include "ipstacktp/CPSReactorInterface.h"
using namespace ipstacktp;
using namespace starbase;
namespace fsutils
{
template <class UpperType, class UpTrptType, class UpSockType>
class CAW_OS_EXPORT CFSConnectorUdpT 
    : public IPSEventHandler
    , public IFSConnectorInternal
    , public IAWTimerHandler
{
public:
    CFSConnectorUdpT(IPSReactor *aReactor, UpperType &aUpper)
        : m_pReactor(aReactor)
        , m_Upper(aUpper)
        , m_pTransport(NULL)
        , m_bResolving(FALSE)
    {
    }

    virtual ~CFSConnectorUdpT()
    {
        Close();
    }

	// interface ACmConnectorInternal
	virtual int Connect(const CAWInetAddr &aAddr, CAWInetAddr *aAddrLocal = NULL)
	{

		const CAWInetAddr *pAddrConnect = &aAddr;
		if (aAddrLocal && aAddrLocal != &m_addrLocal)
			m_addrLocal = *aAddrLocal;

		int nRet = 0;
		CAW_ASSERTE_RETURN(!m_pTransport, -1);
		m_pTransport = new UpTrptType(m_pReactor, *pAddrConnect);
		if (!m_pTransport) 
			return -1;

		UpSockType &sockPeer = m_pTransport->GetPeer();
		CAW_ASSERTE(sockPeer.GetHandle() == -1);
		if (sockPeer.Open(m_addrLocal) == -1) {
			CAW_ERROR_TRACE("CFSConnectorUdpT::Connect, m_Socket.Open() failed!"
				" addr=" << m_addrLocal.GetIpDisplayName() <<
				"err=" << errno);
			return -1;
		}

		nRet = vos_sock_connect(
			(CAW_SOCKET)sockPeer.GetHandle(), 
			reinterpret_cast<const struct vos_sock_sockaddr *>(pAddrConnect->GetPtr()), 
			pAddrConnect->GetSize());
		if (nRet == -1) {
			CAW_WARNING_TRACE("CFSConnectorUdpT::Connect, connect() failed!"
				" addr=" << pAddrConnect->GetIpDisplayName() <<
				" err=" << errno);
			return -1;
		}
		else {
			CAW_INFO_TRACE("CFSConnectorUdpT::Connect, connect() successful."
				" addr=" << pAddrConnect->GetIpDisplayName() <<
				" fd=" << sockPeer.GetHandle());


			// can't use NotifyHandler(WRITE_MASK) due to not RegiesterHandler.
#ifdef CAW_DEBUG
			CAWResult rv = 
#endif // CAW_DEBUG
				m_pReactor->ScheduleTimer(this, NULL, CAWTimeValue::s_tvZero(), 1);
			CAW_ASSERTE(CAW_SUCCEEDED(rv));
			return 0;
		}
	}

    virtual int Close()
    {
        if (m_pReactor)
            m_pReactor->CancelTimer(this);
        if (m_pTransport) {
            delete m_pTransport;
            m_pTransport = NULL;
        }
        return 0;
    }

    /// interface IAWEventHandler
    virtual int GetHandle() const 
    {
        CAW_ASSERTE_RETURN(m_pTransport, -1);
        return m_pTransport->GetHandle();
    }

    virtual int OnClose(int aFd, MASK aMask)
    {
        CAW_ERROR_TRACE("CFSConnectorUdpT::OnClose, it's impossible!"
            " aFd=" << aFd <<
            " aMask=" << aMask);
        return 0;
    }

    void OnTimeout(const CAWTimeValue &aCurTime, LPVOID aArg)
    {
        CAW_UNUSED_ARG(aCurTime);
        CAW_UNUSED_ARG(aArg);
        UpTrptType* pTrans = m_pTransport;
        m_pTransport = NULL;
        m_Upper.OnConnectIndication(CAW_OK, pTrans, this);
    }


private:
    IPSReactor *m_pReactor;
    UpperType &m_Upper;
    UpTrptType *m_pTransport;
    CAWInetAddr m_addrUnResolved;
    CAWInetAddr m_addrLocal;
    BOOL m_bResolving;
};
}//namespace fsutils
#endif // !CFSCONNECTORUDPT_H
