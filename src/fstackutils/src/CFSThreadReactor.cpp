/***********************************************************************
 * Copyright (C) 2016-2018, Nanjing StarOS Technology Co., Ltd 
**********************************************************************/
#include "CFSThreadReactor.h"

namespace fsutils
{
CFSThreadReactor::CFSThreadReactor()
    : m_pReactor(NULL)
{
    CAW_INFO_TRACE_THIS("CFSThreadReactor::CFSThreadReactor");
}

CFSThreadReactor::~CFSThreadReactor()
{
    CAW_INFO_TRACE_THIS("CFSThreadReactor::~CFSThreadReactor");
    delete m_pReactor;
}

CAWResult CFSThreadReactor::Init(IPSReactor *aReactor)
{
    CAW_ASSERTE_RETURN(!m_pReactor, CAW_ERROR_ALREADY_INITIALIZED);
    CAW_ASSERTE_RETURN(aReactor, CAW_ERROR_INVALID_ARG);

    m_pReactor = aReactor;
    return CAW_OK;
}

CAWResult CFSThreadReactor::Create(CAWThreadManager::TType aType, CAWThreadManager::TFlag aFlag)
{
    CAWResult rv = CAWThread::CreateNotRun(aType, aFlag);
    if (CAW_SUCCEEDED(rv)) {
        // have to open reactor here because main function will do some initial stuffs.
        if (m_Type == CAWThreadManager::TT_MAIN) {
            rv = m_pReactor->Open();
            if (CAW_FAILED(rv)) {
                CAW_ERROR_TRACE("CFSThreadReactor::OnThreadRun, m_pReactor->Open() failed! rv=" << rv);
            }
        }
    }
    return rv;
}

void CFSThreadReactor::OnThreadInit()
{
    CAW_ASSERTE_RETURN_VOID(m_pReactor);

    if (m_Type != CAWThreadManager::TT_MAIN) {
        CAWResult rv = m_pReactor->Open();
        if (CAW_FAILED(rv)) {
            CAW_ERROR_TRACE("CFSThreadReactor::OnThreadInit, m_pReactor->Open() failed! rv=" << rv);
            CAW_ASSERTE(FALSE);
        }
    }

}
IPSReactor* CFSThreadReactor::GetPSReactor()
{
    return m_pReactor;
}
void CFSThreadReactor::OnThreadRun()
{
    CAW_ASSERTE_RETURN_VOID(m_pReactor);

    m_pReactor->RunEventLoop();

    // close the notify avoid Close in other thread .
    // because it will remove handler in the reactor.
    m_pReactor->Close();
}

CAWResult CFSThreadReactor::Stop(CAWTimeValue* aTimeout)
{
    CAW_ASSERTE_RETURN(!aTimeout, CAW_ERROR_NOT_IMPLEMENTED);
    CAW_ASSERTE_RETURN(m_pReactor, CAW_ERROR_NOT_INITIALIZED);
    return m_pReactor->StopEventLoop();
}

IAWReactor* CFSThreadReactor::GetReactor()
{
    return NULL;
}

IAWEventQueue* CFSThreadReactor::GetEventQueue()
{
    return m_pReactor;
}

IAWTimerQueue* CFSThreadReactor::GetTimerQueue()
{
    return m_pReactor;
}

}//namespace fsutils