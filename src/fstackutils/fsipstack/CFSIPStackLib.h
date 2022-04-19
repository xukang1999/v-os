#ifndef CFSIPSTACKLIB_H
#define CFSIPSTACKLIB_H
#include "starbase/CAWBase64.h"
#include "starbase/CAWReferenceControl.h"
#include "starbase/CAWReactorInterface.h"
#include <iostream>
#include "starbase/CAWConditionVariable.h"
#include "fstackutils/IFSIPStack.h"
#include "CFSIPStack.h"
#include "starbase/CAWAtomic32.h"
using namespace starbase;
namespace fsutils
{
class CFSIPStackLib:public IFSIPStackManager
{
public:
    CFSIPStackLib();
    virtual ~CFSIPStackLib();
    virtual CAWResult Init();
    virtual BOOL IsInit();
    virtual void InitNetworkConnect();
    virtual IFSIPStack *CreateIPStack(const CAWString &strifname);
    virtual IFSIPStack* CreateIPStack(const CAWString& ifname,
        const CAWString& ipaddr,
        const CAWString& ipmask,
        const CAWString& gateway,
        char macaddr[6]);
    virtual void DestroyIPStack(IFSIPStack*pstack);
    virtual IFSIPStack* GetIPStack(const CAWString &strifname);
public:
    CAWResult LowlevelOutput(const char *pkt,size_t pktsize, uint32_t portno);
private:
    friend class CAWSingletonT<CFSIPStackLib>;
    std::unordered_map<uint32_t,CAWAutoPtr<CFSIPStack>> m_iflist;
    uint32_t m_ethernetif_index;
    CAWMutexThread m_ifMutex;
    CAWAtomic32 m_isInit;
    CAWAtomic32 m_isnetworkInit;
};

IFSIPStack* CreateIPStack(IFSIPStackLowLevel *psend,
                            const CAWString &ifname,
                            const CAWString &ipaddr,
                            const CAWString &ipmask,
                            const CAWString &gateway,
                            char macaddr[6]);
void DestroyIPStack(IFSIPStack *pif);
}

#endif//CSYSDBLIB_H

