#ifndef _SO_API_H
#define _SO_API_H
#include "staros/so_init.h"

#ifdef __cplusplus
extern "C" {
#endif

SO_EXPORT int vos_init(void);
SO_EXPORT int vos_setenv(const char *name, const char *value);

#ifdef __cplusplus
}
#endif

#endif