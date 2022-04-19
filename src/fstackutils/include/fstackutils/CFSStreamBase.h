/***********************************************************************
 * Copyright (C) 2016-2018, Nanjing StarOS Technology Co., Ltd 
**********************************************************************/
#ifndef CFSSTREAMBASE_H
#define CFSSTREAMBASE_H
#include "fstackutils/CFSInterface.h"
using namespace starbase;
namespace fsutils
{
class CAW_OS_EXPORT CFSStreamBase : public IFSTransportSink,
                        public IFSAcceptorConnectorSink,
                        public CAWReferenceControlSingleThread
{ 
public: 
    CFSStreamBase();
    virtual ~CFSStreamBase();

    virtual void OnConnectIndication(
        CAWResult aReason,
        IFSTransport *aTrpt,
        IFSAcceptorConnectorId *aRequestId);

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

    virtual CAWResult Connect(const CAWInetAddr &peerAddr, CAWInetAddr *aAddrLocal = NULL);

    virtual CAWResult SendMessage(CAWMessageBlock &aData);
    virtual CAWResult SendMessage(const char *data, size_t datasize);

    virtual size_t HandleMessage(const char *data, size_t datasize) = 0;
    virtual void OnPeerDisconnect(CAWResult aReason) = 0;
    virtual void OnConnected(CAWResult aReason) = 0;
    virtual void OnRcvBufferOverFlow(WORD32 buffersize) = 0;

    bool IsClient(){return m_bIsClient;}

    virtual CAWResult SetTransportHandle(CAWAutoPtr<IFSTransport> &pTransport);

    virtual CAWResult Close(CAWResult reason);

    void SetSendBufferSize(WORD32 bufferSize);
    void SetRcvBufferSize(WORD32 bufferSize);
    
    inline CAWInetAddr GetPeerAddr() const{return m_addrPeer;}
    inline CAWInetAddr GetLocalAddr() const{return m_addrLocal;}
    inline bool IsSendOverBuffer() const{return m_bisSendoverbuffer;}
    inline bool IsRcvOverBuffer() const{return m_bisRcvoverbuffer;}

protected:
    CAWAutoPtr<IFSTransport> m_pTransport;
    CAWAutoPtr<IFSConnector> m_pConnector;
    CAWMessageBlock *           m_pMbSendBuf;
    CAWMessageBlock *           m_pMbRcvBuf;
    bool                        m_bConnected;
    bool                        m_bIsClient;
    WORD32                      m_dwSendBufMaxLen;
    WORD32                      m_dwRcvBufMaxLen;
    CAWInetAddr                  m_addrPeer;
    CAWInetAddr                  m_addrLocal;
    bool                        m_bisRcvoverbuffer;
    bool                        m_bisSendoverbuffer;
};
}//namespace fsutils

#endif /* CFSStreamBase_H */

