
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
class CSendDataEvent : IAWEvent
{
public:
    CSendDataEvent() {}
    virtual ~CSendDataEvent() {}
    virtual CAWResult OnEventFire() { return CAW_OK; }
private:
};
class CLowLeverThread: public IFSIPStackLowLevel,public IAWUDPTransportSink
{
public:
    CLowLeverThread()
        :m_ipstack(NULL)
    {
    }
    ~CLowLeverThread(){}
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
    virtual CAWResult IPStackLowLevelOutput(const char *pkt, size_t pktsize, uint8_t portid)
    {
        printf("client IPStackLowLevelOutput, aData=%d\n", pktsize);
        if (m_udptransport.Get())
        {
            m_udptransport->SendData(pkt, pktsize, m_peeraddr);
        }
        return CAW_OK;
    }
    virtual void OnReceive(CAWMessageBlock& aData, const CAWInetAddr& aAddrPeer)
    {
        printf("client udp onReceive, aData=%d\n",aData.GetChainedLength());
        CAWThread* pNetwork = CAWThreadManager::Instance()->GetThread(CAWThreadManager::TT_NETWORK);
        printf("current thread type=%d\n", pNetwork->GetThreadType());
        m_ipstack->IPStackInput((char*)aData.GetTopLevelReadPtr(), aData.GetTopLevelLength());
    }
private:
    CAWString m_ifname;
    IFSIPStack *m_ipstack;
    CAWInetAddr m_peeraddr;
    CAWAutoPtr<IAWUDPTransport> m_udptransport;
};

class CTestTransport : public IFSTransportSink
{
public:
	CTestTransport(IFSTransport *ptrans)
	{
		std::cout<<"CTestTransport"<<std::endl;
		m_ptrans = ptrans;
	}
    ~CTestTransport()
    {
        std::cout << "~CTestTransport" << std::endl;
    }
    virtual void OnReceive(
    CAWMessageBlock &aData,
    IFSTransport *aTrptId,
    CFSTransportParameter *aPara = NULL)
    {
        printf("OnReceive, length=%d\n", aData.GetChainedLength());
        //std::string str(aData.GetTopLevelReadPtr(), aData.GetChainedLength());
        //std::cout<<"data="<<str<<std::endl;
        //CAW_INFO_TRACE("OnReceive"); 
        CFSTransportParameter param;
        //param.m_Priority = CAWConnectionManager::CPRIORITY_HIGH;
        //m_ptrans->SendData(aData,&param);
        //delete this;
        //m_ptrans = NULL;
        static int a = 0;
        a = a + aData.GetChainedLength();
        printf("OnReceive, total=%d\n", a);
        m_ptrans->SendData(aData, &param);
    }

    virtual void OnSend(
        IFSTransport *aTrptId,
        CFSTransportParameter *aPara = NULL)
		{
						std::cout<<"OnSend"<<std::endl;
                        //CAW_INFO_TRACE("OnSend"); 
                        //m_ptrans->Disconnect(CAW_OK);
		}

    virtual void OnDisconnect(
    CAWResult aReason,
    IFSTransport *aTrptId)
    {
        std::cout<<"OnDisconnect"<<std::endl;
        //CAW_INFO_TRACE("OnDisconnect"); 
        m_ptrans = NULL;
    }
private:
    CAWAutoPtr<IFSTransport>  m_ptrans;
};

class CTestConnector: public IFSAcceptorConnectorSink,public IFSTimerWrapperIDSink
{
public:
    CTestConnector()
        :m_pthread(NULL)
    {
        //theThreadManager->CreateRealTimerThread(theThreadManager->GetAndIncreamUserType(), m_pthread, "");
        CAWTimeValue aInterval(3,0);
        m_timer.Schedule(this, aInterval);
    }
    virtual ~CTestConnector(){
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
        std::cout<<"OnConnectIndication aReason="<<aReason<<std::endl;
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
            //aTrpt->SendData(mbOn);

            //CAWResult rv;//aTrpt->SendData(mbOn);
            //std::cout<<"send data rv="<<rv<<std::endl;
            //m_ptrans=ptrans;
        }
    }

    CAWResult StartServer(const std::string ip, WORD16 port)
    {
        CAWResult rv;

        CAWInetAddr srvaddr(ip.c_str(), port);

        IFSConnectionManager::CType typeConnector = IFSConnectionManager::CTYPE_TCP;
        rv = IFSConnectionManager::Instance()->CreateConnectionClient(
            typeConnector,
            m_pConnectorTcp.ParaOut());
        if (CAW_FAILED(rv))
        {
            StopServer(rv);
            return CAW_ERROR_FAILURE;
        }

        m_pConnectorTcp->AsycConnect(this, srvaddr);


        return CAW_OK;
    }
    CAWResult StopServer(CAWResult aReason)
    {
        if (m_ptrans.Get())
        {
            m_ptrans->Disconnect(aReason);
            m_ptrans=NULL;
        }
        if (m_pConnectorTcp.Get())
        {
            m_pConnectorTcp->CancelConnect();
            m_pConnectorTcp=NULL;
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
    CAWAutoPtr<IFSConnector>  m_pConnectorTcp;
    CAWAutoPtr<IFSTransport>  m_ptrans;
    CAWThread *m_pthread;
    CFSTimerWrapperID m_timer;
};

#define IPSTACK_MAC_ADDR_BASE            {0x00,0x01,0x02,0x03,0x04,0x05}
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

    IFSIPStackManager::Instance()->Init();
    IFSConnectionManager::Instance();
    NetworkThreadPoolParam param;
    CAWConnectionManager::Instance()->GetNetworkThreadPoolDefaultParam(&param);
    CAWConnectionManager::Instance()->Init(&param);

    //theThreadManager.SleepMs(1000);
    CAWString ipaddr="192.168.81.1";
    CAWString ipmask = "255.255.255.0";
    CAWString gateway="192.168.81.2";

    uint8_t portid=0;
    char macaddr[6]= IPSTACK_MAC_ADDR_BASE;
    CLowLeverThread llthread;
    //::memset(macaddr, 0, 6);
    IFSIPStack *p=CreateIPStack(&llthread,"sss", ipaddr,
                                    ipmask,
                                    gateway, macaddr);
    p->EnablePromiscuous();

    llthread.Open(p,"127.0.0.1",11111,"127.0.0.1",22222);

    CTestConnector test;
    test.StartServer("192.168.81.2",33333);
    //test.StopServer(CAW_OK);


    pMain = theThreadManager->GetThread(CAWThreadManager::TT_MAIN);
    if (!pMain)
    {
        printf("ERROR: Get the Engine thread failed!");
        return 1;
    }

    printf("Start Main Thread\n");

    /*Main Thread Loop*/
    CAWThreadManager::Instance()->MainThreadRun(pMain);
    DestroyIPStack(p);

    printf("Server exit now.");

    return 0;
}



