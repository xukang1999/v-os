/***********************************************************************
 * Copyright (C) 2016-2018, Nanjing StarOS Technology Co., Ltd 
**********************************************************************/

#include "fstackutils/CFSSocket.h"

namespace fsutils
{
int CFSIPCBase::Enable(int aValue) const 
{
//	CAW_ASSERTE(m_Handle != CAW_INVALID_HANDLE);
	switch(aValue) {
	case NON_BLOCK: 
		{
		int nVal = vos_sock_fcntl(m_Handle, VOS_F_GETFL, 0);
		if (nVal == -1)
			return -1;
		nVal |= VOS_O_NONBLOCK;
		if (vos_sock_fcntl(m_Handle, VOS_F_SETFL, nVal) == -1)
			return -1;
		return 0;
		}

	default:
		return -1;
	}
}

int CFSIPCBase::Disable(int aValue) const 
{
//	CAW_ASSERTE(m_Handle != CAW_INVALID_HANDLE);
	switch(aValue) {
	case NON_BLOCK:
		{
		int nVal = vos_sock_fcntl(m_Handle, VOS_F_GETFL, 0);
		if (nVal == -1)
			return -1;
		nVal &= ~VOS_O_NONBLOCK;
		if (vos_sock_fcntl(m_Handle, VOS_F_SETFL, nVal) == -1)
			return -1;
		return 0;
		}

	default:
		return -1;
	}
}

int CFSIPCBase::Control(int aCmd, void *aArg) const
{
	int nRet;
	nRet = vos_sock_ioctl(m_Handle, aCmd, aArg);
	return nRet;
}


//////////////////////////////////////////////////////////////////////
// class CFSSocketBase
//////////////////////////////////////////////////////////////////////

int CFSSocketBase::Open(int aFamily, int aType, int aProtocol, BOOL aReuseAddr)
{
	int nRet = -1;
	Close();
	
	m_Handle = (int)vos_sock_socket(aFamily, aType, aProtocol);
	if (m_Handle != -1) {
		nRet = 0;
		if (aFamily != PF_UNIX && aReuseAddr) {
			int nReuse = 1;
			nRet = SetOption(SOL_SOCKET, SO_REUSEADDR, &nReuse, sizeof(nReuse));
		}
	}

	if (nRet == -1) {
		Close();
	}
	return nRet;
}

int CFSSocketBase::GetRemoteAddr(CAWInetAddr &aAddr) const
{
//	CAW_ASSERTE(m_Handle != CAW_INVALID_HANDLE);

	int nSize = (int)aAddr.GetSize();
	int nGet = vos_sock_getpeername((CAW_SOCKET)m_Handle,
					reinterpret_cast<vos_sock_sockaddr *>(aAddr.GetPtr()),
					reinterpret_cast<socklen_t*>(&nSize)
					);

	return nGet;
}

int CFSSocketBase::GetLocalAddr(CAWInetAddr &aAddr) const
{
//	CAW_ASSERTE(m_Handle != CAW_INVALID_HANDLE);

	int nSize = (int)aAddr.GetSize();
	int nGet = vos_sock_getsockname((CAW_SOCKET)m_Handle,
					reinterpret_cast<vos_sock_sockaddr *>(aAddr.GetPtr()),
					reinterpret_cast<socklen_t*>(&nSize)
					);

	return nGet;
}

int CFSSocketBase::Close()
{
	int nRet = 0;
	if (m_Handle != -1) {
		nRet = vos_sock_close((int)m_Handle);
		m_Handle = -1;
	}
	return nRet;
}

//////////////////////////////////////////////////////////////////////
// class CFSSocketTcp
//////////////////////////////////////////////////////////////////////

int CFSSocketTcp::Open(BOOL aReuseAddr, const CAWInetAddr &aLocal)
{
	if (CFSSocketBase::Open(AF_INET, SOCK_STREAM, 0, aReuseAddr) == -1)
		return -1;

	if (vos_sock_bind((CAW_SOCKET)m_Handle, 
						  reinterpret_cast<const vos_sock_sockaddr *>(aLocal.GetPtr()),
						  static_cast<socklen_t>(aLocal.GetSize())
						  ) == -1)
	{
		Close();
		return -1;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////
// class CFSSocketUdp
//////////////////////////////////////////////////////////////////////

int CFSSocketUdp::Open(const CAWInetAddr &aLocal)
{
    if (CFSSocketBase::Open(AF_INET, SOCK_DGRAM, 0, FALSE) == -1)
    return -1;

    if (vos_sock_bind((CAW_SOCKET)m_Handle, 
    reinterpret_cast<const vos_sock_sockaddr *>(aLocal.GetPtr()),
    static_cast<socklen_t>(aLocal.GetSize())
    ) == -1)
    {
    Close();
    return -1;
    }
    return 0;
}
}//namespace fsutils