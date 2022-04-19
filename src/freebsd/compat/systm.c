#include <sys/cdefs.h>
#include <sys/types.h>
#include <machine/atomic.h>
#include <sys/systm.h>
#include <stdlib.h>
int	xos_sleep(const void* _Nonnull chan, struct lock_object* lock, int pri,
	const char* wmesg, sbintime_t sbt, sbintime_t pr, int flags)
{
	 _sleep(1000);
	 return 0;
}