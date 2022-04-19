
#include "wface/CAWACEWrapper.h"
#include "fstackutils/CFSInterface.h"
#include "fstackutils/IFSIPStack.h"
#include "fstackutils/CFSTimerWrapperID.h"
#include "ports/port_init.h"
#include "staros/staros.h"
using namespace wface;
using namespace fsutils;
char sendbuff[1024]={'h'};
CAWThreadManager *theThreadManager = CAWThreadManager::Instance();
#define IPSTACK_MAC_ADDR_BASE            {0x05,0x04,0x02,0x03,0x04,0x05}
class CLowLeverThread : public IFSIPStackLowLevel, public IAWUDPTransportSink
{
public:
    CLowLeverThread()
        :m_ipstack(NULL)
    {
    }
    ~CLowLeverThread() {}
    void Open(IFSIPStack* ipstack,
        const CAWString& peeripaddr, uint16_t peerport,
        const CAWString& localipaddr, uint16_t localport)
    {
        m_ipstack = ipstack;
        m_peeraddr = CAWInetAddr(peeripaddr, peerport);
        CAWInetAddr localaddr(localipaddr, localport);
        CAWConnectionManager::Instance()->CreateUDPTransport(this, m_udptransport.ParaOut());
        m_udptransport->Open(localaddr);
    }
    virtual CAWResult IPStackLowLevelOutput(const char* pkt, size_t pktsize, uint8_t portid)
    {
        printf("server IPStackLowLevelOutput, aData=%d\n", pktsize);
        if (m_udptransport.Get())
        {
            m_udptransport->SendData(pkt, pktsize, m_peeraddr);
        }
        return CAW_OK;
    }
    virtual void OnReceive(CAWMessageBlock& aData, const CAWInetAddr& aAddrPeer)
    {
        printf("client udp onReceive, aData=%d\n", aData.GetChainedLength());
        CAWThread* pNetwork = CAWThreadManager::Instance()->GetThread(CAWThreadManager::TT_NETWORK);
        printf("current thread type=%d\n", pNetwork->GetThreadType());
        m_ipstack->IPStackInput((char*)aData.GetTopLevelReadPtr(), aData.GetTopLevelLength());
    }
private:
    CAWString m_ifname;
    IFSIPStack* m_ipstack;
    CAWInetAddr m_peeraddr;
    CAWAutoPtr<IAWUDPTransport> m_udptransport;
};

class CTestTransport : public IFSTransportSink
{
public:
	CTestTransport(IFSTransport *ptrans)
	{
        printf("CTestTransport:CTestTransport\n");
		m_ptrans = ptrans;
	}
    ~CTestTransport()
    {
        printf("CTestTransport:~CTestTransport\n");
        if (m_ptrans)
        {
            m_ptrans->Disconnect(CAW_OK);
            m_ptrans = NULL;
        }
    }
    virtual void OnReceive(
    CAWMessageBlock &aData,
    IFSTransport *aTrptId,
    CFSTransportParameter *aPara = NULL)
    {
        printf("CTestTransport:OnReceive data=%d\n",aData.GetChainedLength());
        
        CFSTransportParameter param;
        //param.m_Priority = CAWConnectionManager::CPRIORITY_HIGH;
        //delete this;
        //m_ptrans->Disconnect(CAW_OK);
        //m_ptrans = NULL;
        static int a = 0;
        a = a + aData.GetChainedLength();
        printf("CTestTransport:OnReceive a=%d\n", a);
        m_ptrans->SendData(aData, &param);
    }

    virtual void OnSend(
        IFSTransport *aTrptId,
        CFSTransportParameter *aPara = NULL)
		{
            printf("CTestTransport:OnSend\n");
            CAW_INFO_TRACE("CTestTransport:OnSend");
		}

    virtual void OnDisconnect(
    CAWResult aReason,
    IFSTransport *aTrptId)
    {
        printf("CTestTransport:OnDisconnect\n");
        m_ptrans = NULL;
    }
private:
    CAWAutoPtr<IFSTransport>  m_ptrans;
};

class CTestAcceptor: public IFSAcceptorConnectorSink,public IFSTimerWrapperIDSink
{
public:
    CTestAcceptor()
        :m_pthread(NULL)
    {
        //theThreadManager->CreateRealTimerThread(theThreadManager->GetAndIncreamUserType(), m_pthread, "");
        CAWTimeValue aInterval(3,0);
        m_timer.Schedule(this, aInterval);
    }
    virtual ~CTestAcceptor(){
        if (m_pthread)
        {
            m_pthread->Stop(NULL);
            m_pthread->Join();
            m_pthread->Destory(CAW_OK);
            m_pthread=NULL;
        }
    }
    virtual void OnTimer(CFSTimerWrapperID* aId)
    {
        printf("OnTimer\n");
    }

