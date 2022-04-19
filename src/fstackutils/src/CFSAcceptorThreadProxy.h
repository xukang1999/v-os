/***********************************************************************
 * Copyright (C) 2016-2018, Nanjing StarOS Technology Co., Ltd 
**********************************************************************/

#ifndef CFSACCEPTORTHREADPROXY_H
#define CFSACCEPTORTHREADPROXY_H
#include "fstackutils/CFSInterface.h"
#include "CFSAcceptorConnectorSinkThreadProxy.h"
#include "ipstacktp/CPSReactorInterface.h"
using namespace ipstacktp;
namespace fsutils
{
class CFSAcceptorThreadProxy
    : public IFSAcceptor
    , public CAWReferenceControlMutilThread 
    , public CFSAcceptorConnectorSinkThreadProxyT<CFSAcceptorThreadProxy>
    , public CAWStopFlag
{
public:
    CFSAcceptorThreadProxy(IFSConnectionManager::CType aType,IPSReactor *aThreadNetwork = NULL,CAWThread *aThreadUser = NULL);
    virtual ~CFSAcceptorThreadProxy();

    // interface IAWReferenceControl
    virtual DWORD AddReference();
    virtual DWORD ReleaseReference();
    virtual void OnReferenceDestory();

    // interface IAWAcceptorConnectorId
    virtual BOOL IsConnector();

    // interface IAWAcceptor
    virtual CAWResult StartListen(IFSAcceptorConnectorSink *aSink,const CAWInetAddr &aAddrListen);

    virtual CAWResult StopListen(CAWResult aReason);

    IFSAcceptorConnectorId* GetActualAcceptorConnectorId()
    {
        return m_pAcceptorActual.Get();
    }

    // we have to overlaod this function because class 
    // CAWAcceptorConnectorSinkThreadProxyT<CAWAcceptorBase> will invoke it.
    void SetStopFlag() { }

private:
    CAWThread *m_pThreadUser;
    IPSReactor *m_pThreadNetwork;
    CAWAutoPtr<IFSAcceptor> m_pAcceptorActual;
    IFSConnectionManager::CType m_Type;

    friend class CFSEventStartListen;
    friend class CFSEventStopListen;
    friend class CFSAcceptorConnectorSinkThreadProxyT<CFSAcceptorThreadProxy>;
    friend class CFSEventOnConnectIndication<CFSAcceptorThreadProxy>;
};

class CFSEventStartListen : public IAWEvent
{
public:
    CFSEventStartListen(CFSAcceptorThreadProxy *aThreadProxy,IFSAcceptorConnectorSink *aSink,const CAWInetAddr &aAddrListen);
    virtual ~CFSEventStartListen();
    virtual CAWResult OnEventFire();
private:
    CAWAutoPtr<CFSAcceptorThreadProxy> m_pOwnerThreadProxy;
    IFSAcceptorConnectorSink *m_pSink;
    CAWInetAddr m_addrListen;
};

class CFSEventStopListen : public IAWEvent
{
public:
    CFSEventStopListen(CFSAcceptorThreadProxy *aConnectorThreadProxy,CAWResult aReason);
    virtual ~CFSEventStopListen();
    virtual CAWResult OnEventFire();
private:
    CAWAutoPtr<CFSAcceptorThreadProxy> m_pOwnerThreadProxy;
    CAWResult m_Reason;
};

}//namespace fsutils
#endif // !CMACCEPTORTHREADPROXY_H

