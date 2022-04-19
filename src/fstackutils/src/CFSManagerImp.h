/***********************************************************************
 * Copyright (C) 2016-2018, Nanjing StarOS Technology Co., Ltd 
**********************************************************************/

#ifndef CFSMANAGERIMP_H
#define CFSMANAGERIMP_H

#include "fstackutils/CFSInterface.h"
namespace fsutils
{
class CFSThreadReactor;
class CAW_OS_EXPORT CFSManagerImp : public IFSConnectionManager
{
public:
    CFSManagerImp();
    virtual ~CFSManagerImp();
    virtual CAWResult CreateConnectionClient(CType aType, IFSConnector *&aConClient);
    virtual CAWResult CreateConnectionServer(CType aType, IFSAcceptor *&aAcceptor);
    virtual CAWThread *GetNetworkThread();

    void RunNetworkThread();
public:
    static void CleanupInstance();

    CAWResult SpawnNetworkThread_i();
    CAWResult CreateConnectionClient_i(CType aType, IFSConnector *&aConClient, IPSReactor *pNetworkThread);
    CAWResult CreateConnectionServer_i(CType aType, IFSAcceptor *&aAcceptor, IPSReactor *pNetworkThread);

    static CFSManagerImp s_ConnectionManagerSingleton;
    CFSThreadReactor *m_pThreadNetwork;
};
}//namespace fsutils
#endif // CFSMANAGERIMP_H

