/***********************************************************************
 * Copyright (C) 2016-2018, Nanjing StarOS Technology Co., Ltd 
**********************************************************************/

#ifndef CFSINTERFACE_H
#define CFSINTERFACE_H
#include "wface/CAWACEWrapper.h"
#include "starbase/CAWHashMapT.h"
#include "starbase/CAWUtilTemplates.h"
#include "starbase/CAWString.h"
#include "starbase/CAWDefines.h"
#include "starbase/CAWReferenceControl.h"
#include "starbase/CAWMutex.h"
#include "starbase/CAWThreadManager.h"
#include "starbase/CAWInetAddr.h"
#include "starbase/CAWTimeValue.h"
#include "starbase/CAWUtilClasses.h"
#include "starbase/CAWMessageBlock.h"
#include "starbase/CAWTimerQueueOrderedList.h"
#include "ipstacktp/CPSReactorInterface.h"
using namespace ipstacktp;
using namespace starbase;
using namespace wface;
namespace fsutils
{
class IFSAcceptorConnectorSink;
class IFSTransportSink;
class IFSTransport;
class IFSAcceptorConnectorId;
class IFSConnector;
class IFSAcceptor;

class CAW_OS_EXPORT IFSConnectionManager
{
public:
    static IFSConnectionManager* Instance();

    typedef DWORD CType;
    enum { 
        // connection type
        CTYPE_NONE = 0,
        CTYPE_TCP = (1 << 0),
        CTYPE_UDP = (1 << 1)
    };
    enum CPriority
    {
        CPRIORITY_HIGH,
        CPRIORITY_ABOVE_NORMAL,
        CPRIORITY_NORMAL,
        CPRIORITY_BELOW_NORMAL,
        CPRIORITY_LOW,
    };

    enum 
    {
        UDP_SEND_MAX_LEN = 16 * 1024,
        UDP_ONE_PACKET_MAX_LEN = 1514-14-20-8,
    };

    virtual CAWResult CreateConnectionClient(CType aType, IFSConnector *&aConClient) = 0;
    virtual CAWResult CreateConnectionServer(CType aType, IFSAcceptor *&aAcceptor) = 0;
    virtual CAWThread *GetNetworkThread() = 0;
protected:
    virtual ~IFSConnectionManager() {}
};

class CAW_OS_EXPORT CFSTransportParameter
{
public:
    CFSTransportParameter(IFSConnectionManager::CPriority aPriority = IFSConnectionManager::CPRIORITY_NORMAL)
        : m_dwHaveSent(0)
        , m_Priority(aPriority)
    {
    }

    DWORD m_dwHaveSent;
    IFSConnectionManager::CPriority m_Priority;
};

class CAW_OS_EXPORT IFSAcceptorConnectorSink 
{
public:
    virtual void OnConnectIndication(
        CAWResult aReason,
        IFSTransport *aTrpt,
        IFSAcceptorConnectorId *aRequestId) = 0;

protected:
    virtual ~IFSAcceptorConnectorSink() {}
};

class CAW_OS_EXPORT IFSTransportSink 
{
public:
    virtual void OnReceive(
        CAWMessageBlock &aData,
        IFSTransport *aTrptId,
        CFSTransportParameter *aPara = NULL) = 0;

    virtual void OnSend(
        IFSTransport *aTrptId,
        CFSTransportParameter *aPara = NULL) = 0;

    virtual void OnDisconnect(
    CAWResult aReason,
    IFSTransport *aTrptId) = 0;

   protected:
    virtual ~IFSTransportSink() {}
};

class CAW_OS_EXPORT IFSTransport : public IAWReferenceControl
{
public:
    virtual CAWResult OpenWithSink(IFSTransportSink *aSink) = 0;

    virtual IFSTransportSink* GetSink() = 0;
    virtual CAWResult SendData(CAWMessageBlock &aData, CFSTransportParameter *aPara = NULL) = 0;
    virtual CAWResult SetOption(DWORD aCommand, LPVOID aArg) = 0;
    virtual CAWResult GetOption(DWORD aCommand, LPVOID aArg) = 0;
    virtual CAWResult Disconnect(CAWResult aReason) = 0;
    virtual int GetTransportHandle() const = 0;

protected:
    virtual ~IFSTransport() {}
};

class CAW_OS_EXPORT IFSAcceptorConnectorId : public IAWReferenceControl
{
public:
    virtual BOOL IsConnector() = 0;

protected:
    virtual ~IFSAcceptorConnectorId() {}
};

class CAW_OS_EXPORT IFSConnector : public IFSAcceptorConnectorId
{
public:
    virtual void AsycConnect(
        IFSAcceptorConnectorSink *aSink,
        const CAWInetAddr &aAddrPeer, 
        CAWTimeValue *aTimeout = NULL,
        CAWInetAddr *aAddrLocal = NULL) = 0;
    virtual void CancelConnect() = 0;

protected:
    virtual ~IFSConnector() {}
};

class CAW_OS_EXPORT IFSAcceptor : public IFSAcceptorConnectorId
{
public:
    virtual CAWResult StartListen(IFSAcceptorConnectorSink *aSink,const CAWInetAddr &aAddrListen) = 0;
    virtual CAWResult StopListen(CAWResult aReason) = 0;
protected:
    virtual ~IFSAcceptor() {}
};

class CAW_OS_EXPORT IFSConnectorInternal
{
public:
    virtual int Connect(const CAWInetAddr &aAddr, CAWInetAddr *aAddrLocal = NULL) = 0;
    virtual int Close() = 0;
    virtual ~IFSConnectorInternal() { }
};
}//namespace fsutils
#endif // IFSCONNECTIONINTERFACE_H

