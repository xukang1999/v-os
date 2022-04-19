/***********************************************************************
 * Copyright (C) 2016-2018, Nanjing StarOS Technology Co., Ltd 
**********************************************************************/
#ifndef CFSREACTORBASE_H
#define CFSREACTORBASE_H

#include "fstackutils/CFSInterface.h"
#include "CFSEventQueueBase.h"
#include "ipstacktp/CPSReactorInterface.h"
using namespace ipstacktp;
namespace fsutils
{
class CFSReactorBase;

class CFSEventHandlerRepository
{
public:
    CFSEventHandlerRepository();
    ~CFSEventHandlerRepository();

    CAWResult Open();
    CAWResult Close();

    struct CElement
    {
        IPSEventHandler*m_pEh;
        IPSEventHandler::MASK m_Mask;

        CElement(IPSEventHandler*aEh = NULL,
            IPSEventHandler::MASK aMask = IPSEventHandler::NULL_MASK)
        : m_pEh(aEh), m_Mask(aMask)
        {
        }

        void Clear()
        {
            m_pEh = NULL; 
            m_Mask = IPSEventHandler::NULL_MASK;
        }

        BOOL IsCleared() const { return m_pEh == NULL; }
    };

    /**
    * If success:
    *    if <aFd> is found, return CAW_OK;
    *    else return CAW_ERROR_NOT_FOUND;
    */
    CAWResult Find(int aFd, CElement &aEle);

    /**
    * If success:
    *    if <aFd> is found, return CAW_ERROR_FOUND;
    *    else return CAW_OK;
    */
    CAWResult Bind(int aFd, const CElement &aEle);

    /**
    * If success:
    *    return CAW_OK;
    */
    CAWResult UnBind(int aFd);

    BOOL IsVaildHandle(int aFd)
    {
        return TRUE;
    }

private:
    //CElement *m_pHandlers;
    //int m_nMaxHandler;
    std::unordered_map<int, CElement> m_pHandlers;
};


// base class for rector, 
// we have to inherit from <CAWEventQueueUsingMutex> because we 
// will over write PostEvent() to do NotifyHandler().
class CFSReactorBase
                : public IPSReactor
                , public CAWStopFlag
                , public CFSEventQueueUsingMutex 
{
public:
    CFSReactorBase();
    virtual ~CFSReactorBase();

    // interface IAWReactor
    virtual CAWResult Open();

    virtual CAWResult RegisterHandler(
        IPSEventHandler*aEh,
        IPSEventHandler::MASK aMask);

    virtual CAWResult RemoveHandler(
        IPSEventHandler*aEh,
        IPSEventHandler::MASK aMask = IPSEventHandler::ALL_EVENTS_MASK);

    virtual CAWResult Close();

    // interface IAWTimerQueue
    virtual CAWResult ScheduleTimer(IAWTimerHandler *aTh, 
                    LPVOID aArg,
                    const CAWTimeValue &aInterval,
                    DWORD aCount);

    virtual CAWResult CancelTimer(IAWTimerHandler *aTh);
    virtual char *GetBufferPtr() ;
    virtual size_t GetBufferSize();

    virtual void *GetCookie() ;
    virtual void SetCookie(void *pcookie) ;

    // interface IAWEventQueue
    virtual CAWResult SendEvent(IAWEvent *aEvent);
    virtual CAWResult PostEvent(
        IAWEvent *aEvent,       
        EPriority aPri = IAWReactor::EPRIORITY_NORMAL);
    virtual DWORD GetPendingEventsCount();

    CAWResult ProcessHandleEvent(
        int aFd, 
        IPSEventHandler::MASK aMask,
        CAWResult aReason,
        BOOL aIsNotify,
        BOOL aDropConnect = FALSE);

protected:
    CAWResult ProcessTimerTick();

    virtual void OnHandleRemoved(int aFd) = 0;
    virtual CAWResult OnHandleRegister(
        int aFd, 
        IPSEventHandler::MASK aMask, 
        IPSEventHandler *aEh) = 0;

    CAWEnsureSingleThread m_Est;
    CAWTimerQueueOrderedList *m_pTimerQueue;

private:
    CAWResult RemoveHandleWithoutFinding_i(
        int aFd, 
        const CFSEventHandlerRepository::CElement &aHe, 
        IPSEventHandler::MASK aMask);

protected:
    CFSEventHandlerRepository m_EhRepository;
    char m_buffer[65535];
    void *m_cookie;

};


// inline functions
inline CAWResult CFSEventHandlerRepository::Find(int aFd, CElement &aEle)
{
    CAW_ASSERTE_RETURN(IsVaildHandle(aFd), CAW_ERROR_INVALID_ARG);

    CElement &eleFind = m_pHandlers[aFd];
    if (eleFind.IsCleared()) 
        return CAW_ERROR_NOT_FOUND;
    else {
        aEle = eleFind;
        return CAW_OK;
    }
}

inline CAWResult CFSEventHandlerRepository::Bind(int aFd, const CElement &aEle)
{
    CAW_ASSERTE_RETURN(IsVaildHandle(aFd), CAW_ERROR_INVALID_ARG);
    CAW_ASSERTE_RETURN(!aEle.IsCleared(), CAW_ERROR_INVALID_ARG);

    CElement &eleBind = m_pHandlers[aFd];

    BOOL bNotBound = eleBind.IsCleared();
    eleBind = aEle;
    return bNotBound ? CAW_OK : CAW_ERROR_FOUND;
}

inline CAWResult CFSEventHandlerRepository::UnBind(int aFd)
{
    CAW_ASSERTE_RETURN(IsVaildHandle(aFd), CAW_ERROR_INVALID_ARG);
    m_pHandlers[aFd].Clear();

    return CAW_OK;
}
}//namespace fsutils
#endif // !CMREACTORBASE_H

