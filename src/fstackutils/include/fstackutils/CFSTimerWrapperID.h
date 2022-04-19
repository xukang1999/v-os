/***********************************************************************
 * Copyright (C) 2016-2018, Nanjing StarOS Technology Co., Ltd 
**********************************************************************/

#ifndef IFSTIMERWRAPPERID_H
#define IFSTIMERWRAPPERID_H
#include "starbase/CAWDebug.h"
#include "fstackutils/CFSInterface.h"

using namespace starbase;
namespace fsutils
{
class CFSTimerWrapperID;

class CAW_OS_EXPORT IFSTimerWrapperIDSink
{
public:
    virtual void OnTimer(CFSTimerWrapperID* aId) = 0;
protected:
    virtual ~IFSTimerWrapperIDSink(){}
};

class CAW_OS_EXPORT CFSTimerWrapperID : public IAWTimerHandler
{
public:
    CFSTimerWrapperID();
    virtual ~CFSTimerWrapperID();

    /// Schedule an timer that will expire after <aInterval> for <aCount> times. 
    /// if <aCount> is 0, schedule infinite times.
    CAWResult Schedule(
        IFSTimerWrapperIDSink* aSink,
        const CAWTimeValue &aInterval,
        uint32_t aCount = 0);

    /// Cancel the timer.
    CAWResult Cancel();

protected:
    virtual void OnTimeout(const CAWTimeValue &aCurTime, void* aArg);

private:
    bool m_bScheduled;
    IAWTimerQueue *m_pTimerQueue;
};
}//namespace fsutils
#endif // !CAWTIMERWRAPPERID_H

