#include "fstackutils/CFSInterface.h"
#include "CFSPipe.h"
#include "staros/staros.h"
namespace fsutils
{
CFSPipe::CFSPipe()
{
    m_Handles[0] = -1;
    m_Handles[1] = -1;
}

CFSPipe::~CFSPipe()
{
    Close();
}

CAWResult CFSPipe::Open()
{
    CAW_ASSERTE(m_Handles[0] == -1 && m_Handles[1] == -1);

    int nRet = 0;
    nRet = vos_sock_socketpair(AF_UNIX, SOCK_STREAM, 0, m_Handles);
    if (nRet == -1) {
        CAW_ERROR_TRACE_THIS("CAWPipe::Open, socketpair() failed! err=" << errno);
        return CAW_ERROR_FAILURE;
    }
    printf("socket1=%d,socket2=%d\n", m_Handles[0], m_Handles[1]);
#if 0
    int on = 1;
    struct sockaddr_in client_addr;

    CAWInetAddr addrListen("127.0.0.1", 5555);
    CAWInetAddr addrConnect;
    CAWInetAddr addrPeer;
    socklen_t nAddrLen = addrPeer.GetSize();
    int fd = vos_sock_socket(AF_INET, SOCK_STREAM, 0);

    if (fd < 0) {
        CAW_ERROR_TRACE_THIS("CFSPipe::Open, open() failed! err="<<errno);
        goto fail;
    }

    nRet = vos_sock_bind(fd,
        reinterpret_cast<const struct vos_sock_sockaddr *>(addrListen.GetPtr()),
        addrListen.GetSize());
    if (nRet < 0) {
        CAW_ERROR_TRACE_THIS("CFSPipe::Open, bind() failed! err="<<errno);
        goto fail;
    }

    if (vos_sock_listen(fd, 5) == -1) {
        CAW_ERROR_TRACE_THIS("CFSPipe::Open, listen() failed! err="<<errno);
        goto fail;
    }

    m_Handles[1] = vos_sock_socket(AF_INET, SOCK_STREAM, 0);
    CAW_INFO_TRACE_THIS("CFSPipe::Open, fs_socket m_Handles[1]="<<m_Handles[1]);
    if (m_Handles[1]<0)
    {
        CAW_ERROR_TRACE_THIS("CFSPipe::Open, fs_socket failure");
    }
    //CAW_ASSERTE(m_Handles[1]<0);

    /*vos_sock_ioctl(m_Handles[1], VOS_FIONBIO, &on);*/


    memset(&client_addr, 0, sizeof(client_addr));
    int size = sizeof(client_addr);
    int rc = vos_sock_getsockname(fd, (struct vos_sock_sockaddr*)&client_addr, &size);

    printf("client_addr port=%d\n", ntohs(client_addr.sin_port));

    nRet = vos_sock_connect(m_Handles[1],
        reinterpret_cast<const struct vos_sock_sockaddr *>(&client_addr),
        size);
    if (nRet == -1) {
        printf("errno=%d\n", errno);
        CAW_ERROR_TRACE_THIS("CFSPipe::Open, connect() failed! err="<<errno);
        if (errno != VOS_EINPROGRESS)
        {
            goto fail;
        }
    }
    //fs_ioctl(fd, FIONBIO, &on);

    m_Handles[0] = vos_sock_accept(fd,
        reinterpret_cast<struct vos_sock_sockaddr *>(addrPeer.GetPtr()),
        &nAddrLen);
    if (m_Handles[0] < 0) {
        CAW_ERROR_TRACE_THIS("CFSPipe::Open, accept() failed! err="<<errno);
        goto fail;
    }
#endif
    return CAW_OK;
fail:
    Close();
    return CAW_ERROR_NOT_AVAILABLE;
}

CAWResult CFSPipe::Close()
{
    int nRet = 0;
    if (m_Handles[0] != -1) {
        nRet = vos_sock_close(m_Handles[0]);
        m_Handles[0] = -1;
    }
    if (m_Handles[1] != -1) {
        nRet |= vos_sock_close(m_Handles[1]);
        m_Handles[1] = -1;
    }
    return nRet == 0 ? CAW_OK : CAW_ERROR_NETWORK_SOCKET_ERROR;
}

int CFSPipe::GetReadHandle() const
{
    return m_Handles[0];
}

int CFSPipe::GetWriteHandle() const
{
    return m_Handles[1];
}
}//namespace fsutils
