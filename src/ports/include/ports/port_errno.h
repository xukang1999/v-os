#ifndef _PORT_ERRNO_H
#define _PORT_ERRNO_H
#include "staros/so_init.h"
#ifdef __cplusplus
extern "C" {
#endif
	SO_EXPORT void port_set_errno(int error);
	SO_EXPORT int port_get_errno();
#ifdef __cplusplus
}
#endif
#endif//_PORT_ERRNO_H