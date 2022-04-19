#ifndef CFSREACTORNOTIFYPIPE_H
#define CFSREACTORNOTIFYPIPE_H

#include "CFSPipe.h"
#include "fstackutils/CFSInterface.h"
#include "ipstacktp/CPSReactorInterface.h"
using namespace ipstacktp;
namespace fsutils
{
class CFSReactorBase;

class CFSReactorNotifyPipe : public IPSEventHandler
{
public:
    CFSReactorNotifyPipe();
    virtual ~CFSReactorNotifyPipe();

    CAWResult Open(CFSReactorBase *aReactor);
    CAWResult Close();

    // interface IAWEventHandler
    virtual int GetHandle() const ;
    virtual int OnInput(int aFd = -1);

    CAWResult Notify(IPSEventHandler *aEh, IPSEventHandler::MASK aMask);

private:
    struct CBuffer
    {
        CBuffer(int aFd = -1,
            IPSEventHandler::MASK aMask = IPSEventHandler::NULL_MASK)
        : m_Fd(aFd), m_Mask(aMask)
        {
        }

        int m_Fd;
        IPSEventHandler::MASK m_Mask;
    };

    CFSPipe m_PipeNotify;
    CFSReactorBase *m_pReactor;
};
}//namespace fsutils
#endif // !CMREACTORNOTIFYPIPE_H

