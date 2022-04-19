#include "freebsdport.h"
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
int before_main(void)
{
    printf("before main!\n");
    return 0;
}
#define DEBUG(...)  printf(__VA_ARGS__)
typedef int func();

#pragma section(".CRT$XIU", long, read)
#pragma data_seg(".CRT$XIU")
static func* before[1] = { before_main };
#pragma data_seg()
#if 0
#include <stdio.h>

struct thing {
    int val;
    const char* str;
    int another_val;
};
struct thing data1 = { 1, "one" };
struct thing data2 = { 2, "two" };

/* The following two pointers will be placed in "my_custom_section".
 * Store pointers (instead of structs) in "my_custom_section" to ensure
 * matching alignment when accessed using iterator in main(). */
struct thing* p_one __attribute__((section("my_custom_section"))) = &data1;
struct thing* p_two __attribute__((section("my_custom_section"))) = &data2;

/* The linker automatically creates these symbols for "my_custom_section". */
extern struct thing* __start_my_custom_section;
extern struct thing* __stop_my_custom_section;

int main(void) {
    struct thing** iter = &__start_my_custom_section;
    for (; iter < &__stop_my_custom_section; ++iter) {
        printf("Have thing %d: '%s'\n", (*iter)->val, (*iter)->str);
    }
    return 0;
}
#endif

#pragma section("mysec",read,write)
int j = 2;
__declspec(allocate("mysec"))
int i = 3;

#pragma data_seg("mysec")
static int x;
#pragma data_seg()

#if 0
int main()
{
	printf("%d,%d\n", sizeof(long), sizeof(long long));

    printf("j=%d,x=%d,i=%d\n", j,x,i);

	init_freebsd_port();
}
#endif
// pragma_directive_init_seg.cpp
#include <stdio.h>
#pragma warning(disable : 4075)
typedef void(__cdecl* PF)(void);
int cxpf = 0;   // number of destructors we need to call
PF pfx[200];    // pointers to destructors.


int myexit(PF pf) {
    printf("exit\n");
    (pf)();
    printf("exit2\n");
    pfx[cxpf++] = pf;
    return 0;
}
int myexit2(PF pf) 
{
    printf("myexit2, %p\n",pf);
    return 0;
}
void  test(void)
{
    printf("test\n");
}
struct A {
    A() { puts("A()"); }
    ~A() { puts("~A()"); }
};

struct B {
    int a;
    int c;
    B() { printf("B()"); }
    ~B() { printf("~B()"); }
};

//#pragma section(".mine", write, read)
#pragma section(".mine", read)
__declspec(allocate(".mine"))const PF _sssInitSegStart1 = (PF)1;
#if 0
//#pragma section(".mine", write, read)
_Pragma("section(\".mine\", write, read)")
_Pragma("data_seg(\".mine$bs\")")
__declspec(allocate(".mine$bs"))  const PF _sssInitSegStart = (PF)myexit2;
_Pragma("data_seg()")
#endif
// ctor & dtor called by CRT startup code
// because this is before the pragma init_seg
//A aaaa;

// The order here is important.
// Section names must be 8 characters or less.
// The sections with the same name before the $
// are merged into one section. The order that
// they are merged is determined by sorting
// the characters after the $.
// InitSegStart and InitSegEnd are used to set
// boundaries so we can find the real functions
// that we need to call for initialization.

_Pragma("section(\".mine\",read)")__declspec(allocate(".mine"))static const PF _sssInitSegStart2 = (PF)myexit2;

_Pragma("section(\".mine\",read)")__declspec(allocate(".mine"))static const PF InitSegEnd = (PF)myexit2;

#define Test(x) 

_Pragma("section(\"\"\".set\"\"mine\"\"\", read)")__declspec(allocate(".mine"))static const PF __InitSegStart = (PF)myexit2;

// The comparison for 0 is important.
// For now, each section is 256 bytes. When they
// are merged, they are padded with zeros. You
// can't depend on the section being 256 bytes, but
// you can depend on it being padded with zeros.

void InitializeObjects() {

    printf("_sssInitSegStart1=%p\n", &_sssInitSegStart1);
    printf("InitializeObjects start, __InitSegStart=%p, InitSegEnd=%p,_sssInitSegStart1=%p\n", &__InitSegStart, &InitSegEnd, &_sssInitSegStart2);
    const PF* x = &_sssInitSegStart2;
    const PF* pend = &__InitSegStart;
    const PF* p = NULL;
    for (p=x;p<=pend;p++)
    {
        printf("InitializeObjects x=%p\n", p);
        if (*p)
        {
            printf("InitializeObjects (*x)()");
            (*p)();
        }
    }
    printf("InitializeObjects end\n");
}

void DestroyObjects() {
    printf("DestroyObjects, cxpf=%d\n", cxpf);
    while (cxpf > 0) {
        --cxpf;
        (pfx[cxpf])();
    }
}

// by default, goes into a read only section
//#pragma init_seg(".mine$b", myexit)
//B bbb;
//extern const PF bbbb= test;
#define	CACHE_LINE_SHIFT	6
#define	CACHE_LINE_SIZE		(1 << CACHE_LINE_SHIFT)
#define CACHE_ALIGN __declspec(align(sizeof(void*)))

struct CACHE_ALIGN S1 { // cache align all instances of S1
    int a, b, c, d;
};
struct S1 s1;   // s1 is 32-byte cache aligned
int
pmap_pte_index(uint64_t va)
{
    printf("%lld\n",va);
    return ((va >> 12) & ((1ull << 9) - 1));
}
struct testnoslabbits {
    int a;
    char bits[0];
};

void testfun()
{
    int a = 6;
    int b = 6;
    printf("%d\n", a % b);
}
int main(int argc, char **argv) {
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
    /*Main Thread Loop*/
    printf("main start, long size=%d\n", sizeof(long));
    //InitializeObjects();
    //DestroyObjects();
    u_long ss;
    port_freebsd_init();

    vos_freebsd_init();

    testfun();
    int a = vos_sock_socket(AF_INET, SOCK_STREAM, 0);
    int b = vos_sock_socket(AF_INET, SOCK_STREAM, 0);
    int c = vos_sock_socket(AF_INET, SOCK_STREAM, 0);
    printf("socket=%d,b=%d,c=%d\n", a, b, c);
    printf("main end, %d\n",1);
}