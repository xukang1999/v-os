/***********************************************************************
 * Copyright (C) 2016-2018, Nanjing StarOS Technology Co., Ltd 
**********************************************************************/
#ifndef CFSACCEPTORUDP_H
#define CFSACCEPTORUDP_H

#include "CFSAcceptorBase.h"
#include "fstackutils/CFSSocket.h"
#include "CFSTransportUdp.h"
namespace fsutils
{
class CFSAcceptorUdp 
    : public CFSAcceptorBase
    , public IPSEventHandler 
    , public CAWStopFlag
{
public:
    CFSAcceptorUdp(IPSReactor *pNetworkThread);
    virtual ~CFSAcceptorUdp();

    // interface IAWAcceptor
    virtual CAWResult StartListen(IFSAcceptorConnectorSink *aSink,const CAWInetAddr &aAddrListen);
    virtual CAWResult StopListen(CAWResult aReason);

    // iterface IAWEventHandler
    virtual int GetHandle() const ;
    virtual int OnInput(int aFd = -1);
    virtual int OnClose(int aFd, MASK aMask);
    int OnEpollInput(int fd);

    // it will be invoked by CAWTransportUdp::Close_t().
    CAWResult RemoveTransport(const CAWInetAddr &aAddr, CFSTransportUdp *aTrpt);

private:
    CFSSocketUdp m_Socket;
    CAWInetAddr m_AddrLocol;
};
}//namespace fsutils
#endif // !CFSACCEPTORUDP_H

