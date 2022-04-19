/***********************************************************************
 * Copyright (C) 2016-2018, Nanjing StarOS Technology Co., Ltd 
**********************************************************************/

#ifndef CFSACCEPTORBASE_H
#define CFSACCEPTORBASE_H

#include "fstackutils/CFSInterface.h"
#include "ipstacktp/CPSReactorInterface.h"
using namespace ipstacktp;
namespace fsutils
{
class  CFSAcceptorBase : public IFSAcceptor, public CAWReferenceControlSingleThread 
{
public:
    CFSAcceptorBase(IPSReactor *pThreadNetwork);
    virtual ~CFSAcceptorBase();

    // interface IAWReferenceControl
    virtual DWORD AddReference();
    virtual DWORD ReleaseReference();

    // interface IAWAcceptorConnectorId
    virtual BOOL IsConnector();

protected:
    IPSReactor *m_pReactor;
    IFSAcceptorConnectorSink *m_pSink;
};
}//namespace fsutils
#endif // !CFSACCEPTORBASE_H

