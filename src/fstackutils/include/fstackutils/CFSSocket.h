/***********************************************************************
 * Copyright (C) 2016-2018, Nanjing StarOS Technology Co., Ltd 
**********************************************************************/
#ifndef CFSSOCKET_H
#define CFSSOCKET_H

#include "starbase/CAWDefines.h"
#include "starbase/CAWError.h"
#include "starbase/CAWInetAddr.h"
#include "staros/staros.h"


using namespace starbase;
namespace fsutils
{
class  CAW_OS_EXPORT CFSIPCBase
{
public:
    enum { NON_BLOCK };

    CFSIPCBase() : m_Handle(-1) { }

    int GetHandle() const;
    void SetHandle(int aNew);

    int Enable(int aValue) const ;
    int Disable(int aValue) const ;
    int Control(int aCmd, void *aArg) const;

protected:
    int m_Handle;
};


class  CAW_OS_EXPORT CFSSocketBase : public CFSIPCBase
{
protected:
    CFSSocketBase();
    ~CFSSocketBase();

public:
    /// Wrapper around the BSD-style <socket> system call (no QoS).
    int Open(int aFamily, int aType, int aProtocol, BOOL aReuseAddr);

    /// Close down the socket handle.
    int Close();

    /// Wrapper around the <setsockopt> system call.
    int SetOption(int aLevel, int aOption, const void *aOptval, int aOptlen) const ;

    /// Wrapper around the <getsockopt> system call.
    int GetOption(int aLevel, int aOption, void *aOptval, int *aOptlen) const ;

    /// Return the address of the remotely connected peer (if there is
    /// one), in the referenced <aAddr>.
    int GetRemoteAddr(CAWInetAddr &aAddr) const;

    /// Return the local endpoint address in the referenced <aAddr>.
    int GetLocalAddr(CAWInetAddr &aAddr) const;

    /// Recv an <aLen> byte buffer from the connected socket.
    int Recv(char *aBuf, DWORD aLen, int aFlag = 0) const ;

    /// Recv an <aIov> of size <aCount> from the connected socket.
    int RecvV(iovec aIov[], DWORD aCount) const ;

    /// Send an <aLen> byte buffer to the connected socket.
    int Send(const char *aBuf, DWORD aLen, int aFlag = 0) const ;

    /// Send an <aIov> of size <aCount> from the connected socket.
    int SendV(const iovec aIov[], DWORD aCount) const ;
};


class  CAW_OS_EXPORT CFSSocketTcp : public CFSSocketBase
{
public:
    CFSSocketTcp();
    ~CFSSocketTcp();

    int Open(BOOL aReuseAddr = FALSE);
    int Open(BOOL aReuseAddr, const CAWInetAddr &aLocal);
    int Close(CAWResult aReason = CAW_OK);
    int CloseWriter();
    int CloseReader();
};

class  CAW_OS_EXPORT CFSSocketUdp : public CFSSocketBase
{
public:
    CFSSocketUdp();
    ~CFSSocketUdp();

    int Open(const CAWInetAddr &aLocal);

    int RecvFrom(char *aBuf, 
                DWORD aLen, 
                CAWInetAddr &aAddr, 
                int aFlag = 0) const ;

    int SendTo(const char *aBuf, 
                DWORD aLen, 
                const CAWInetAddr &aAddr, 
                int aFlag = 0) const ;

