#include "ports/port_sx_lock.h"
#include "staros/so_init.h"
#ifdef _WIN32
#include <Windows.h>
#endif
#include <iostream>
#include <mutex>    //unique_lock
#include <shared_mutex> //shared_mutex shared_lock
#include <thread>
#include "webrtc/base/sharedexclusivelock.h"

using namespace rtc;
void initialize_sx_lock(char** lock)
{
	SharedExclusiveLock* p = new SharedExclusiveLock();
	if (p == NULL)
	{
		return;
	}

	*lock = (char *)p;
}
void uninitialize_sx_lock(char* lock)
{
	SharedExclusiveLock* p = (SharedExclusiveLock*)lock;
	if (p)
		delete p;
}
void acquire_lock_exclusive(void* lock)
{
	SharedExclusiveLock* mutex_ = (SharedExclusiveLock*)lock;
	if (mutex_)
	{
		mutex_->LockExclusive();
	}
}
void release_lock_exclusive(void* lock)
{
	SharedExclusiveLock* mutex_ = (SharedExclusiveLock*)lock;
	if (mutex_)
	{
		mutex_->UnlockExclusive();
	}
}
void acquire_lock_shared(void* lock)
{
	SharedExclusiveLock* mutex_ = (SharedExclusiveLock*)lock;
	if (mutex_)
	{
		mutex_->LockShared();
	}
}
void release_lock_shared(void* lock)
{
	SharedExclusiveLock* mutex_ = (SharedExclusiveLock*)lock;
	if (mutex_)
	{
		mutex_->UnlockShared();
	}
}
