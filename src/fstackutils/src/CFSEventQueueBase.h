/***********************************************************************
 * Copyright (C) 2016-2018, Nanjing StarOS Technology Co., Ltd 
**********************************************************************/
#ifndef CFSEVENTQUEUEBASE_H
#define CFSEVENTQUEUEBASE_H

#include "fstackutils/CFSInterface.h"
namespace fsutils
{
class CFSEventQueueBase;

class CFSEventSynchronous : public IAWEvent
{
public:
    CFSEventSynchronous(IAWEvent *aEventPost, CFSEventQueueBase *aEventQueue);
    virtual ~CFSEventSynchronous();

    // interface IAWEvent
    virtual CAWResult OnEventFire();
    virtual void OnDestorySelf();

    CAWResult WaitResultAndDeleteThis();

private:
    IAWEvent *m_pEventPost;
    CAWResult m_Result;
    CFSEventQueueBase *m_pEventQueue;
    BOOL m_bHasDestoryed;
    CAWEventThread m_SendEvent;
};

class CAW_OS_EXPORT CFSEventQueueBase : public IAWEventQueue
{
public:
    CFSEventQueueBase();
    virtual ~CFSEventQueueBase();

    // interface IAWEventQueue
    virtual CAWResult PostEvent(IAWEvent *aEvent, EPriority aPri = EPRIORITY_NORMAL);
    virtual CAWResult SendEvent(IAWEvent *aEvent);
    virtual DWORD GetPendingEventsCount();

    void Stop();

    void DestoryPendingEvents();

    void Reset2CurrentThreadId();

    enum { MAX_GET_ONCE = 100000 };
    typedef std::list<IAWEvent *> EventsType;

    // Don't make the following two functions static because we want trace size.
    CAWResult ProcessEvents(const EventsType &aEvents);
    CAWResult ProcessOneEvent(IAWEvent *aEvent);

    static CAWTimeValue s_tvReportInterval;

protected:
    CAWResult PopPendingEvents(
                    EventsType &aEvents, 
                    DWORD aMaxCount = MAX_GET_ONCE, 
                    DWORD *aRemainSize = NULL);

    CAWResult PopOnePendingEvent(
                    IAWEvent *&aEvent, 
                    DWORD *aRemainSize = NULL);

    EventsType m_Events;
    // we have to record the size of events list due to limition of std::list in Linux.
    DWORD m_dwSize;
    CAWTimeValue m_tvReportSize;

private:
    CAW_THREAD_ID m_Tid;
    BOOL m_bIsStopped;

    friend class CFSEventSynchronous;
};

class CFSEventQueueUsingMutex : public CFSEventQueueBase 
{
public:
    CFSEventQueueUsingMutex();
    virtual ~CFSEventQueueUsingMutex();

    // interface IAWEventQueue
    virtual CAWResult PostEvent(IAWEvent *aEvent, EPriority aPri = EPRIORITY_NORMAL);

    // Pop <aMaxCount> pending events in the queue, 
    // if no events are pending, return at once.
    CAWResult PopPendingEventsWithoutWait(
        CFSEventQueueBase::EventsType &aEvents, 
        DWORD aMaxCount = MAX_GET_ONCE, 
        DWORD *aRemainSize = NULL);

    // Pop one pending events, and fill <aRemainSize> with remain size.
    // if no events are pending, return at once.
    CAWResult PopOnePendingEventWithoutWait(
        IAWEvent *&aEvent, 
        DWORD *aRemainSize = NULL);

    CAWResult PostEventWithOldSize(
        IAWEvent *aEvent, 
        EPriority aPri = EPRIORITY_NORMAL, 
        DWORD *aOldSize = NULL);

private:
    typedef CAWMutexThread MutexType;
    MutexType m_Mutex;
};


// inline functions
inline void CFSEventQueueBase::Reset2CurrentThreadId()
{
    m_Tid = CAWThreadManager::GetThreadSelfId();
}

inline void CFSEventQueueBase::Stop()
{
    m_bIsStopped = TRUE;
}


inline CFSEventQueueUsingMutex::CFSEventQueueUsingMutex()
{
}

inline CAWResult CFSEventQueueUsingMutex::PopPendingEventsWithoutWait(CFSEventQueueBase::EventsType &aEvents, 
    DWORD aMaxCount, DWORD *aRemainSize)
{
    CAWMutexGuardT<MutexType> theGuard(m_Mutex);
    return PopPendingEvents(aEvents, aMaxCount, aRemainSize);
}

inline CAWResult CFSEventQueueUsingMutex::PopOnePendingEventWithoutWait(IAWEvent *&aEvent, DWORD *aRemainSize)
{
    CAWMutexGuardT<MutexType> theGuard(m_Mutex);
    return PopOnePendingEvent(aEvent, aRemainSize);
}
}//namespace fsutils
#endif // !CAWEVENTQUEUEBASE_H