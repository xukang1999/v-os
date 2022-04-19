/***********************************************************************
 * Copyright (C) 2016-2018, Nanjing StarOS Technology Co., Ltd 
**********************************************************************/
#include "CFSReactorBase.h"

namespace fsutils
{
CFSEventHandlerRepository::CFSEventHandlerRepository()
{
}

CFSEventHandlerRepository::~CFSEventHandlerRepository()
{
    Close();
}

CAWResult CFSEventHandlerRepository::Open()
{

    return CAW_OK;
}

CAWResult CFSEventHandlerRepository::Close()
{
    return CAW_OK;
}


//////////////////////////////////////////////////////////////////////
// class CFSReactorBase
//////////////////////////////////////////////////////////////////////

CFSReactorBase::CFSReactorBase()
    : m_pTimerQueue(NULL)
{
}

CFSReactorBase::~CFSReactorBase()
{
    
}

CAWResult CFSReactorBase::Open()
{
    m_Est.Reset2CurrentThreadId();
    CFSEventQueueUsingMutex::Reset2CurrentThreadId();
    CAWStopFlag::m_Est.Reset2CurrentThreadId();

    // check whether inheried class instanced the timer queue.
    if (!m_pTimerQueue) {
        m_pTimerQueue = new CAWTimerQueueOrderedList(NULL);
        if (!m_pTimerQueue)
            return CAW_ERROR_OUT_OF_MEMORY;
    }

    CAWResult rv = m_EhRepository.Open();
    if (CAW_FAILED(rv))
        return rv;
    return rv;
}

CAWResult CFSReactorBase::RegisterHandler(IPSEventHandler*aEh, IPSEventHandler::MASK aMask)
{
    // FIXME TODO: Register handler after OnClose!

    m_Est.EnsureSingleThread();
    CAWResult rv;
    CAW_ASSERTE_RETURN(aEh, CAW_ERROR_INVALID_ARG);

    IPSEventHandler::MASK maskNew = aMask & IPSEventHandler::ALL_EVENTS_MASK;
    if (maskNew == IPSEventHandler::NULL_MASK) {
        CAW_WARNING_TRACE("CFSReactorBase::RegisterHandler, NULL_MASK. aMask=" << aMask);
        return CAW_ERROR_INVALID_ARG;
    }

    CFSEventHandlerRepository::CElement eleFind;
    int fdNew = aEh->GetHandle();
    rv = m_EhRepository.Find(fdNew, eleFind);
    if (maskNew == eleFind.m_Mask && aEh == eleFind.m_pEh) {
        CAW_WARNING_TRACE("CFSReactorBase::RegisterHandler. already have fdNew=" << fdNew<<", aMask="<<aMask);
        return CAW_OK;
    }
    CAW_INFO_TRACE("CFSReactorBase::RegisterHandler. fdNew=" << fdNew<<", aMask="<<aMask);

    if (eleFind.IsCleared()) 
    {
        rv = OnHandleRegister(fdNew, maskNew, aEh);

        // needn't remove handle when OnHandleRegister() failed
        // because the handle didn't be inserted at all
        if (CAW_FAILED(rv))
        {
            CAW_INFO_TRACE("CFSReactorBase::RegisterHandler. OnHandleRegister error ,rv=" << rv);
            return rv;
        }
    }

    CFSEventHandlerRepository::CElement eleNew(aEh, maskNew);
    rv = m_EhRepository.Bind(fdNew, eleNew);
    return rv;
}

CAWResult CFSReactorBase::RemoveHandler(IPSEventHandler*aEh, IPSEventHandler::MASK aMask)
{
    m_Est.EnsureSingleThread();
    CAWResult rv;
    CAW_ASSERTE_RETURN(aEh, CAW_ERROR_INVALID_ARG);

    IPSEventHandler::MASK maskNew = aMask & IPSEventHandler::ALL_EVENTS_MASK;
    if (maskNew == IPSEventHandler::NULL_MASK) {
        CAW_WARNING_TRACE("CFSReactorBase::RemoveHandler, NULL_MASK. aMask=" << aMask);
        return CAW_ERROR_INVALID_ARG;
    }

    CFSEventHandlerRepository::CElement eleFind;
    int fdNew = aEh->GetHandle();
    rv = m_EhRepository.Find(fdNew, eleFind);
    if (CAW_FAILED(rv)) 
    {
        CAW_WARNING_TRACE("CFSReactorBase::RemoveHandler can not find fd="<<fdNew);
        return CAW_OK;
    }
    CAW_INFO_TRACE("CFSReactorBase::RemoveHandler. fdNew=" << fdNew<<", aMask="<<aMask);

    rv = RemoveHandleWithoutFinding_i(fdNew, eleFind, maskNew);
    return rv;
}

CAWResult CFSReactorBase::Close()
{
    if (m_pTimerQueue) {
        delete m_pTimerQueue;
        m_pTimerQueue = NULL;
    }
    m_EhRepository.Close();
    CFSEventQueueBase::DestoryPendingEvents();
    return CAW_OK;
}

CAWResult CFSReactorBase::ScheduleTimer(IAWTimerHandler *aTh, 
                                        LPVOID aArg, 
                                        const CAWTimeValue &aInterval, 
                                        DWORD aCount)
{
    m_Est.EnsureSingleThread();
    if (!m_pTimerQueue) {
        CAW_WARNING_TRACE("CFSReactorBase::ScheduleTimer, m_pTimerQueue not inited or closed.");
        return CAW_ERROR_NOT_INITIALIZED;
    }

    return m_pTimerQueue->ScheduleTimer(aTh, aArg, aInterval, aCount);
}

CAWResult CFSReactorBase::CancelTimer(IAWTimerHandler *aTh)
{
    m_Est.EnsureSingleThread();
    if (!m_pTimerQueue) {
        CAW_WARNING_TRACE("CFSReactorBase::CancelTimer, m_pTimerQueue not inited or closed.");
        return CAW_ERROR_NOT_INITIALIZED;
    }

    return m_pTimerQueue->CancelTimer(aTh);
}

CAWResult CFSReactorBase::ProcessHandleEvent(int aFd, 
                                                IPSEventHandler::MASK aMask, 
                                                CAWResult aReason, 
                                                BOOL aIsNotify, 
                                                BOOL aDropConnect)
{
    CAW_INFO_TRACE("CFSReactorBase::ProcessHandleEvent="<<aFd);

    m_Est.EnsureSingleThread();
    if (aFd == -1) {
        CAW_ASSERTE(aMask == IAWEventHandler::EVENTQUEUE_MASK);

        // get one pending event once,
        // so that the events and signals are serial.
        // can't do this because it causes signal queue overflow.

        DWORD dwRemainSize = 0;
        CFSEventQueueBase::EventsType listEvents;
        CAWResult rv = CFSEventQueueUsingMutex::PopPendingEventsWithoutWait(listEvents, CFSEventQueueBase::MAX_GET_ONCE, &dwRemainSize);
        CAW_INFO_TRACE("CFSReactorBase::ProcessHandleEvent process eventlist size="<<listEvents.size()
            <<", rv="<<rv);
        if (CAW_SUCCEEDED(rv))
        {
            CAW_INFO_TRACE("CFSReactorBase::ProcessEvents");
            rv = CFSEventQueueBase::ProcessEvents(listEvents);
        }
        
        if (dwRemainSize)
        {
            CAW_INFO_TRACE("CFSReactorBase::ProcessHandleEvent, dwRemainSize="<<dwRemainSize);
            NotifyHandler(NULL, IPSEventHandler::EVENTQUEUE_MASK);
        }
        return rv;
    }

    CFSEventHandlerRepository::CElement eleFind;
    CAWResult rv = m_EhRepository.Find(aFd, eleFind);
    if (CAW_FAILED(rv)) {
        if (!aDropConnect) {
            CAW_WARNING_TRACE("CFSReactorBase::ProcessHandleEvent, handle not registed."
            " aFd=" << aFd <<
            " aMask=" << aMask <<
            " aReason=" << aReason <<
            " rv=" << rv);
        }
        return rv;
    }

    if (CAW_BIT_DISABLED(aMask, IAWEventHandler::CLOSE_MASK)) {
        IAWEventHandler::MASK maskActual = eleFind.m_Mask & aMask;
        // needn't check the registered mask if it is notify.
        if (!maskActual && !aIsNotify) {
            CAW_WARNING_TRACE("CFSReactorBase::ProcessHandleEvent, mask not registed."
            " aFd=" << aFd <<
            " aMask=" << aMask <<
            " m_Mask=" << eleFind.m_Mask <<
            " aReason=" << aReason);
            return CAW_OK;
        }
		
		int nOnCall = 0;
		if (aDropConnect && maskActual & IAWEventHandler::CONNECT_MASK) {
			CAW_WARNING_TRACE("CFSReactorBase::ProcessHandleEvent, drop connect."
				" aFd=" << aFd <<
				" aMask=" << aMask <<
				" m_Mask=" << eleFind.m_Mask);
			nOnCall = -1;
		}
		else {
			if (maskActual & IAWEventHandler::ACCEPT_MASK
				|| maskActual & IAWEventHandler::READ_MASK)
			{
                nOnCall = eleFind.m_pEh->OnInput(aFd);
			}
			if ((nOnCall == 0) && 
				(maskActual & IAWEventHandler::CONNECT_MASK
				|| maskActual & IAWEventHandler::WRITE_MASK))
			{
				nOnCall = eleFind.m_pEh->OnOutput(aFd);
			}
		}

		if (nOnCall != 0) {
			// maybe the handle is reregiested or removed when doing callbacks. 
			// so we have to refind it.
			CFSEventHandlerRepository::CElement eleFindAgain;
			rv = m_EhRepository.Find(aFd, eleFindAgain);
			if (CAW_FAILED(rv) || eleFind.m_pEh != eleFindAgain.m_pEh) {
				CAW_ERROR_TRACE("CFSReactorBase::ProcessHandleEvent,"
					" callback shouldn't return fail after the fd is reregiested or removed!"
					" aFd=" << aFd << 
					" EHold=" << eleFind.m_pEh << 
					" EHnew=" << eleFindAgain.m_pEh << 
					" find=" << rv);
				CAW_ASSERTE(FALSE);
				
				if (CAW_SUCCEEDED(rv))
					rv = CAW_ERROR_UNEXPECTED;
			}
			else {
				rv = RemoveHandleWithoutFinding_i(aFd, eleFindAgain, 
					IAWEventHandler::ALL_EVENTS_MASK | IAWEventHandler::SHOULD_CALL);
			}
		}
	}
	else {
//		CAW_INFO_TRACE("CFSReactorBase::ProcessHandleEvent, handle is closed."
//			" aFd=" << aFd <<
//			" aMask=" << aMask <<
//			" aReason=" << aReason);

        rv = RemoveHandleWithoutFinding_i(aFd, eleFind, 
            IAWEventHandler::ALL_EVENTS_MASK | IAWEventHandler::SHOULD_CALL);
    }

	return rv;
}

CAWResult CFSReactorBase::ProcessTimerTick()
{
    m_Est.EnsureSingleThread();
    CAW_ASSERTE_RETURN(m_pTimerQueue, CAW_ERROR_NOT_INITIALIZED);
    if (m_pTimerQueue) 
        m_pTimerQueue->CheckExpire();

    return CAW_OK;
}

CAWResult CFSReactorBase::
RemoveHandleWithoutFinding_i(int aFd, 
                    const CFSEventHandlerRepository::CElement &aHe, 
                    IPSEventHandler::MASK aMask)
{
    CAW_INFO_TRACE("CFSReactorBase::RemoveHandleWithoutFinding_i");
    IPSEventHandler::MASK maskNew = aMask & IPSEventHandler::ALL_EVENTS_MASK;
    IPSEventHandler::MASK maskEh = aHe.m_Mask;
    IPSEventHandler::MASK maskSelect = (maskEh & maskNew) ^ maskEh;
    if (maskSelect == maskEh) {
        CAW_WARNING_TRACE("CFSReactorBase::RemoveHandleWithoutFinding_i, mask is equal. aMask=" << aMask);
        return CAW_OK;
    }

    if (maskSelect == IPSEventHandler::NULL_MASK) {
        CAWResult rv = m_EhRepository.UnBind(aFd);
        if (CAW_FAILED(rv)) {
            CAW_WARNING_TRACE("CFSReactorBase::RemoveHandleWithoutFinding_i, UnBind() failed!"
            " aFd=" << aFd <<
            " aMask=" << aMask <<
            " rv=" << rv);
        }
        CAW_INFO_TRACE("CFSReactorBase::RemoveHandleWithoutFinding_i,unbind" 
        "onclose");
        OnHandleRemoved(aFd);
        if (aMask & IAWEventHandler::SHOULD_CALL) {
            aHe.m_pEh->OnClose(aFd, maskEh);
        }
        return CAW_OK;
    }
    else {
        CFSEventHandlerRepository::CElement eleBind = aHe;
        eleBind.m_Mask = maskSelect;
        CAWResult rvBind = m_EhRepository.Bind(aFd, eleBind);
        CAW_ASSERTE(rvBind == CAW_ERROR_FOUND);
        return rvBind;
    }
}

// this function can be invoked in the different thread.
CAWResult CFSReactorBase::SendEvent(IAWEvent *aEvent)
{
    CAW_INFO_TRACE("CFSReactorBase::SendEvent");
    return CFSEventQueueUsingMutex::SendEvent(aEvent);
}

// this function can be invoked in the different thread.
CAWResult CFSReactorBase::PostEvent(IAWEvent* aEvent, EPriority aPri)
{
    CAW_INFO_TRACE("CFSReactorBase::PostEvent");
    DWORD dwOldSize = 0;
    CAWResult rv = CFSEventQueueUsingMutex::
        PostEventWithOldSize(aEvent, aPri, &dwOldSize);
    if (CAW_SUCCEEDED(rv) && dwOldSize == 0)
        NotifyHandler(NULL, IAWEventHandler::EVENTQUEUE_MASK);
    return rv;
}

// this function can be invoked in the different thread.
DWORD CFSReactorBase::GetPendingEventsCount()
{
    return CFSEventQueueUsingMutex::GetPendingEventsCount();
}

char *CFSReactorBase::GetBufferPtr()
{
    return m_buffer;
}
size_t CFSReactorBase::GetBufferSize()
{
    return sizeof(m_buffer);
}

void *CFSReactorBase::GetCookie()
{
    return m_cookie;
}
void CFSReactorBase::SetCookie(void *pcookie)
{
    m_cookie=pcookie;
}
}//namespace fsutils

