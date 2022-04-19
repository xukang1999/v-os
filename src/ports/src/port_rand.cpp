#include "ports/port_rand.h"
#include "vos/vos_time.h"
#include <openssl/rand.h>
void port_randbyte(void* ptr, unsigned int len)
{
	RAND_bytes((unsigned char* )ptr, len);
}