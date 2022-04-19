#include "CFSIPStack.h"
#include "CFSIPStackLib.h"
#include "starbase/CAWUtils.h"
using namespace starbase;
#define ETH_MIN_FRAME_LEN      60U
#define ETH_MAX_FRAME_LEN      1518U
#define IPSTACK_MAC_ADDR_BASE            {0x00,0x01,0x02,0x03,0x04,0x05}
namespace fsutils
{
    int eth_output(struct vos_veth* veth, void* m)
    {
        CFSIPStack *pstack = (CFSIPStack *)vos_veth_get_cookie(veth);
        if (pstack)
        {
            int16_t total = vos_mbuf_getpktlen(m);
            char data[1600];
            int len = total > 1600 ? 1600 : total;
            int ret = vos_mbuf_copydata(m, data, 0, len);
            pstack->OnOutput(data, len);
        }
        return 0;
    }
class CFSIPStackOutputEvent : public IAWEvent
{
public:
    CFSIPStackOutputEvent(CFSIPStackLib *ipmgr,const char *pkt, size_t pktsize, uint8_t portid)
        :m_ipmgr(ipmgr)
        ,m_pkt(NULL)
        ,m_pktsize(0)
        ,m_portid(portid)
    {
        m_pkt=new char[pktsize];
        if (m_pkt)
        {
            ::memcpy(m_pkt,pkt,pktsize);
            m_pktsize=pktsize;
        }
    }
    virtual ~CFSIPStackOutputEvent()
    {
        delete[] m_pkt;
    }
    virtual CAWResult OnEventFire()
    {
        if ((m_pkt!=NULL)&&(m_pktsize != 0))
        {
            m_ipmgr->LowlevelOutput(m_pkt,m_pktsize,m_portid);
        }
        return CAW_OK;
    }
private:
    CFSIPStackLib *m_ipmgr;
    char *m_pkt;
    size_t m_pktsize;
    uint8_t m_portid;
};

CFSIPStack::CFSIPStack(CFSIPStackLib *pmgr,const CAWString &strifname, uint32_t indexs)
    :m_pmgr(pmgr)
    ,m_psend(NULL)
    ,m_ifname(strifname)
    ,m_portid(indexs)
    ,m_tx(0)
    ,m_rx(0)
    ,m_ispromiscuous(true)
    ,m_pcurrentthread(CAWThreadManager::Instance()->GetThread(CAWThreadManager::TT_CURRENT))
{
    CAW_INFO_TRACE("CFSIPStack::CFSIPStack");
    ::memset(m_macaddr, 0, sizeof(m_macaddr));
    m_vnet = NULL;
}

CFSIPStack::CFSIPStack(CFSIPStackLib* pmgr, const CAWString& ifname,
    const CAWString& ipaddr,
    const CAWString& ipmask,
    const CAWString& gateway,
    char macaddr[6], uint32_t index)
    :m_pmgr(pmgr)
    , m_psend(NULL)
    , m_ifname(ifname)
    , m_portid(index)
    , m_tx(0)
    , m_rx(0)
    , m_stripaddr(ipaddr)
    , m_stripmask(ipmask)
    , m_strgateway(gateway)
    , m_ispromiscuous(true)
    , m_pcurrentthread(CAWThreadManager::Instance()->GetThread(CAWThreadManager::TT_CURRENT))
{
    CAW_INFO_TRACE("CFSIPStack::CFSIPStack");
    ::memcpy(m_macaddr, macaddr, 6);
    m_vnet = NULL;
}

DWORD CFSIPStack::AddReference()
{
    return CAWReferenceControlMutilThread::AddReference();
}
DWORD CFSIPStack::ReleaseReference()
{
    return CAWReferenceControlMutilThread::ReleaseReference();
}
CAWResult CFSIPStack::OpenWithLowLevel(IFSIPStackLowLevel *plowlevel)
{
    CAW_INFO_TRACE("CFSIPStack::OpenWithLowLevel,m_portid="<<m_portid<<",m_ifname="<<m_ifname);

    m_psend=plowlevel;


    CAWInetAddr Inetaddr(m_stripaddr.c_str(), 0);
    CAWInetAddr Inetmask(m_stripmask.c_str(), 0);
    CAWInetAddr Inetgw(m_strgateway.c_str(), 0);

    uint32_t i4bytes = (uint32_t)Inetaddr.GetIpAddrIn4Bytes();
    uint32_t m4bytes = (uint32_t)Inetmask.GetIpAddrIn4Bytes();
    uint32_t g4bytes = (uint32_t)Inetgw.GetIpAddrIn4Bytes();
    uint32_t broadcast = 0;
    char ifname[VOS_IF_NAMESIZE] = { 0 };
    snprintf(ifname, sizeof(ifname), "veth-0-%d", m_portid);
    m_vnet = create_vos_veth(&i4bytes,
        &m4bytes,
        &broadcast,
        &g4bytes,
        m_macaddr,
        ifname,
        eth_output, this);
    return CAW_OK;
}
CAWResult CFSIPStack::SetIP(const CAWString &stripaddr)
{
    in_addr_t ipaddr;

    
    CAWInetAddr Inetaddr(stripaddr.c_str(), 0);


    uint32_t i4bytes = (uint32_t)Inetaddr.GetIpAddrIn4Bytes();


    ::memcpy(&ipaddr, &i4bytes, 4);


    vos_veth_set_gateway(m_vnet, &ipaddr);
    m_stripaddr = stripaddr;

    return CAW_OK;

}

CAWResult CFSIPStack::SetIPNetMask(const CAWString &ipnetmask)
{
    in_addr_t netmask;
    
    CAWInetAddr Inetmask(ipnetmask.c_str(), 0);

    uint32_t m4bytes = (uint32_t)Inetmask.GetIpAddrIn4Bytes();

    ::memcpy(&netmask, &m4bytes, 4);


    m_stripmask = ipnetmask;

    return CAW_OK;

}

CAWResult CFSIPStack::SetGateway(const CAWString &ipgw)
{

    in_addr_t gw;
    
    CAWInetAddr Inetgw(ipgw.c_str(), 0);


    uint32_t g4bytes = (uint32_t)Inetgw.GetIpAddrIn4Bytes();

    ::memcpy(&gw, &g4bytes, 4);

    vos_veth_set_gateway(m_vnet, &gw);

    return CAW_OK;

}

CAWResult CFSIPStack::SetIPAddress(const CAWString &stripaddr,
                                const CAWString &stripmask,
                                const CAWString &strbroadcast)
{
    m_stripaddr=stripaddr;
    m_stripmask=stripmask;
    m_strbroadcast = strbroadcast;
    in_addr_t ipaddr;
    in_addr_t netmask;
    in_addr_t gw;
    in_addr_t ba=0;
    CAWInetAddr Inetaddr(stripaddr.c_str(), 0);
    CAWInetAddr Inetmask(stripmask.c_str(), 0);
    CAWInetAddr Inetgw(m_strbroadcast.c_str(), 0);

    uint32_t i4bytes = (uint32_t)Inetaddr.GetIpAddrIn4Bytes();
    uint32_t m4bytes = (uint32_t)Inetmask.GetIpAddrIn4Bytes();
    uint32_t g4bytes = (uint32_t)Inetgw.GetIpAddrIn4Bytes();

    ::memcpy(&ipaddr, &i4bytes, 4);
    ::memcpy(&netmask, &m4bytes, 4);
    ::memcpy(&gw, &g4bytes, 4);

    vos_veth_setaddr(m_vnet,&ipaddr,&netmask,&gw);


    return CAW_OK;
}
CAWResult CFSIPStack::GetIPAddress(CAWString &ipaddr,
                            CAWString &ipmask,
                            CAWString &gateway)
{
    ipaddr=m_stripaddr;
    ipmask=m_stripmask;
    gateway=m_strgateway;
    return CAW_OK;
}
CAWResult CFSIPStack::RemoveIPAddress()
{
    destroy_vos_veth(m_vnet);
    m_vnet = NULL;
    return CAW_OK;
}

CAWResult CFSIPStack::RemoveIPv6Addr(int8_t addr_idx, const CAWInetAddr &ipv6addr)
{
    struct in6_addr ip6;

    memset(&ip6, 0, sizeof(ip6));

    vos_veth_setaddr6(m_vnet, &ip6);
    return CAW_OK;
}

CAWResult CFSIPStack::AddIPv6Addr(int8_t addr_idx, const CAWInetAddr &ipv6addr)
{
    struct in6_addr ip6;

    memcpy(&ip6,(char *)ipv6addr.GetIpAddrPointer(),sizeof(ip6));
    
    vos_veth_setaddr6(m_vnet, &ip6);
    return CAW_OK;
}

CAWResult CFSIPStack::SetMacAddress(char mac[6])
{
    char zeromac[6]={0};
    if (memcmp(zeromac,mac,6)==0)
    {
        return CAW_ERROR_FAILURE;
    }
    vos_veth_set_macaddr(m_vnet,mac);
    return CAW_OK;
}

CAWResult CFSIPStack::GetMacAddress(char mac[6])
{
    vos_veth_get_macaddr(m_vnet, mac);
    return CAW_OK;
}

                                
CAWResult CFSIPStack::SetDefaultNet()
{
    return CAW_OK;
}

CAWResult CFSIPStack::Up()
{
    return CAW_OK;
}
CAWResult CFSIPStack::Down()
{
    return CAW_OK;
}


CFSIPStack::~CFSIPStack()
{
    CAW_INFO_TRACE("CFSIPStack::~CFSIPStack");
}

void CFSIPStack::IPStackInput(const char *pkt, size_t pktsize)
{
    CAW_INFO_TRACE("CFSIPStack::IPStackInput,pktsize="<<pktsize<<",m_ispromiscuous="<<m_ispromiscuous);

    if (m_vnet)
    {
        vos_veth_input(m_vnet, pkt, pktsize, 1);
    }
}

CAWResult CFSIPStack::IPStackLowLevelOutput(const char *pkt, size_t pktsize, uint8_t portid)
{
    CAWResult rv=CAW_ERROR_FAILURE;
    if (m_psend==NULL)
    {
        return CAW_ERROR_FAILURE;
    }

    rv=m_psend->IPStackLowLevelOutput((char *)pkt, pktsize,portid);
    if (CAW_FAILED(rv))
    {
        return CAW_ERROR_FAILURE;
    }
    else 
    {
        TXInc();
        return CAW_OK;
    } 
}

uint32_t CFSIPStack::GetPortId()
{
    return m_portid;
}
CAWString CFSIPStack::GetPortName()
{
    return m_ifname;
}

void CFSIPStack::StartDHCP()
{

}
void CFSIPStack::StopDHCP()
{

}

uint32_t CFSIPStack::GetTXCount()
{
    return m_tx;
}
uint32_t CFSIPStack::GetRXCount()
{
    return m_rx;
}

void CFSIPStack::RXInc()
{
    m_tx++;
}
void CFSIPStack::TXInc()
{
    m_rx++;
}

void CFSIPStack::EnablePromiscuous()
{
    m_ispromiscuous=true;
}
void CFSIPStack::DisablePromiscuous()
{
    m_ispromiscuous=false;
}

void CFSIPStack::OnOutput(const char* pkt, int16_t len)
{
    if (m_psend)
    {
        m_psend->IPStackLowLevelOutput(pkt, len, m_portid);
    }
}

}
