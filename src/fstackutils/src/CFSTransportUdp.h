/***********************************************************************
 * Copyright (C) 2016-2018, Nanjing StarOS Technology Co., Ltd 
**********************************************************************/

#ifndef CFSTRANSPORTUDP_H
#define CFSTRANSPORTUDP_H

#include "CFSTransportBase.h"
#include "fstackutils/CFSSocket.h"
namespace fsutils
{
class CFSAcceptorUdp;

class CFSTransportUdp : public CFSTransportBase  
{
public:
    CFSTransportUdp(IPSReactor *pReactor, const CAWInetAddr &aAddrSend, CFSAcceptorUdp *pAcceptor = NULL);
    virtual ~CFSTransportUdp();

    // interface IAWEventHandler
    virtual int GetHandle() const ;
    virtual int OnInput(int aFd = -1);

    // interface IAWTransport
    virtual CAWResult SendData(CAWMessageBlock &aData, CFSTransportParameter *aPara = NULL);
    virtual CAWResult SetOption(DWORD aCommand, LPVOID aArg);
    virtual CAWResult GetOption(DWORD aCommand, LPVOID aArg);

    CFSSocketUdp& GetPeer();

    void OnReceiveCallback(LPSTR aData, DWORD aLen);
    virtual int GetTransportHandle() const;

protected:
    virtual CAWResult Open_t();
    virtual CAWResult Close_t(CAWResult aReason);
    virtual CAWResult Disconnect_t(CAWResult aReason);
    int OnEpollInput(int aFd);
private:
    CFSSocketUdp m_SocketUdp;
    CAWAutoPtr<CFSAcceptorUdp> m_pAcceptor;
    CAWInetAddr m_AddrSend;
};


// inline functions
inline void CFSTransportUdp::OnReceiveCallback(LPSTR aData, DWORD aLen)
{
    CAWMessageBlock mbOn(
                aLen, 
                aData, 
                CAWMessageBlock::DONT_DELETE | CAWMessageBlock::WRITE_LOCKED, 
                aLen);

    CAW_ASSERTE(m_pSink);
    if (m_pSink)
        m_pSink->OnReceive(mbOn, this);
}
}//namespace fsutils
#endif // !CFSTRANSPORTUDP_H

