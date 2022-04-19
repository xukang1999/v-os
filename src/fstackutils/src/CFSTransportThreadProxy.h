/***********************************************************************
 * Copyright (C) 2016-2018, Nanjing StarOS Technology Co., Ltd 
**********************************************************************/
#ifndef CFSTRANSPORTTHREADPROXY_H
#define CFSTRANSPORTTHREADPROXY_H

#include "fstackutils/CFSInterface.h"
#include "ipstacktp/CPSReactorInterface.h"
using namespace ipstacktp;
namespace fsutils
{
class CFSTransportThreadProxy 
    : public IFSTransport
    , public IFSTransportSink
    , public CAWReferenceControlMutilThread
    , public CAWStopFlag
    , public CAWTimerWrapperIDSink
{
public:
    CFSTransportThreadProxy(
            IFSTransport *aActual, 
            IPSReactor *aThreadNetwork,
            CAWThread *aThreadUser,
            IFSConnectionManager::CType aType);

    virtual ~CFSTransportThreadProxy();

    // interface IAWReferenceControl
    virtual DWORD AddReference();
    virtual DWORD ReleaseReference();
    DWORD GetRefCount();
    virtual void OnReferenceDestory();

    // interface IAWTransport
    virtual CAWResult OpenWithSink(IFSTransportSink *aSink);
    virtual IFSTransportSink* GetSink();
    virtual CAWResult SendData(CAWMessageBlock &aData, CFSTransportParameter *aPara = NULL);
    virtual CAWResult SetOption(DWORD aCommand, LPVOID aArg);
    virtual CAWResult GetOption(DWORD aCommand, LPVOID aArg);
    virtual CAWResult Disconnect(CAWResult aReason);
    virtual int GetTransportHandle() const;

    // interface IAWTransportSink
    virtual void OnReceive(
        CAWMessageBlock &aData,
        IFSTransport *aTrptId,
        CFSTransportParameter *aPara = NULL);

    virtual void OnSend(
        IFSTransport *aTrptId,
        CFSTransportParameter *aPara = NULL);

    virtual void OnDisconnect(
        CAWResult aReason,
        IFSTransport *aTrptId);

    virtual void OnTimer(CAWTimerWrapperID* aId);
    BOOL m_isOnRecieve;


private:
    CAWResult Send_i(
        CAWMessageBlock *aData, 
        CFSTransportParameter* aPara,
        BOOL aIsDuplicated);

    CAWAutoPtr<IFSTransport> m_pTransportActual;
    IFSTransportSink *m_pSinkActual;
    CAWThread *m_pThreadUser;
    IPSReactor *m_pThreadNetwork;
    IFSConnectionManager::CType m_Type;
    CAWTimerWrapperID m_TimerReference0;

    struct CItem 
    {
        CItem(CAWMessageBlock *aMb, CFSTransportParameter *aPara);
        ~CItem();

        CAWMessageBlock *m_pMbSend;
        CFSTransportParameter m_TransportParameter;
        CFSTransportParameter *m_pParaTransportParameter;
    };
    typedef std::list<CItem> SendBufferType;

    typedef std::list<CItem> RcvBufferType;


    // for send buffer, we don't need mutex becase.
    // <m_SendBuffer> and <m_bIsBufferFull> are all modified in the network thread.
    // <m_bNeedOnSend> is modified in the user thread.
    BOOL m_bIsBufferFull;
    BOOL m_bNeedOnSend;
    SendBufferType m_SendBuffer;
    BOOL m_bPartialDataLastTime;	//Indicate last time send error is CAW_ERROR_PARTIAL_DATA

    friend class CFSEventSendData;
    friend class CFSEventDisconnect;
    friend class CFSEventOnReceive;
    friend class CFSEventOnSend;
    friend class CFSEventOnDisconnect;
    friend class CFSEventOpenWithSink;
};

class CFSEventSendData : public IAWEvent
{
public:
    CFSEventSendData(
        CFSTransportThreadProxy *aThreadProxy,
        CAWMessageBlock &aData, 
        CFSTransportParameter *aPara);

    virtual ~CFSEventSendData();

    virtual CAWResult OnEventFire();

private:
    CAWAutoPtr<CFSTransportThreadProxy> m_pOwnerThreadProxy;
    CAWMessageBlock *m_pMessageBlock;
    CFSTransportParameter m_TransportParameter;
    CFSTransportParameter *m_pParaTransportParameter;
};

class CFSEventDisconnect : public IAWEvent
{
public:
    CFSEventDisconnect(
        CFSTransportThreadProxy *aThreadProxy,
        CAWResult aReason);

    virtual ~CFSEventDisconnect();

    virtual CAWResult OnEventFire();

private:
    CAWAutoPtr<CFSTransportThreadProxy> m_pOwnerThreadProxy;
    CAWResult m_Reason;
};

class CFSEventOpenWithSink : public IAWEvent
{
public:
    CFSEventOpenWithSink(
        CFSTransportThreadProxy *aThreadProxy);

    virtual ~CFSEventOpenWithSink();

    virtual CAWResult OnEventFire();

private:
    CAWAutoPtr<CFSTransportThreadProxy> m_pOwnerThreadProxy;
};

class CFSEventOnReceive : public IAWEvent
{
public:
    CFSEventOnReceive(
        CFSTransportThreadProxy *aThreadProxy,
        CAWMessageBlock &aData,
        IFSTransport *aTrptId,
        CFSTransportParameter *aPara);

    virtual ~CFSEventOnReceive();

    virtual CAWResult OnEventFire();

private:
    CAWAutoPtr<CFSTransportThreadProxy> m_pOwnerThreadProxy;
    CAWMessageBlock *m_pData;
    IFSTransport *m_pTrptId;
    CFSTransportParameter m_TransportParameter;
    CFSTransportParameter *m_pParaTransportParameter;
};

class CFSEventOnSend : public IAWEvent
{
public:
    CFSEventOnSend(
        CFSTransportThreadProxy *aThreadProxy,
        IFSTransport *aTrptId,
        CFSTransportParameter *aPara);

    virtual ~CFSEventOnSend();

    virtual CAWResult OnEventFire();

private:
    CAWAutoPtr<CFSTransportThreadProxy> m_pOwnerThreadProxy;
    IFSTransport *m_pTrptId;
    CFSTransportParameter m_TransportParameter;
    CFSTransportParameter *m_pParaTransportParameter;
};

class CFSEventOnDisconnect : public IAWEvent
{
public:
    CFSEventOnDisconnect(
        CFSTransportThreadProxy *aThreadProxy,
        CAWResult aReason,
        IFSTransport *aTrptId);

    virtual ~CFSEventOnDisconnect();

    virtual CAWResult OnEventFire();

private:
    CAWAutoPtr<CFSTransportThreadProxy> m_pOwnerThreadProxy;
    CAWResult m_Reason;
    IFSTransport *m_pTrptId;
};
}//namespace fsutils
#endif // !CFSTRANSPORTTHREADPROXY_H

