#include "ports/port_rw_lock.h"
#include <stdio.h>
#include <stdlib.h>
#include "webrtc/system_wrappers/include/rw_lock_wrapper.h"
#ifdef _WIN32
#include "win_pthread.h"
#else
#include <pthread.h>
#endif
using namespace webrtc;
void
port_rw_init(void** mutex)
{
	RWLockWrapper* p = RWLockWrapper::CreateRWLock();
	*mutex = p;
}

void
port_rw_rlock(void* mutex)
{
	RWLockWrapper* p = (RWLockWrapper*)mutex;
	p->AcquireLockShared();
}

void
port_rw_runlock(void* mutex)
{
	RWLockWrapper* p = (RWLockWrapper*)mutex;
	p->ReleaseLockShared();
}
void
port_rw_wlock(void* mutex)
{
	RWLockWrapper* p = (RWLockWrapper*)mutex;
	p->AcquireLockExclusive();
}

void
port_rw_wunlock(void* mutex)
{
	RWLockWrapper* p = (RWLockWrapper*)mutex;
	p->ReleaseLockExclusive();
}
void
port_rw_destroy(void* mutex)
{
	RWLockWrapper* p = (RWLockWrapper*)mutex;
	delete p;
}