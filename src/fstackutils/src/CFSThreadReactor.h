/**********************************************************************************
 * Copyright (C) 2013-2015, Nanjing WFNEX Technology Co., Ltd. All rights reserved.
***********************************************************************************/
#ifndef CFSTHREADREACTOR_H
#define CFSTHREADREACTOR_H

#include "fstackutils/CFSInterface.h"
#include "ipstacktp/CPSReactorInterface.h"
using namespace ipstacktp;
namespace fsutils
{
class CFSThreadReactor : public CAWThread  
{
public:
    CFSThreadReactor();
    virtual ~CFSThreadReactor();
    CAWResult Init(IPSReactor *aReactor);
    // interface CAWThread
    virtual CAWResult Create(CAWThreadManager::TType aType, CAWThreadManager::TFlag aFlag = CAWThreadManager::TF_JOINABLE);
    virtual CAWResult Stop(CAWTimeValue* aTimeout = NULL);

    virtual void OnThreadInit();
    virtual void OnThreadRun();

    virtual IAWReactor* GetReactor();
    virtual IAWEventQueue* GetEventQueue();
    virtual IAWTimerQueue* GetTimerQueue();
    IPSReactor* GetPSReactor();
 private:
    IPSReactor *m_pReactor;
};
}//namespace fsutils
#endif //CFSTHREADREACTOR_H

