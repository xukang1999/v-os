#include "ports/port_init.h"
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <starbase/CAWStarBase.h>
#include "wface/CAWACEWrapper.h"
#include "staros/staros.h"
using namespace starbase;
using namespace wface;
int main(int argc, char **argv) {
	printf("start\n");
    CAWThread* pMain = NULL;
    CAWResult rv;
    //rv = CAWThreadManager::Instance()->InitMainThread(argc, argv,CAWThreadManager::TM_SINGLE_MAIN, 5);
    //rv = CAWThreadManager::Instance()->Init_SingleMain_MultiNetwork_Thread(argc, argv, 0, 5);
    //rv = CAWThreadManager::Instance()->Init_SingleMain_Thread(argc, argv);
    //rv = CAWThreadManager::Instance()->Init_SingleMain_MultiNetwork_Thread(argc, argv, 0, 5);
    rv = CAWThreadManager::Instance()->Init_MultiWorkThread_MultiNetworkThread(argc, argv);
    if (CAW_FAILED(rv))
    {
        CAW_ERROR_TRACE("ERROR: InitMainThread() failed! rv=" << rv);
        return (int)rv;
    }
    NetworkThreadPoolParam param;
    CAWConnectionManager::Instance()->GetNetworkThreadPoolDefaultParam(&param);
    //param.reactorperthreads = 1;
    CAWConnectionManager::Instance()->Init(&param);

    pMain = CAWThreadManager::Instance()->GetThread(CAWThreadManager::TT_MAIN);
    if (!pMain)
    {
        printf("ERROR: Get the Engine thread failed!");
    }
    CAWThreadManager::Instance()->MainThreadDetachRun(pMain, NULL);

    port_freebsd_init();

    port_second_sleep(1);

    vos_init();

    int a = vos_sock_socket(AF_INET, SOCK_STREAM, 0);
    int b = vos_sock_socket(AF_INET, SOCK_STREAM, 0);
    int c = vos_sock_socket(AF_INET, SOCK_STREAM, 0);
    printf("socket=%d,b=%d,c=%d\n", a, b, c);
    printf("main end, %d\n",1);

}