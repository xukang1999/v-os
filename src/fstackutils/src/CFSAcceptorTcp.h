/***********************************************************************
 * Copyright (C) 2016-2018, Nanjing StarOS Technology Co., Ltd 
**********************************************************************/

#ifndef CFSACCEPTORTCP_H
#define CFSACCEPTORTCP_H

#include "CFSAcceptorBase.h"
#include "fstackutils/CFSSocket.h"
#include "ipstacktp/CPSReactorInterface.h"
using namespace ipstacktp;
namespace fsutils
{
class CAW_OS_EXPORT CFSAcceptorTcp 
        : public CFSAcceptorBase
        , public IPSEventHandler
{
public:
    CFSAcceptorTcp(IPSReactor *pThreadNetwork);
    virtual ~CFSAcceptorTcp();

    // interface IAWAcceptor
    virtual CAWResult StartListen(IFSAcceptorConnectorSink *aSink,const CAWInetAddr &aAddrListen);
    virtual CAWResult StopListen(CAWResult aReason);

    // iterface IAWEventHandler
    virtual int GetHandle() const ;
    virtual int OnInput(int aFd = -1);
    virtual int OnClose(int aFd, MASK aMask);
    int OnEpollInput(int aFd);
private:
    CFSSocketTcp m_Socket;
};
}//namespace fsutils
#endif // !CFSACCEPTORTCP_H

