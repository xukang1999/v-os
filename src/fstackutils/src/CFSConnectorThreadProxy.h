/***********************************************************************
 * Copyright (C) 2016-2018, Nanjing StarOS Technology Co., Ltd 
**********************************************************************/

#ifndef CFSCONNECORTHREADPROXY_H
#define CFSCONNECORTHREADPROXY_H

#include "fstackutils/CFSInterface.h"
#include "CFSAcceptorConnectorSinkThreadProxy.h"
#include "ipstacktp/CPSReactorInterface.h"
using namespace ipstacktp;
namespace fsutils
{
class CFSConnectorThreadProxy 
    : public IFSConnector
    , public CFSAcceptorConnectorSinkThreadProxyT<CFSConnectorThreadProxy>
    , public CAWReferenceControlMutilThread
    , public CAWStopFlag
{
public:
    CFSConnectorThreadProxy(
        IFSConnectionManager::CType aType,
        IPSReactor *aThreadNetwork = NULL,
        CAWThread *aThreadUser = NULL);

    virtual ~CFSConnectorThreadProxy();

    // interface IAWReferenceControl
    virtual DWORD AddReference();
    virtual DWORD ReleaseReference();
    virtual void OnReferenceDestory();

    // interface IAWAcceptorConnectorId
    virtual BOOL IsConnector();

    // interface IAWConnector
    virtual void AsycConnect(
        IFSAcceptorConnectorSink *aSink,
        const CAWInetAddr &aAddrPeer,
        CAWTimeValue *aTimeout = NULL,
        CAWInetAddr *aAddrLocal = NULL);

    virtual void CancelConnect();

    IFSAcceptorConnectorId* GetActualAcceptorConnectorId()
    {
        return m_pConActual.Get();
    }

    // we have to overlaod this function we want to ResetSink.
    void SetStopFlag();

private:
    CAWThread *m_pThreadUser;
    IPSReactor *m_pThreadNetwork;
    CAWAutoPtr<IFSConnector> m_pConActual;
    IFSConnectionManager::CType m_Type;

    friend class CFSEventAsycConnect;
    friend class CFSEventCancelConnect;
    friend class CFSAcceptorConnectorSinkThreadProxyT<CFSConnectorThreadProxy>;
    friend class CFSEventOnConnectIndication<CFSConnectorThreadProxy>;
};

class CFSEventAsycConnect : public IAWEvent
{
public:
    CFSEventAsycConnect(
        CFSConnectorThreadProxy *aConnectorThreadProxy,
        IFSAcceptorConnectorSink *aSink, 
        const CAWInetAddr &aAddrPeer, 
        CAWTimeValue *aTimeout,
        CAWInetAddr *aAddrLocal);

    virtual ~CFSEventAsycConnect();

    virtual CAWResult OnEventFire();

private:
    CAWAutoPtr<CFSConnectorThreadProxy> m_pOwnerThreadProxy;
    IFSAcceptorConnectorSink *m_pSink;
    CAWInetAddr m_addrPeer;
    CAWTimeValue m_tvTimeout;
    CAWTimeValue *m_pParaTimeout;
    CAWInetAddr *m_pParaAddrLocal;
    CAWInetAddr m_addrLocal;
};

class CFSEventCancelConnect : public IAWEvent
{
public:
    CFSEventCancelConnect(CFSConnectorThreadProxy *aConnectorThreadProxy);

    virtual ~CFSEventCancelConnect();

    virtual CAWResult OnEventFire();

private:
    CAWAutoPtr<CFSConnectorThreadProxy> m_pOwnerThreadProxy;
};
}//namespace fsutils
#endif // !CMCONNECORTHREADPROXY_H

