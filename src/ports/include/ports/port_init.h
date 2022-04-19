#ifndef PORTS_INIT_H
#define PORTS_INIT_H
#include "staros/so_export.h"
#include "ports/port_mmap.h"
#include "staros/staros.h"
#include "ports/port_atomic.h"
#include "ports/port_sx_lock.h"
#include "ports/port_stdout.h"
#include "ports/port_rw_lock.h"
#include "ports/port_thread.h"
#include "ports/port_time.h"
#include "ports/port_critical.h"
#include "ports/port_memory.h"
#include "ports/port_cpu.h"
#include "ports/port_rand.h"
#include "ports/port_process.h"
#include "ports/port_errno.h"
#ifdef __cplusplus
extern "C" {
#endif
SO_EXPORT void port_freebsd_init();
#ifdef __cplusplus
}
#endif

#endif//FREEBSD_PORTS_H