#include "ports/port_critical.h"
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

static CAWMutexThread m_glock;

void port_critical_enter(void)
{
	m_glock.Lock();
}
void port_critical_exit(void)
{
	m_glock.UnLock();
}