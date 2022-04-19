/***********************************************************************
 * Copyright (C) 2016-2018, Nanjing StarOS Technology Co., Ltd 
**********************************************************************/

#ifndef __IFSSTACK_H__
#define __IFSSTACK_H__
#include "starbase/CAWDefines.h"
#include "starbase/CAWUtilTemplates.h"
#include "starbase/CAWMutex.h"
#include "starbase/CAWReferenceControl.h"
#include "starbase/CAWString.h"
#include <stdint.h>
using namespace starbase;
namespace fsutils
{
class CAW_OS_EXPORT IFSIPStackLowLevel
{
public:
    virtual ~IFSIPStackLowLevel(){}
    virtual CAWResult IPStackLowLevelOutput(const char *pkt, size_t pktsize, uint8_t portid)=0;
};

class CAW_OS_EXPORT IFSIPStack:public IAWReferenceControl
{
public:
    virtual ~IFSIPStack(){}
    virtual CAWResult OpenWithLowLevel(IFSIPStackLowLevel *plowlevel)=0;
    virtual void IPStackInput(const char *pkt, size_t pktsize)=0;
    virtual CAWResult SetIPAddress(const CAWString &ipaddr,
                                    const CAWString &ipmask,
                                    const CAWString &gateway)=0;

    virtual CAWResult RemoveIPAddress()=0;

    
    virtual CAWResult SetIP(const CAWString &ipaddr)=0;
    virtual CAWResult SetIPNetMask(const CAWString &ipnetmask)=0;
    virtual CAWResult SetGateway(const CAWString &ipgw)=0;

    virtual CAWResult GetIPAddress(CAWString &ipaddr,
                                    CAWString &ipmask,
                                    CAWString &gateway)=0;

    virtual CAWResult RemoveIPv6Addr(int8_t addr_idx, const CAWInetAddr &ipv6addr)=0;    
    virtual CAWResult AddIPv6Addr(int8_t addr_idx, const CAWInetAddr &ipv6addr)=0;  
    virtual CAWResult SetMacAddress(char mac[6])=0;
    virtual CAWResult GetMacAddress(char mac[6])=0;
    virtual CAWResult Up()=0;
    virtual CAWResult Down()=0;
    virtual uint32_t GetPortId()=0;
    virtual CAWString GetPortName()=0;
    virtual void StartDHCP()=0;
    virtual void StopDHCP()=0;
    virtual CAWResult SetDefaultNet()=0;
    virtual uint32_t GetTXCount()=0;
    virtual uint32_t GetRXCount()=0;
    virtual void EnablePromiscuous()=0;
    virtual void DisablePromiscuous()=0;
};

class CAW_OS_EXPORT IFSIPStackManager
{
public:
    static IFSIPStackManager* Instance();
    virtual ~IFSIPStackManager() {}
    virtual CAWResult Init() = 0;
    virtual void InitNetworkConnect() = 0;
    virtual BOOL IsInit() = 0;
    virtual IFSIPStack* CreateIPStack(const CAWString& strifname) = 0;
    virtual IFSIPStack* CreateIPStack(const CAWString& ifname,
        const CAWString& ipaddr,
        const CAWString& ipmask,
        const CAWString& gateway,
        char macaddr[6]) = 0;
    virtual void DestroyIPStack(IFSIPStack* pstack) = 0;
    virtual IFSIPStack* GetIPStack(const CAWString& strifname) = 0;

protected:
};

extern IFSIPStack CAW_OS_EXPORT *CreateIPStack(IFSIPStackLowLevel *psend, 
                            const CAWString &ifname,
                            const CAWString &ipaddr,
                            const CAWString &ipmask,
                            const CAWString &gateway,
                            char macaddr[6]);
extern void CAW_OS_EXPORT DestroyIPStack(IFSIPStack *pif);

}/*namespace fsutils*/

#endif//__IFSSTACK_H__

