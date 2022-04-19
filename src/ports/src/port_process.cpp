#include "ports/port_process.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
int win_fork(void)
{
	return -1;
}
#endif
int port_fork(void)
{
#ifdef _WIN32
	return win_fork();
#else
	return fork();
#endif
}