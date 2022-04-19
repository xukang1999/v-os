#ifndef CFSIPSTACK_H
#define CFSIPSTACK_H
#include "fstackutils/IFSIPStack.h"
#include "starbase/CAWInetAddr.h"
#include "staros/staros.h"
using namespace starbase;
namespace fsutils
{
class CFSIPStackLib;
class CFSIPStack : public IFSIPStack, public CAWReferenceControlMutilThread
{
public:
    CFSIPStack(CFSIPStackLib*pmgr,const CAWString &strifname, uint32_t index);
    CFSIPStack(CFSIPStackLib* pmgr, const CAWString& ifname,
        const CAWString& ipaddr,
        const CAWString& ipmask,
        const CAWString& gateway,
        char macaddr[6], uint32_t index);
    virtual ~CFSIPStack();
    virtual DWORD AddReference();
    virtual DWORD ReleaseReference();

    virtual CAWResult OpenWithLowLevel(IFSIPStackLowLevel *plowlevel);
    virtual void IPStackInput(const char *pkt, size_t pktsize);
    virtual CAWResult SetIPAddress(const CAWString &ipaddr,
                                    const CAWString &ipmask,
                                    const CAWString &gateway);

    virtual CAWResult RemoveIPAddress();

    
    virtual CAWResult SetIP(const CAWString &ipaddr);
    virtual CAWResult SetIPNetMask(const CAWString &ipnetmask);
    virtual CAWResult SetGateway(const CAWString &ipgw);

    virtual CAWResult GetIPAddress(CAWString &ipaddr,
                                    CAWString &ipmask,
                                    CAWString &gateway);

    virtual CAWResult RemoveIPv6Addr(int8_t addr_idx, const CAWInetAddr &ipv6addr);    
    virtual CAWResult AddIPv6Addr(int8_t addr_idx, const CAWInetAddr &ipv6addr);  
    virtual CAWResult SetMacAddress(char mac[6]);
    virtual CAWResult GetMacAddress(char mac[6]);
    virtual CAWResult Up();
    virtual CAWResult Down();
    virtual uint32_t GetPortId();
    virtual CAWString GetPortName();
    virtual void StartDHCP();
    virtual void StopDHCP();
    virtual CAWResult SetDefaultNet();
    virtual uint32_t GetTXCount();
    virtual uint32_t GetRXCount();
    virtual void EnablePromiscuous();
    virtual void DisablePromiscuous();
    void OnOutput(const char* pkt, int16_t len);
public:
    void RXInc();
    void TXInc();
    CAWResult IPStackLowLevelOutput(const char *pkt, size_t pktsize, uint8_t portid);
public:
    struct vos_veth* m_vnet;
    CFSIPStackLib *m_pmgr;
    IFSIPStackLowLevel *m_psend;
    CAWString m_ifname;
    uint32_t m_portid;
    uint32_t m_tx;
    uint32_t m_rx;
    bool m_ispromiscuous;
    CAWString m_stripaddr;
    CAWString m_stripmask;
    CAWString m_strgateway;
    CAWString m_strbroadcast;
    CAWThread *m_pcurrentthread;
    char m_macaddr[6];
};
}

#endif//

