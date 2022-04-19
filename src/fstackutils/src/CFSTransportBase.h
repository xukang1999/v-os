/***********************************************************************
 * Copyright (C) 2016-2018, Nanjing StarOS Technology Co., Ltd 
**********************************************************************/
#ifndef CFSTRANSPORTBASE_H
#define CFSTRANSPORTBASE_H
#include "fstackutils/CFSInterface.h"
#include "ipstacktp/CPSReactorInterface.h"
using namespace ipstacktp;
namespace fsutils
{
class CFSSocketBase;

class CFSTransportBase 
    : public IPSEventHandler
    , public IFSTransport
    , public CAWReferenceControlSingleThreadTimerDelete
{
public:
    CFSTransportBase(IPSReactor *pReactor);
    virtual ~CFSTransportBase();

    // interface IAWReferenceControl
    virtual DWORD AddReference();
    virtual DWORD ReleaseReference();

    // interface IAWTransport
    virtual CAWResult OpenWithSink(IFSTransportSink *aSink);
    virtual IFSTransportSink* GetSink();
    virtual CAWResult Disconnect(CAWResult aReason);

    // interface IAWEventHandler
    virtual int OnClose(int aFd, MASK aMask);

protected:
    // template method for open() and close()
    virtual CAWResult Open_t() = 0;
    virtual CAWResult Close_t(CAWResult aReason) = 0;
    virtual CAWResult Disconnect_t(CAWResult aReason) = 0;
    IFSTransportSink *m_pSink;
    IPSReactor *m_pReactor;
};
}//namespace fsutils
#endif // !CFSTRANSPORTBASE_H
