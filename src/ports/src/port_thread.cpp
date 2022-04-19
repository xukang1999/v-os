#include "ports/port_thread.h"
#include "stdlib.h"
#ifdef _WIN32
#include "win_pthread.h"
#else
#include <pthread.h>
#endif
#include <starbase/CAWStarBase.h>
#include "wface/CAWACEWrapper.h"
using namespace starbase;
using namespace wface;

class CFBPortThreadItem :public IAWReferenceControl, public CAWReferenceControlMutilThread
{
public:
	CFBPortThreadItem(void (*func)(void*),
		void (*init)(void*),
		void* arg,
		void* newtd,
		uint32_t id);
	virtual ~CFBPortThreadItem();
	virtual void OnThreadInit();
	virtual void OnThreadRun();
	inline void* GetTd() { return m_newtd; }
	CAWResult StartThread();
	virtual DWORD AddReference();
	virtual DWORD ReleaseReference();
	inline uint32_t GetTD() { return m_id; }
#ifdef _WIN32
	static unsigned WINAPI MainThreadFun(void* arg);
#else
	static void* MainThreadFun(void* arg);
#endif // CAW_WIN32
private:
	void (*m_func)(void*);
	void (*m_init)(void*);
	void* m_arg;
	void* m_newtd;
	uint32_t m_id;
	CAWEventThread* m_pEvent4Start;
};

class CFBPortThreadManager
{
public:
	CFBPortThreadManager();
	static CFBPortThreadManager* Instance();
	virtual ~CFBPortThreadManager();
	CAWResult AddThread(uint32_t id, CFBPortThreadItem* item);
	CAWResult RemoveThread(uint32_t id);
	CAWResult FindThread(uint32_t id, CFBPortThreadItem *&pthread);
	CAWResult FindThreadByTid(void* tid, CFBPortThreadItem*& pthread);
	CAWResult CreateThread(void (*func)(void*),
		void (*init)(void*),
		void* arg,
		void* newtd);
	CAWResult DestroyThread(void* td);
public:
	uint32_t m_idbase;
	CAWHashMapT<uint32_t, CFBPortThreadItem*> m_threadmap;
};


typedef struct thread_run_param
{
	void (*func)(void*);
	void (*init)(void*);
	void* arg;
	void* newtd;
} thread_run_param;

class CThreadDestroyEvent : public IAWEvent
{
public:
	CThreadDestroyEvent(void* newtd) {
		m_newtd = newtd;
	}
	virtual ~CThreadDestroyEvent()
	{

	}
	virtual CAWResult OnEventFire()
	{
		printf("CThreadDestroyEvent OnEventFire\n");
		return CFBPortThreadManager::Instance()->DestroyThread(m_newtd);
	}
	void* m_newtd;
};

class CThreadCreateEvent : public IAWEvent
{
public:
	CThreadCreateEvent(void (*func)(void*),
		void (*init)(void*),
		void* arg,
		void* newtd) {
		m_func = func;
		m_arg = arg;
		m_init = init;
		m_newtd = newtd;
	}
	virtual ~CThreadCreateEvent()
	{

	}
	virtual CAWResult OnEventFire()
	{
		printf("CThreadCreateEvent OnEventFire\n");
		return CFBPortThreadManager::Instance()->CreateThread(m_func, m_init, m_arg, m_newtd);
	}
	void (*m_func)(void*);
	void (*m_init)(void*);
	void* m_arg;
	void* m_newtd;
};

void thread_run_func(void* arg)
{
	thread_run_param* ppram = (thread_run_param*)arg;
	(ppram->init)(ppram->newtd);
	(ppram->func)(ppram->arg);
}

void thread_create(void (*func)(void*),
	void (*init)(void*),
	void* arg,
	void** threadid,
	void* newtd)
{
	printf("thread_create\n");
	CAWThread* pmaincurrent = CAWThreadManager::Instance()->GetThread(CAWThreadManager::TT_CURRENT);
	CAWThread* pmain = CAWThreadManager::Instance()->GetThread(CAWThreadManager::TT_MAIN);
	if (pmain == pmaincurrent)
	{
		CFBPortThreadManager::Instance()->CreateThread(func, init, arg, newtd);
	}
	else
	{
		CThreadCreateEvent* pevent = new CThreadCreateEvent(func, init, arg, newtd);
		pmaincurrent->PostEvent(pevent);
	}
}
void thread_destroy(void* threadid)
{
	printf("thread_destroy\n");
	CAWThread* pmaincurrent = CAWThreadManager::Instance()->GetThread(CAWThreadManager::TT_MAIN);
	if (pmaincurrent)
	{
		CThreadDestroyEvent* pevent = new CThreadDestroyEvent(threadid);
		pmaincurrent->PostEvent(pevent);
	}
}

