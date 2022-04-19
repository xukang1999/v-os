#include "CFSIPStackLib.h"
#include "CFSIPStack.h"
namespace fsutils
{
IFSIPStackManager* IFSIPStackManager::Instance()
{
    return CAWSingletonT<CFSIPStackLib>::Instance();
}

CFSIPStackLib::CFSIPStackLib()
    :m_ethernetif_index(0)
    , m_isInit(FALSE)
    , m_isnetworkInit(FALSE)
{
    CAW_INFO_TRACE("CFSIPStackLib::CFSIPStackLib");
}

CFSIPStackLib::~CFSIPStackLib()
{
    CAW_INFO_TRACE("CFSIPStackLib::~CFSIPStackLib");
}
BOOL CFSIPStackLib::IsInit()
{
    return m_isInit.Value();
}
void CFSIPStackLib::InitNetworkConnect()
{

}
CAWResult CFSIPStackLib::Init()
{
    if (m_isInit.Value() == TRUE)
    {
        return CAW_OK;
    }
    CAW_INFO_TRACE("CFSIPStackLib::Init");

    m_isInit = TRUE;
    return CAW_OK;
}

IFSIPStack* CFSIPStackLib::CreateIPStack(const CAWString& ifname,
    const CAWString& ipaddr,
    const CAWString& ipmask,
    const CAWString& gateway,
    char macaddr[6])
{
    CAWMutexGuardT<CAWMutexThread> theGuard(m_ifMutex);

    CFSIPStack* pstack = new CFSIPStack(this, ifname, ipaddr,ipmask,gateway,macaddr,m_ethernetif_index);
    if (pstack == NULL)
    {
        return NULL;
    }

    CAWAutoPtr<CFSIPStack> tmpstack(pstack);

    m_iflist[m_ethernetif_index] = tmpstack;

    m_ethernetif_index = m_ethernetif_index + 1;

    return pstack;
}
IFSIPStack *CFSIPStackLib::CreateIPStack(const CAWString &ifname)
{
    CAWMutexGuardT<CAWMutexThread> theGuard(m_ifMutex); 

    CFSIPStack *pstack = new CFSIPStack(this,ifname,m_ethernetif_index);
    if (pstack==NULL)
    {
        return NULL;
    }

    CAWAutoPtr<CFSIPStack> tmpstack(pstack);

    m_iflist[m_ethernetif_index]=tmpstack;

    m_ethernetif_index=m_ethernetif_index+1;

    return pstack;  
}
IFSIPStack * CFSIPStackLib::GetIPStack(const CAWString &strifname)
{
    CAWMutexGuardT<CAWMutexThread> theGuard(m_ifMutex);
    std::unordered_map<uint32_t,CAWAutoPtr<CFSIPStack>>::iterator it = m_iflist.begin();
    while(it != m_iflist.end())
    {
        CAWAutoPtr<CFSIPStack> &ipstack=it->second;
        if (ipstack->GetPortName()==strifname)
        {
            return ipstack.Get();
        }
        it++;
    }
    return NULL;  
}

void CFSIPStackLib::DestroyIPStack(IFSIPStack *pstack)
{
    CAWMutexGuardT<CAWMutexThread> theGuard(m_ifMutex);
    if (pstack == NULL)
    {
        return;
    }
    uint32_t index=pstack->GetPortId();
    std::unordered_map<uint32_t,CAWAutoPtr<CFSIPStack>>::iterator it = m_iflist.find(index);
    if(it == m_iflist.end())
    {
        return;
    }
    m_iflist.erase(it);

    return;
}
CAWResult CFSIPStackLib::LowlevelOutput(const char *pkt,size_t pktsize, uint32_t portno)
{
    CAW_INFO_TRACE("CFSIPStackLib::LowlevelOutput,pktsize="<<pktsize<<",portno="<<portno);
    CAWMutexGuardT<CAWMutexThread> theGuard(m_ifMutex);
    if ((pkt == NULL) || (pktsize==0))
    {
        return CAW_ERROR_FAILURE;
    }
    std::unordered_map<uint32_t,CAWAutoPtr<CFSIPStack>>::iterator it = m_iflist.find(portno);
    if(it == m_iflist.end())
    {
        return CAW_ERROR_FAILURE;
    }
    CAWAutoPtr<CFSIPStack> &ipstack=it->second;
    return ipstack->IPStackLowLevelOutput(pkt,pktsize,portno);
}

IFSIPStack *CreateIPStack(IFSIPStackLowLevel *psend,
                            const CAWString &ifname,
                            const CAWString &ipaddr,
                            const CAWString &ipmask,
                            const CAWString &gateway,
                            char macaddr[6])
{
    IFSIPStack *pstack=IFSIPStackManager::Instance()->CreateIPStack(ifname,ipaddr,ipmask,gateway,macaddr);
    if (pstack)
    {
        pstack->OpenWithLowLevel(psend);
        pstack->Up();
        return pstack;
    }
    else 
    {
        return NULL;
    }
    
}

void DestroyIPStack(IFSIPStack *pif)
{
    if (pif!=NULL)
    {
        IFSIPStackManager::Instance()->DestroyIPStack(pif);
    }
}
}

