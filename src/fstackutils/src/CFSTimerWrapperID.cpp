/***********************************************************************
 * Copyright (C) 2016-2018, Nanjing StarOS Technology Co., Ltd 
**********************************************************************/


#include "fstackutils/CFSInterface.h"
#include "fstackutils/CFSTimerWrapperID.h"


namespace fsutils
{
CFSTimerWrapperID::CFSTimerWrapperID()
    : m_bScheduled(FALSE)
    , m_pTimerQueue(NULL)
{

}

CFSTimerWrapperID::~CFSTimerWrapperID()
{
    Cancel();
}

CAWResult CFSTimerWrapperID::
Schedule(IFSTimerWrapperIDSink *aSink, const CAWTimeValue &aInterval, uint32_t aCount)
{
    CAWResult rv = CAW_ERROR_NULL_POINTER;
    CAW_ASSERTE(aSink);

    if (!m_pTimerQueue) {
        CAWThread* pThread = IFSConnectionManager::Instance()->GetNetworkThread();
        if (pThread)
        {
            m_pTimerQueue = pThread->GetTimerQueue();
        }

        if (!m_pTimerQueue) {
            CAW_ERROR_TRACE("CAWTimerWrapperID::Schedule, this thread doesn't suppport TimerQueue!");
            return rv;
        }
    }

    m_bScheduled = TRUE;
    rv = m_pTimerQueue->ScheduleTimer(this, aSink, aInterval, aCount);
    return rv;
}

CAWResult CFSTimerWrapperID::Cancel()
{
    if (!m_bScheduled)
        return 0;
    m_bScheduled = FALSE;

    CAWResult rv = CAW_ERROR_NULL_POINTER;
    if (m_pTimerQueue)
        rv = m_pTimerQueue->CancelTimer(this);
    return rv;
    }

void CFSTimerWrapperID::OnTimeout(const CAWTimeValue &, void * aArg)
{
    CAW_ASSERTE(m_bScheduled);

    IFSTimerWrapperIDSink *pSink = static_cast<IFSTimerWrapperIDSink *>(aArg);
    CAW_ASSERTE(pSink);
    if (pSink)
    {
        pSink->OnTimer(this);
    }
}

}//namespace fsutils
