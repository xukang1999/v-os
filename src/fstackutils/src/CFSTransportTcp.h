/***********************************************************************
 * Copyright (C) 2016-2018, Nanjing StarOS Technology Co., Ltd 
**********************************************************************/

#ifndef CFSTRANSPORTTCP_H
#define CFSTRANSPORTTCP_H

#include "CFSTransportBase.h"
#include "fstackutils/CFSSocket.h"
using namespace starbase;
namespace fsutils
{
class CFSTransportTcp : public CFSTransportBase, public CAWTimerWrapperIDSink
{
public:
    CFSTransportTcp(IPSReactor *pReactor);
    virtual ~CFSTransportTcp();

    // interface IAWEventHandler
    virtual int GetHandle() const ;
    virtual int OnInput(int aFd = -1);
    virtual int OnOutput(int aFd = -1);

    // interface IAWTransport
    virtual CAWResult SendData(CAWMessageBlock &aData, CFSTransportParameter *aPara = NULL);
    virtual CAWResult SetOption(DWORD aCommand, LPVOID aArg);
    virtual CAWResult GetOption(DWORD aCommand, LPVOID aArg);
    int OnEpollInput(int afd);

    CFSSocketTcp& GetPeer();
    virtual int GetTransportHandle() const;


protected:
    virtual CAWResult Open_t();
    virtual CAWResult Close_t(CAWResult aReason);
    virtual CAWResult Disconnect_t(CAWResult aReason);

    int Recv_i(LPSTR aBuf, DWORD aLen, BOOL aNeedTimerRereceive = TRUE);
    int Send_i(LPCSTR aBuf, DWORD aLen);

    virtual void OnTimer(CAWTimerWrapperID *aId);

protected:
    CFSSocketTcp m_SocketTcp;
    CAWTimerWrapperID m_TimerRereceive;
    BOOL m_bNeedOnSend;
    CAWInetAddr m_LocalAddr;
    CAWInetAddr m_RemoteAddr;
};


// inline functions
inline int CFSTransportTcp::Recv_i(LPSTR aBuf, DWORD aLen, BOOL aNeedTimerRereceive)
{
    // the recv len must be as large as possible
    // due to avoid lossing real-time signal
    CAW_ASSERTE(aBuf && aLen > 0);
    int nRecv = m_SocketTcp.Recv(aBuf, aLen);
    if (nRecv < 0) {
        if (errno == VOS_EWOULDBLOCK) {
            int nAvailable = 0;
            if (aNeedTimerRereceive && m_SocketTcp.Control(FIONREAD, &nAvailable) != -1 && nAvailable > 0) {
                CAW_INFO_TRACE_THIS("CAWTransportTcp::Recv_i, EWOULDBLOCK, fd=" << m_SocketTcp.GetHandle() << " available=" << nAvailable);
                m_TimerRereceive.Schedule(this, CAWTimeValue(0, 0), 1);
            }
            return 0;
        }
        else {
            CAWErrnoGuard egTmp;
            CAW_WARNING_TRACE_THIS("CAWTransportTcp::Recv_i, recv() failed! err=" << errno << 
            " fd=" << m_SocketTcp.GetHandle() << 
            " addr1=" << m_LocalAddr.GetIpDisplayName() <<
            " addr2=" << m_RemoteAddr.GetIpDisplayName());
            return -1;
        }
    }
    if (nRecv == 0) {
        CAW_INFO_TRACE_THIS("CAWTransportTcp::Recv_i, 0 fd=" << m_SocketTcp.GetHandle() << 
        " addr1=" << m_LocalAddr.GetIpDisplayName() << 
        " addr2=" << m_RemoteAddr.GetIpDisplayName());
        // it is a graceful disconnect
        return -1;
    }
    return nRecv;
}

inline int CFSTransportTcp::Send_i(LPCSTR aBuf, DWORD aLen)
{
    CAW_ASSERTE(aBuf && aLen > 0);
    int nSend = m_SocketTcp.Send(aBuf, aLen);

    if (nSend < 0) {
        if (errno == VOS_EWOULDBLOCK)
            return 0;
        else {
            CAWErrnoGuard egTmp;
            CAW_WARNING_TRACE_THIS("CAWTransportTcp::Send_i, send() failed! err=" << errno << 
                "fd=" << m_SocketTcp.GetHandle());
            return -1;
        }
    }
    return nSend;
}
}//namespace fsutils
#endif // !CFSTRANSPORTTCP_H

