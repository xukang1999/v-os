#ifndef _PORT_RAND_H
#define _PORT_RAND_H
#include "staros/so_init.h"
#ifdef __cplusplus
extern "C" {
#endif
SO_EXPORT void port_randbyte(void* ptr, unsigned int len);
#ifdef __cplusplus
}
#endif
#endif//_PORT_RAND_H