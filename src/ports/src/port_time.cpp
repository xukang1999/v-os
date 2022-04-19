#include "ports/port_time.h"
#include "vos/vos_time.h"
void port_mssleep(int msecs)
{
	VOS_SleepMs(msecs);
}

void port_ussleep(uint64_t usecs)
{
	VOS_USleep((time_t)usecs);
}

void port_second_sleep(uint32_t secs)
{
	return VOS_SleepMs(secs*1000);
}