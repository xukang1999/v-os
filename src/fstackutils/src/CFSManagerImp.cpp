/***********************************************************************
 * Copyright (C) 2016-2018, Nanjing StarOS Technology Co., Ltd 
**********************************************************************/
#include "CFSManagerImp.h"
#include "CFSConnectorWrapper.h"
#include "CFSAcceptorTcp.h"
#include "CFSAcceptorUdp.h"
#include "CFSConnectorThreadProxy.h"
#include "CFSAcceptorThreadProxy.h"
#include "CFSReactorEpoll.h"
#include "CFSThreadReactor.h"
namespace fsutils
{
CFSManagerImp CFSManagerImp::s_ConnectionManagerSingleton;
void kthread_func(void* arg)
{
    CFSManagerImp* pimp = (CFSManagerImp*)arg;
    if (pimp)
    {
        pimp->RunNetworkThread();
    }
}
CFSManagerImp::CFSManagerImp()
    : m_pThreadNetwork(NULL)
{
}

CFSManagerImp::~CFSManagerImp()
{
}

IFSConnectionManager* IFSConnectionManager::Instance()
{
    if (!CFSManagerImp::s_ConnectionManagerSingleton.m_pThreadNetwork) {
        CAWResult rv = CFSManagerImp::s_ConnectionManagerSingleton.SpawnNetworkThread_i();
        if (CAW_FAILED(rv)) {
            CAW_ERROR_TRACE("CFSManagerImp::Instance, SpawnNetworkThread_i() failed!"
            " rv=" << rv);
            return NULL;
        }
    }
    return &CFSManagerImp::s_ConnectionManagerSingleton;
}
void CFSManagerImp::RunNetworkThread()
{
    if (m_pThreadNetwork)
    {
        m_pThreadNetwork->SetThreadId(VOS_GetCurThreadID());
        CAWThreadManager::Instance()->RegisterThread(m_pThreadNetwork);
        m_pThreadNetwork->OnThreadInit();
        m_pThreadNetwork->OnThreadRun();
    }
}
void CFSManagerImp::CleanupInstance()
{
    s_ConnectionManagerSingleton.m_pThreadNetwork = NULL;
}


CAWResult CFSManagerImp::CreateConnectionClient_i(CType aType, IFSConnector *&aConClient, IPSReactor *pNetworkThread)
{
    CAWResult rv = CAW_ERROR_FAILURE;
    CAW_ASSERTE(!aConClient);
    aConClient = NULL;

    CType typeConnection = aType;

    switch(typeConnection) {
    case CTYPE_TCP: 
    case CTYPE_UDP:
    {
        CFSConnectorWrapper *pCw = new CFSConnectorWrapper(pNetworkThread);
        if (pCw) {
            rv = pCw->Init(typeConnection);
            if (CAW_SUCCEEDED(rv))
            {
                aConClient = pCw;
            }
            else 
            {
                delete pCw;
                return CAW_ERROR_FAILURE;
            }
        }
        else {
            rv = CAW_ERROR_OUT_OF_MEMORY;
            aConClient= NULL;
        }
        break;
    }

    /////////////////////Connection Service Add End//////////////////////
    default:
        CAW_ERROR_TRACE("CFSManagerImp::CreateConnectionClient_i, wrong type=" << aType);
        rv = CAW_ERROR_INVALID_ARG;
        aConClient= NULL;
        break;
    }

    if (aConClient)
        aConClient->AddReference();
    return rv;
}


CAWResult CFSManagerImp::
CreateConnectionServer_i(CType aType, IFSAcceptor *&aAcceptor, IPSReactor *pNetworkThread)
{
    CAWResult rv = CAW_ERROR_FAILURE;

    switch(aType) {
    case CTYPE_TCP: 
        aAcceptor = new CFSAcceptorTcp(pNetworkThread);
        if (aAcceptor)
            rv = CAW_OK;
        break;

    case CTYPE_UDP:
        aAcceptor = new CFSAcceptorUdp(pNetworkThread);
        if (aAcceptor)
            rv = CAW_OK;
        break;

    /////////////////////Connection Service Add End//////////////////////
    default:
        CAW_ERROR_TRACE("CFSManagerImp::CreateConnectionServer, wrong type=" << aType);
        rv = CAW_ERROR_INVALID_ARG;
        break;
    }

    if (aAcceptor)
        aAcceptor->AddReference();
    return rv;
}


CAWResult CFSManagerImp::SpawnNetworkThread_i()
{
    CAWResult rv=CAW_OK;
    CAW_INFO_TRACE("CFSManagerImp::InitMainThread,create MainThread");
    m_pThreadNetwork = new CFSThreadReactor();
    IPSReactor * aReactor=new CFSReactorEpoll();

    CAWInetAddr loaddr("127.0.0.1", 0);
    CAWInetAddr loaddrnetmask("255.0.0.0", 0);
    CAWInetAddr loaddrbroadcast("127.255.255.255", 0);
    in_addr_t ipaddr = (in_addr_t)loaddr.GetIpAddrIn4Bytes();
    in_addr_t netmask = (in_addr_t)loaddrnetmask.GetIpAddrIn4Bytes();
    in_addr_t broadcast= (in_addr_t)loaddrbroadcast.GetIpAddrIn4Bytes();
    int result = vos_veth_set_addr("lo0", &ipaddr, &netmask, &broadcast);
    printf("SpawnNetworkThread_i result=%d\n", result);
#if 0
    in_addr_t gw=0;
    char macaddr[VOS_MAC_LEN] = { 0 };

    struct vos_veth* plo= create_vos_veth(&ipaddr,
        &netmask,
        &broadcast,
        &gw,
        macaddr,
        NULL,NULL);
#endif
    m_pThreadNetwork->Init(aReactor);
    kthread_create(kthread_func, this,"fstackthread");
    
    return CAW_OK;


}

CAWResult CFSManagerImp::
CreateConnectionClient(CType aType, IFSConnector *&aConClient)
{
    if (m_pThreadNetwork==NULL)
    {
        CAW_ASSERTE(0);
    }

    if (CAWThreadManager::IsEqualCurrentThread(m_pThreadNetwork->GetThreadId()))
    {
        // the current thread is the network thread,
        // and will not invoke functions in the mutil-thread.
        return CreateConnectionClient_i(aType, aConClient, m_pThreadNetwork->GetPSReactor());
    }
    else {
        CAW_INFO_TRACE("CFSManagerImp::CreateConnectionClient, create CAWConnectorThreadProxy.");
        CAWThread *pCurrent = CAWThreadManager::Instance()->GetThread(CAWThreadManager::TT_CURRENT);
        if ((pCurrent==NULL) || (pCurrent->GetEventQueue()==NULL))
        {
            return CAW_ERROR_FAILURE;
        }

        CFSConnectorThreadProxy *pConProxy = new CFSConnectorThreadProxy(aType, m_pThreadNetwork->GetPSReactor(), pCurrent);
        if (!pConProxy)
            return CAW_ERROR_OUT_OF_MEMORY;

        aConClient = pConProxy;
        aConClient->AddReference();
        return CAW_OK;
    }

}

CAWThread *CFSManagerImp::GetNetworkThread()
{
    return m_pThreadNetwork;
}

CAWResult CFSManagerImp::
CreateConnectionServer(CType aType, IFSAcceptor *&aAcceptor)
{
    CAW_ASSERTE(!aAcceptor);
    if (m_pThreadNetwork==NULL)
    {
        CAW_ASSERTE(0);
    }

    //CAWThread *pNetwork = CAWThreadManager::Instance()->GetThread(CAWThreadManager::TT_IPC);

    if (CAWThreadManager::IsEqualCurrentThread(m_pThreadNetwork->GetThreadId()))
    {
        //printf("CreateConnectionServer no proxy\n");
        return CreateConnectionServer_i(aType, aAcceptor, m_pThreadNetwork->GetPSReactor());
    }
    else
    {
        CAWThread *pCurrent = CAWThreadManager::Instance()->GetThread(CAWThreadManager::TT_CURRENT);
        if ((pCurrent==NULL) || (pCurrent->GetEventQueue()==NULL))
        {
            return CAW_ERROR_FAILURE;
        }
        CAW_INFO_TRACE("CFSManagerImp::CreateConnectionServer, create CAWAcceptorThreadProxy.");
        CFSAcceptorThreadProxy *pAcceptorProxy = new CFSAcceptorThreadProxy(aType, m_pThreadNetwork->GetPSReactor(), pCurrent);
        if (!pAcceptorProxy)
            return CAW_ERROR_OUT_OF_MEMORY;

        aAcceptor = pAcceptorProxy;
        aAcceptor->AddReference();
        return CAW_OK;
    }

}
}//namespace fsutils