CFBPortThreadItem::CFBPortThreadItem(void (*func)(void*),
	void (*init)(void*),
	void* arg,
	void* newtd,
	uint32_t id)
	:m_pEvent4Start(NULL)
{
	m_func = func;
	m_arg = arg;
	m_init = init;
	m_newtd = newtd;
	m_id = id;
}
CFBPortThreadItem::~CFBPortThreadItem()
{

}
void CFBPortThreadItem::OnThreadInit()
{
	printf("CFBPortThreadItem::OnThreadInit begin()=%p\n", m_newtd);
	(m_init)(m_newtd);
	printf("CFBPortThreadItem::OnThreadInit end()\n");
}
void CFBPortThreadItem::OnThreadRun()
{
	printf("CFBPortThreadItem::OnThreadRun begin()\n");
	m_func(m_arg);
	printf("CFBPortThreadItem::OnThreadRun end()\n");
}
DWORD CFBPortThreadItem::AddReference()
{
	return CAWReferenceControlMutilThread::AddReference();
}
DWORD CFBPortThreadItem::ReleaseReference()
{
	return CAWReferenceControlMutilThread::ReleaseReference();
}
#ifdef _WIN32
unsigned WINAPI CFBPortThreadItem::MainThreadFun(void* arg)
#else
void* CFBPortThreadItem::MainThreadFun(void* arg)
#endif // CAW_WIN32
{
	CFBPortThreadItem* pthreadmgr = (CFBPortThreadItem*)arg;
	CAW_ASSERTE(pthreadmgr->m_pEvent4Start);
	if (pthreadmgr->m_pEvent4Start)
		pthreadmgr->m_pEvent4Start->Signal();

#ifdef _WIN32
	if (pthreadmgr)
	{
		pthreadmgr->OnThreadInit();
		pthreadmgr->OnThreadRun();
	}
	return 0;
#else
	pthread_detach(pthread_self());
	if (pthreadmgr)
	{
		pthreadmgr->OnThreadInit();
		pthreadmgr->OnThreadRun();
	}
	pthread_exit(NULL);
#endif
}

CAWResult CFBPortThreadItem::StartThread()
{
	CAW_THREAD_ID tid;
	CAW_THREAD_HANDLE m_Handle;
	CAW_ASSERTE(!m_pEvent4Start);
	m_pEvent4Start = new CAWEventThread();
	if (!m_pEvent4Start)
	{
		return CAW_ERROR_OUT_OF_MEMORY;
	}
#ifdef CAW_WIN32
	m_Handle = (HANDLE)::_beginthreadex(
		NULL,
		0,
		MainThreadFun,
		this,
		0,
		(unsigned int*)(&tid));
	if (m_Handle == 0) {
		CAW_ERROR_TRACE("CAWThread::Create, _beginthreadex() failed! err=" << errno);
		return CAW_ERROR_UNEXPECTED;
	}
#else // !CAW_WIN32
	if (pthread_create(&tid, NULL, MainThreadFun, (void*)this) != 0) {
		fprintf(stderr, "thread create failed\n");
		return CAW_ERROR_FAILURE;
	}
#endif // CAW_WIN32
	m_pEvent4Start->Wait();
	delete m_pEvent4Start;
	m_pEvent4Start = NULL;

	return CAW_OK;
}
CFBPortThreadManager::CFBPortThreadManager()
	:m_idbase(0)
{

}
CFBPortThreadManager* CFBPortThreadManager::Instance()
{
	return CAWSingletonT< CFBPortThreadManager>::Instance();
}
CFBPortThreadManager::~CFBPortThreadManager()
{

}

CAWResult CFBPortThreadManager::AddThread(uint32_t id, CFBPortThreadItem* item)
{
	m_threadmap[id] = item;
	return CAW_OK;
}
CAWResult CFBPortThreadManager::RemoveThread(uint32_t id)
{
	m_threadmap.erase(id);
	return CAW_OK;
}
CAWResult CFBPortThreadManager::FindThread(uint32_t id, CFBPortThreadItem*& pthread)
{
	CAWHashMapT<uint32_t, CFBPortThreadItem*>::iterator it = m_threadmap.find(id);
	if (it == m_threadmap.end())
	{
		return CAW_ERROR_FAILURE;
	}
	CFBPortThreadItem *item = it->second;

	pthread = item;
	return CAW_OK;
}

CAWResult CFBPortThreadManager::FindThreadByTid(void* tid, CFBPortThreadItem*& pthread)
{
	CAWHashMapT<uint32_t, CFBPortThreadItem*>::iterator it = m_threadmap.begin();
	while (it != m_threadmap.end())
	{
		CFBPortThreadItem *item = it->second;
		if (item->GetTd() == tid)
		{
			pthread = item;
			return CAW_OK;
		}
		it++;
	}
	return CAW_ERROR_FAILURE;
}

CAWResult CFBPortThreadManager::CreateThread(void (*func)(void*),
	void (*init)(void*),
	void* arg,
	void* newtd)
{
	CFBPortThreadItem* poldthread = NULL;
	if (FindThreadByTid(newtd, poldthread) == CAW_OK)
	{
		return CAW_OK;
	}
	uint32_t id = m_idbase++;
	CFBPortThreadItem *newitem = new CFBPortThreadItem(func, init, arg, newtd, id);
	if (newitem)
	{
		newitem->StartThread();
		return AddThread(id, newitem);
	}
	else
	{
		return CAW_OK;
	}
}
CAWResult CFBPortThreadManager::DestroyThread(void* td)
{
	CFBPortThreadItem* pitem = NULL;
	if (FindThreadByTid(td, pitem) != CAW_OK)
	{
		return CAW_ERROR_FAILURE;
	}
	if (pitem)
	{
		RemoveThread(pitem->GetTD());
	}
	return CAW_OK;
}