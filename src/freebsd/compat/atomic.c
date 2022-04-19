#include <sys/cdefs.h>
#include <sys/types.h>
#include <machine/atomic.h>
#include "context.h"

extern so_atomic_hooks global_atomic_hooks;

int
atomic_testandset_long(volatile u_long *p, u_int v)
{
	u_long bit, old;
	bool ret;

	bit = (1ull << (v % (sizeof(*p) * 8)));

	old = atomic_load_long(p);
	ret = false;
	while (!ret && (old & bit) == 0)
		ret = atomic_fcmpset_long(p, &old, old | bit);

	return (!ret);
}


int
atomic_testandclear_long(volatile u_long *p, u_int v)
{
	u_long bit, old;
	bool ret;

	bit = (1ull << (v % (sizeof(*p) * 8)));

	old = atomic_load_long(p);
	ret = false;
	while (!ret && (old & bit) != 0)
		ret = atomic_fcmpset_long(p, &old, old & ~bit);

	return (ret);
}
void atomic_add_barr_int(volatile u_int* P, u_int V)
{
	global_atomic_hooks.atomic_add_barr_int(P, V);
}
void atomic_add_barr_long(volatile u_long* P, u_long V)
{
	global_atomic_hooks.atomic_add_barr_long(P, V);
}
void atomic_set_int(volatile u_int* P, u_int V)
{
	global_atomic_hooks.atomic_set_int(P, V);
}
void atomic_clear_int(volatile u_int* P, u_int V)
{
	global_atomic_hooks.atomic_clear_int(P, V);
}

void atomic_add_int(volatile u_int* P, u_int V) {
	global_atomic_hooks.atomic_add_int(P, V);
}
void atomic_subtract_int(volatile u_int* P, u_int V)
{
	global_atomic_hooks.atomic_subtract_int(P, V);
}
void atomic_add_long(volatile u_long* P, u_long V) {
	global_atomic_hooks.atomic_add_long(P, V);
}
void atomic_subtract_long(volatile u_long* P, u_long V)
{
	global_atomic_hooks.atomic_subtract_long(P, V);
}

u_int atomic_fetchadd_int(volatile u_int* p, u_int v)
{
	return 	global_atomic_hooks.atomic_fetchadd_int(p, v);
}
u_long	atomic_fetchadd_long(volatile u_long* p, u_long v)
{
	return 	global_atomic_hooks.atomic_fetchadd_long(p, v);
}

u_int	atomic_fetchsub_int(volatile u_int* p, u_int v)
{
	return 	global_atomic_hooks.atomic_fetchsub_int(p, v);
}
u_long	atomic_fetchsub_long(volatile u_long* p, u_long v)
{
	return 	global_atomic_hooks.atomic_fetchsub_long(p, v);
}

int	atomic_cmpset_char(volatile u_char* object, u_char expect, u_char desired)
{
	return 	global_atomic_hooks.atomic_cmpset_char(object, expect,desired);
}
int	atomic_cmpset_short(volatile u_short* dst, u_short expect, u_short desired)
{
	return 	global_atomic_hooks.atomic_cmpset_short(dst, expect, desired);
}
int	atomic_cmpset_int(volatile u_int* dst, u_int expect, u_int desired)
{
	return 	global_atomic_hooks.atomic_cmpset_int(dst, expect, desired);
}
int	atomic_cmpset_long(volatile u_long* dst, u_long expect, u_long desired)
{
	return 	global_atomic_hooks.atomic_cmpset_long(dst, expect, desired);
}
int	atomic_fcmpset_char(volatile u_char* object, u_char* expected, u_char desired)
{
	return 	global_atomic_hooks.atomic_fcmpset_char(object, expected, desired);
}
int	atomic_fcmpset_short(volatile u_short* object, u_short* expected, u_short desired)
{
	return 	global_atomic_hooks.atomic_fcmpset_short(object, expected, desired);
}
int	atomic_fcmpset_int(volatile u_int* object, u_int* expected, u_int desired)
{
	return 	global_atomic_hooks.atomic_fcmpset_int(object, expected, desired);
}
int	atomic_fcmpset_long(volatile u_long* object, u_long* expected, u_long desired)
{
	return 	global_atomic_hooks.atomic_fcmpset_long(object, expected, desired);
}

void
atomic_thread_fence_acq(void)
{

	global_atomic_hooks.atomic_thread_fence_acq();
}

void
atomic_thread_fence_rel(void)
{

	global_atomic_hooks.atomic_thread_fence_rel();
}

void
atomic_thread_fence_acq_rel(void)
{
	global_atomic_hooks.atomic_thread_fence_acq_rel();

}

void
atomic_thread_fence_seq_cst(void)
{

	global_atomic_hooks.atomic_thread_fence_seq_cst();
}

void
atomic_store_rel_int(volatile u_int* p, u_int v)
{
	global_atomic_hooks.atomic_store_rel_int(p, v);
}
void
atomic_store_rel_long(volatile u_long* p, u_long v)
{
	global_atomic_hooks.atomic_store_rel_long(p, v);
}
u_long
atomic_load_acq_long(volatile u_long* p)
{
	return global_atomic_hooks.atomic_load_acq_long(p);
}
u_int
atomic_load_acq_int(volatile u_int* p)
{
	return global_atomic_hooks.atomic_load_acq_int(p);
}
void atomic_subtract_barr_int(volatile u_int* p, u_int val)
{
	global_atomic_hooks.atomic_subtract_barr_int(p,val);
}

u_int	atomic_swap_int(volatile u_int* p, u_int v)
{
	return global_atomic_hooks.atomic_swap_int(p,v);
}
u_long	atomic_swap_long(volatile u_long* p, u_long v)
{
	return global_atomic_hooks.atomic_swap_long(p,v);
}
void
atomic_set_long(volatile u_long* p, u_long v)
{
	global_atomic_hooks.atomic_set_long(p, v);
}
void atomic_clear_long(volatile u_long* p, u_long v)
{
	global_atomic_hooks.atomic_clear_long(p, v);
}
void atomic_set_char(volatile u_char* P, u_char V)
{
	global_atomic_hooks.atomic_set_char(P, V);
}
void atomic_clear_char(volatile u_char* P, u_char V)
{
	global_atomic_hooks.atomic_clear_char(P, V);
}
void atomic_subtract_barr_long(volatile u_long* p, u_long val)
{
	global_atomic_hooks.atomic_subtract_barr_long(p, val);
}
