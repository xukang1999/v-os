/***********************************************************************
 * Copyright (C) 2016-2018, Nanjing StarOS Technology Co., Ltd 
**********************************************************************/

#include "fstackutils/CFSInterface.h"
#include "CFSEventQueueBase.h"

namespace fsutils
{
CFSEventSynchronous::CFSEventSynchronous(IAWEvent *aEventPost, CFSEventQueueBase *aEventQueue)
    : m_pEventPost(aEventPost)
    , m_Result(CAW_ERROR_NOT_AVAILABLE)
    , m_pEventQueue(aEventQueue)
    , m_bHasDestoryed(FALSE)
{
    CAW_ASSERTE(m_pEventPost);
    CAW_ASSERTE(m_pEventQueue);
    CAW_INFO_TRACE_THIS("CFSEventSynchronous::CFSEventSynchronous");
}

CFSEventSynchronous::~CFSEventSynchronous()
{
    CAW_INFO_TRACE_THIS("CFSEventSynchronous::~CFSEventSynchronous");

    if (m_pEventPost)
        m_pEventPost->OnDestorySelf();
    if (!m_bHasDestoryed)
        m_SendEvent.Signal();
}

CAWResult CFSEventSynchronous::OnEventFire()
{
    CAW_INFO_TRACE_THIS("CFSEventSynchronous::OnEventFire, m_bHasDestoryed="<<m_bHasDestoryed
        <<"m_pEventPost="<<m_pEventPost);

    CAWResult rv = CAW_OK;
    if (m_bHasDestoryed)
    {
        return rv;
    }

    if (m_pEventPost)
    {
        CAW_INFO_TRACE("CFSEventSynchronous::OnEventFire, eventpost="<<m_pEventPost);
        rv = m_pEventPost->OnEventFire();
    }
    else
    {
        CAW_ERROR_TRACE("CFSEventSynchronous::OnEventFire");
        rv = CAW_ERROR_NULL_POINTER;
    }
    CAW_INFO_TRACE_THIS("CFSEventSynchronous::OnEventFire, end");

    m_Result = rv;
    m_SendEvent.Signal();
    return rv;
}

void CFSEventSynchronous::OnDestorySelf()
{
    CAW_INFO_TRACE_THIS("CFSEventSynchronous::OnDestorySelf");

    if (m_bHasDestoryed) {
    // delete this in the cecond time of OnDestorySelf.
        delete this;
    }
    else {
    // Don't assign <m_bHasDestoryed> in the function WaitResultAndDeleteThis().
    // Do operations on <m_bHasDestoryed> in the same thread.
        m_bHasDestoryed = TRUE;
        if (m_pEventPost) {
            m_pEventPost->OnDestorySelf();
            m_pEventPost = NULL;
        }
    }
}

CAWResult CFSEventSynchronous::WaitResultAndDeleteThis()
{
    CAW_INFO_TRACE_THIS("CFSEventSynchronous::WaitResultAndDeleteThis");
    CAWResult rv = m_SendEvent.Wait();
    if (CAW_FAILED(rv)) {
        CAW_ERROR_TRACE("CFSEventSynchronous::WaitResultAndDeleteThis,"
        " m_SendEvent.Wait() failed!");
        return rv;
    }

    rv = m_Result;
    if (m_pEventQueue)
        m_pEventQueue->PostEvent(this);
    return rv;
}


//////////////////////////////////////////////////////////////////////
// class CFSEventQueueBase
//////////////////////////////////////////////////////////////////////


CAWTimeValue CFSEventQueueBase::s_tvReportInterval(0, 50*1000);

CFSEventQueueBase::CFSEventQueueBase()
    : m_dwSize(0)
    , m_bIsStopped(FALSE)
{
    m_tvReportSize = CAWTimeValue::GetTimeOfDay();
}

CFSEventQueueBase::~CFSEventQueueBase()
{
    DestoryPendingEvents();
}

void CFSEventQueueBase::DestoryPendingEvents()
{
    EventsType::iterator iter = m_Events.begin();
    for ( ; iter != m_Events.end(); ++iter)
        (*iter)->OnDestorySelf();
    m_Events.clear();
}

CAWResult CFSEventQueueBase::PostEvent(IAWEvent *aEvent, EPriority aPri)
{
    CAW_UNUSED_ARG(aPri);
    CAW_ASSERTE_RETURN(aEvent, CAW_ERROR_INVALID_ARG);

    if (m_bIsStopped) {
        CAW_ERROR_TRACE("CFSEventQueueBase::PostEvent, has been stopped.");
        aEvent->OnDestorySelf();
        return CAW_ERROR_NOT_INITIALIZED;
    }

    m_Events.push_back(aEvent);
    m_dwSize++;

    return CAW_OK;
}

CAWResult CFSEventQueueBase::SendEvent(IAWEvent *aEvent)
{
    CAW_ASSERTE_RETURN(aEvent, CAW_ERROR_INVALID_ARG);

    if (m_bIsStopped) {
        CAW_ERROR_TRACE("CFSEventQueueBase::SendEvent, has been stopped.");
        aEvent->OnDestorySelf();
        return CAW_ERROR_NOT_INITIALIZED;
    }

    // if send event to the current thread, just do callbacks.
    if (CAWThreadManager::IsEqualCurrentThread(m_Tid)) {
        CAWResult rv = aEvent->OnEventFire();
        aEvent->OnDestorySelf();
        return rv;
    }

    CFSEventSynchronous *pSend = new CFSEventSynchronous(aEvent, this);
    CAWResult rv = PostEvent(pSend);
    if (CAW_FAILED(rv))
        return rv;

    rv = pSend->WaitResultAndDeleteThis();
    return rv;
}

DWORD CFSEventQueueBase::GetPendingEventsCount()
{
    return m_dwSize;
}

CAWResult CFSEventQueueBase::PopPendingEvents(EventsType &aEvents, DWORD aMaxCount, DWORD *aRemainSize)
{
    CAW_ASSERTE(aEvents.empty());
    CAW_ASSERTE(aMaxCount > 0);

    DWORD dwTotal = m_dwSize;
    if (dwTotal == 0)
        return CAW_ERROR_NOT_FOUND;

    if (dwTotal <= aMaxCount) {
        aEvents.swap(m_Events);
        m_dwSize = 0;
        CAW_ASSERTE(m_Events.empty());
    }
    else {
        for (DWORD i = 0; i < aMaxCount; i++) {
            aEvents.push_back(m_Events.front());
            m_Events.pop_front();
            m_dwSize--;
        }
        CAW_ASSERTE(!m_Events.empty());
    }

    if (aRemainSize)
        *aRemainSize = m_dwSize;
    return CAW_OK;
}

CAWResult CFSEventQueueBase::PopOnePendingEvent(IAWEvent *&aEvent, DWORD *aRemainSize)
{
    CAW_ASSERTE(!aEvent);

    if (m_dwSize == 0)
        return CAW_ERROR_NOT_FOUND;

    aEvent = m_Events.front();
    m_Events.pop_front();
    m_dwSize--;

    if (aRemainSize)
        *aRemainSize = m_dwSize;
    return CAW_OK;
}

CAWResult CFSEventQueueBase::ProcessEvents(const EventsType &aEvents)
{
    EventsType::const_iterator iter = aEvents.begin();
    for ( ; iter != aEvents.end(); ++iter) {
        ProcessOneEvent(*iter);
    }
    return CAW_OK;
}

CAWResult CFSEventQueueBase::ProcessOneEvent(IAWEvent *aEvent)
{
    CAW_ASSERTE_RETURN(aEvent, CAW_ERROR_INVALID_ARG);

    aEvent->OnEventFire();
    aEvent->OnDestorySelf();


    return CAW_OK;
}


//////////////////////////////////////////////////////////////////////
// class CFSEventQueueUsingMutex
//////////////////////////////////////////////////////////////////////

CFSEventQueueUsingMutex::~CFSEventQueueUsingMutex()
{
}

CAWResult CFSEventQueueUsingMutex::PostEvent(IAWEvent *aEvent, EPriority aPri)
{
    CAWMutexGuardT<MutexType> theGuard(m_Mutex);
    return CFSEventQueueBase::PostEvent(aEvent, aPri);
}

CAWResult CFSEventQueueUsingMutex::PostEventWithOldSize(IAWEvent *aEvent, EPriority aPri, DWORD *aOldSize)
{
    CAWMutexGuardT<MutexType> theGuard(m_Mutex);
    if (aOldSize)
        *aOldSize = m_dwSize;
    return CFSEventQueueBase::PostEvent(aEvent, aPri);
}
}//namespace fsutils