    //IAWAcceptorConnectorSink
    virtual void OnConnectIndication(
                    CAWResult aReason,
                    IFSTransport *aTrpt,
                    IFSAcceptorConnectorId *aRequestId)
    {
        std::cout<<"acceptor OnConnectIndication aReason="<<aReason<<std::endl;
        //CAW_INFO_TRACE("OnConnectIndication aReason="<<aReason);
        if (aTrpt)
        {
            CTestTransport *ptrans = new CTestTransport(aTrpt);
            aTrpt->OpenWithSink(ptrans);
            CAWMessageBlock mbOn(
                sizeof(sendbuff), 
                sendbuff, 
                CAWMessageBlock::DONT_DELETE | CAWMessageBlock::WRITE_LOCKED, 
                sizeof(sendbuff));
            aTrpt->SendData(mbOn);

            //CAWResult rv;//aTrpt->SendData(mbOn);
            //std::cout<<"send data rv="<<rv<<std::endl;
            //m_ptrans=ptrans;
        }
    }

    CAWResult StartServer(const std::string ip, WORD16 port)
    {
        CAWResult rv;

        CAWInetAddr srvaddr(ip.c_str(),port);

        IFSConnectionManager::CType typeConnector = IFSConnectionManager::CTYPE_TCP;
        rv = IFSConnectionManager::Instance()->CreateConnectionServer(
            typeConnector,
            m_pAcceptorTcp.ParaOut());
        if (CAW_FAILED(rv))
        goto fail;

        rv = m_pAcceptorTcp->StartListen(this, srvaddr);
        if (CAW_FAILED(rv))
        goto fail;

        return CAW_OK;

fail:
        ///CAW_ASSERTE(CAW_SUCCEEDED(rv));
        StopServer(rv);
        return rv;
    }
    CAWResult StopServer(CAWResult aReason)
    {
        if (m_ptrans.Get())
        {
            m_ptrans->Disconnect(aReason);
            m_ptrans=NULL;
        }
        if (m_pAcceptorTcp.Get())
        {
            m_pAcceptorTcp->StopListen(aReason);
            m_pAcceptorTcp=NULL;
        }
        std::cout<<"StopServer"<<endl;
        if (m_pthread)
        {
            std::cout<<"StopServer Stop"<<endl;
            m_pthread->Stop(NULL);
            m_pthread->Join();
            m_pthread->Destory(CAW_OK);
            m_pthread=NULL;
        }
        std::cout<<"StopServer end"<<endl;

        return CAW_OK;
    }
    void close (void)
    {
    }

private:
    CAWAutoPtr<IFSAcceptor>  m_pAcceptorTcp;
    CAWAutoPtr<IFSTransport>  m_ptrans;
    CAWThread *m_pthread;
    CFSTimerWrapperID m_timer;
};

int main(int argc, char **argv)
{
    CAWThread* pMain = NULL;
    CAWResult rv;
    //rv = CAWThreadManager::Instance()->InitMainThread(argc, argv,CAWThreadManager::TM_SINGLE_MAIN, 5);
    //rv = CAWThreadManager::Instance()->Init_SingleMain_MultiNetwork_Thread(argc, argv, 0, 5);
    //rv = CAWThreadManager::Instance()->Init_SingleMain_Thread(argc, argv, 0);
    //rv = CAWThreadManager::Instance()->Init_SingleMain_MultiNetwork_Thread(argc, argv, 0, 5);
    rv = CAWThreadManager::Instance()->Init_MultiWorkThread_MultiNetworkThread(argc, argv, 0);
    if (CAW_FAILED(rv))
    {
        CAW_ERROR_TRACE("ERROR: InitMainThread() failed! rv=" << rv);
        return (int)rv;
    }

    port_freebsd_init();

    vos_init();

    IFSConnectionManager::Instance();
    NetworkThreadPoolParam param;
    CAWConnectionManager::Instance()->GetNetworkThreadPoolDefaultParam(&param);
    CAWConnectionManager::Instance()->Init(&param);

    IFSIPStackManager::Instance()->Init();

    //theThreadManager.SleepMs(1000);
    CAWString ipaddr="192.168.81.2";
    CAWString ipmask = "255.255.255.0";
    CAWString gateway="192.168.81.2";
    uint8_t portid=0;
    char macaddr[6]= IPSTACK_MAC_ADDR_BASE;
    CLowLeverThread llthread;
    //::memset(macaddr, 0, 6);
    IFSIPStack *p=CreateIPStack(&llthread,"ss", ipaddr,
                                    ipmask,
                                    gateway, macaddr);
    p->EnablePromiscuous();

    llthread.Open(p, "127.0.0.1", 22222, "127.0.0.1", 11111);




    CTestAcceptor acceptor;
    acceptor.StartServer("192.168.81.2",33333);


    pMain = theThreadManager->GetThread(CAWThreadManager::TT_MAIN);
    if (!pMain)
    {
        printf("ERROR: Get the Engine thread failed!");
        return 1;
    }

    printf("Start Main Thread\n");

    /*Main Thread Loop*/
    CAWThreadManager::Instance()->MainThreadRun(pMain);
    //DestroyIPStack(p);

    printf("Server exit now.");

    return 0;
}



