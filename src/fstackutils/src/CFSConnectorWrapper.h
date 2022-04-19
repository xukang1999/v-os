/***********************************************************************
 * Copyright (C) 2016-2018, Nanjing StarOS Technology Co., Ltd 
**********************************************************************/

#ifndef CFSCONNECTORWRAPPER_H
#define CFSCONNECTORWRAPPER_H

#include "fstackutils/CFSInterface.h"
#include "ipstacktp/CPSReactorInterface.h"
using namespace ipstacktp;
namespace fsutils
{
class CFSConnectorWrapper 
    : public IFSConnector
    , public IAWTimerHandler
    , public CAWReferenceControlSingleThread
    {
public:
    CFSConnectorWrapper(IPSReactor *pNetworkThread);
    virtual ~CFSConnectorWrapper();

    // interface IAWReferenceControl
    virtual DWORD AddReference();
    virtual DWORD ReleaseReference();

    virtual CAWResult Init(IFSConnectionManager::CType aType);

    virtual int OnConnectIndication(
            CAWResult aReason, 
            IFSTransport *aTrpt,
            IFSConnectorInternal *aId);

    // interface IAWAcceptorConnectorId
    virtual BOOL IsConnector();

    // interface IAWConnector
    virtual void AsycConnect(
            IFSAcceptorConnectorSink* aSink,
            const CAWInetAddr& aAddrPeer, 
            CAWTimeValue* aTimeout = NULL,
            CAWInetAddr *aAddrLocal = NULL);

    virtual void CancelConnect();

protected:
    // interface IAWTimerHandler
    virtual void OnTimeout(const CAWTimeValue &aCurTime, LPVOID aArg);

    void Close_i();

protected:
    IPSReactor *m_pReactor;
    IFSAcceptorConnectorSink *m_pSink;
    IFSConnectorInternal *m_pConnector;
    BOOL m_bClosed;
};
}//namespace fsutils
#endif // !CFSCONNECTORWRAPPER_H

