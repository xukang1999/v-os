#ifndef __PORT_CRITICAL_H
#define __PORT_CRITICAL_H
#include "staros/so_init.h"
#ifdef __cplusplus
extern "C" {
#endif
SO_EXPORT void port_critical_enter(void);
SO_EXPORT void port_critical_exit(void);
#ifdef __cplusplus
}
#endif
#endif//__PORT_CRITICAL_H