    int SendVTo(const iovec aIov[], 
                DWORD aCount,
                const CAWInetAddr &aAddr) const ;
};


// inline functions
inline int CFSIPCBase::GetHandle() const 
{
    return m_Handle;
}

inline void CFSIPCBase::SetHandle(int aNew)
{
    CAW_ASSERTE(m_Handle == -1 || aNew == -1);
    m_Handle = aNew;
}

inline CFSSocketBase::CFSSocketBase()
{
}

inline CFSSocketBase::~CFSSocketBase()
{
    Close();
}

inline int CFSSocketBase::SetOption(int aLevel, int aOption, const void *aOptval, int aOptlen) const 
{
    //	CFS_ASSERTE(m_Handle != CFS_INVALID_HANDLE);
    int nRet = vos_sock_setsockopt((CAW_SOCKET)m_Handle, aLevel, aOption, 
    aOptval,
    aOptlen);

    return nRet;
}

inline int CFSSocketBase::GetOption(int aLevel, int aOption, void *aOptval, int *aOptlen) const 
{
    //	CFS_ASSERTE(m_Handle != CFS_INVALID_HANDLE);
    int nRet = vos_sock_getsockopt((CAW_SOCKET)m_Handle, aLevel, aOption,
    	aOptval,
    	reinterpret_cast<socklen_t*>(aOptlen)
    	);
    return nRet;
}

inline int CFSSocketBase::Recv(char *aBuf, DWORD aLen, int aFlag) const
{
    //	CFS_ASSERTE(m_Handle != CFS_INVALID_HANDLE);
    CAW_ASSERTE(aBuf);

    int nRet = vos_sock_recv((CAW_SOCKET)m_Handle, aBuf, aLen, aFlag);
    if (nRet == -1 && errno == EAGAIN)
    errno = EWOULDBLOCK;

    return nRet;
}

inline int CFSSocketBase::RecvV(iovec aIov[], DWORD aCount) const 
{
    int nRet;
    //	CFS_ASSERTE(m_Handle != CFS_INVALID_HANDLE);
    CAW_ASSERTE(aIov);

    nRet = vos_sock_readv(m_Handle, aIov, aCount);
    return nRet;
}

inline int CFSSocketBase::Send (const char *aBuf, DWORD aLen, int aFlag) const 
{
    //	CFS_ASSERTE(m_Handle != CFS_INVALID_HANDLE);
    CAW_ASSERTE(aBuf);

    int nRet = vos_sock_send((CAW_SOCKET)m_Handle, aBuf, aLen, aFlag);
    if (nRet == -1 && errno == EAGAIN)
    	errno = EWOULDBLOCK;
	return nRet;
}

inline int CFSSocketBase::SendV(const iovec aIov[], DWORD aCount) const 
{
    int nRet;
    //	CFS_ASSERTE(m_Handle != CFS_INVALID_HANDLE);
    CAW_ASSERTE(aIov);
    nRet = vos_sock_writev(m_Handle, aIov, aCount);
    return nRet;
}

inline CFSSocketTcp::CFSSocketTcp()
{
}

inline CFSSocketTcp::~CFSSocketTcp()
{
	Close();
}

inline int CFSSocketTcp::Open(BOOL aReuseAddr)
{
    return CFSSocketBase::Open(AF_INET, SOCK_STREAM, 0, aReuseAddr);
}

inline int CFSSocketTcp::Close(CAWResult aReason)
{
    return CFSSocketBase::Close();
}

inline int CFSSocketTcp::CloseWriter()
{
    //	CFS_ASSERTE(m_Handle != CFS_INVALID_HANDLE);
    int nRet = vos_sock_shutdown((CAW_SOCKET)m_Handle, CAW_SD_SEND);

    return nRet;
}

inline int CFSSocketTcp::CloseReader()
{
    //	CFS_ASSERTE(m_Handle != CFS_INVALID_HANDLE);
    int nRet = vos_sock_shutdown((CAW_SOCKET)m_Handle, CAW_SD_RECEIVE);
    return nRet;
}

inline CFSSocketUdp::CFSSocketUdp()
{
}

inline CFSSocketUdp::~CFSSocketUdp()
{
    Close();
}

inline int CFSSocketUdp::
RecvFrom(char *aBuf, DWORD aLen, CAWInetAddr &aAddr, int aFlag) const 
{
//	CFS_ASSERTE(m_Handle != CFS_INVALID_HANDLE);

	int nSize = (int)aAddr.GetSize();
	int nRet = vos_sock_recvfrom((CAW_SOCKET)m_Handle,
						  aBuf,
						  aLen,
						  aFlag,
						  reinterpret_cast<vos_sock_sockaddr *>(aAddr.GetPtr()),
						  reinterpret_cast<socklen_t*>(&nSize)
						   );
	return nRet;
}

inline int CFSSocketUdp::
SendTo(const char *aBuf, DWORD aLen, const CAWInetAddr &aAddr, int aFlag) const 
{
//	CFS_ASSERTE(m_Handle != CFS_INVALID_HANDLE);

	int nRet = vos_sock_sendto((CAW_SOCKET)m_Handle,
						  aBuf,
						  aLen,
						  aFlag,
						  reinterpret_cast<const vos_sock_sockaddr *>(aAddr.GetPtr()),
						  static_cast<socklen_t>(aAddr.GetSize())
						  );
	
	return nRet;
}

inline int CFSSocketUdp::
SendVTo(const iovec aIov[], DWORD aCount, const CAWInetAddr &aAddr) const 
{
    int nRet;
    //	CFS_ASSERTE(m_Handle != CFS_INVALID_HANDLE);
    CAW_ASSERTE(aIov);

    return nRet;
}
}//namespace fsutils

#endif // !CMSOCKET_H
