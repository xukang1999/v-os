#include "ports/port_stdout.h"
#include <stdio.h>
#include <stdlib.h>

int port_putchar(int c)
{
	return putchar(c);
}