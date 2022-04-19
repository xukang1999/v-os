#include "ports/port_atomic.h"
#include "staros/so_init.h"
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <stddef.h>
#include <stdint.h>
#include <windows.h>
#endif

void atomic_set_int(u_int* P, u_int V)
{
	std::atomic<u_int> value(*P);
	value |= V;
	*P = value.load();
	/*(*(u_int*)(P) |= (V));*/
}
void atomic_clear_int(u_int* P, u_int V)
{
	std::atomic<u_int> value(*P);
	value &= ~(V);
	*P = value.load();
	/*(*(u_int*)(P) &= ~(V));*/
}

void atomic_add_int(u_int* P, u_int V) {
	std::atomic<u_int> value(*P);
	value += V;
	*P = value.load();
}
void atomic_subtract_int(u_int* P, u_int V)
{
	std::atomic<u_int> value(*P);
	value -= V;
	*P = value.load();
}
void atomic_add_long(unsigned long long* P, unsigned long long V) {
	std::atomic<unsigned long long> value(*P);
	value += V;
	*P = value.load();
}
void atomic_subtract_long(unsigned long long* P, unsigned long long V)
{
	std::atomic<unsigned long long> value(*P);
	value -= V;
	*P = value.load();
}
u_int atomic_fetchsub_int(volatile u_int* p, u_int v)
{
	std::atomic<u_int> value(*p);
	u_int retvalue = value.fetch_sub(v);
	*p = value.load();
	return retvalue;
}
unsigned long long	atomic_fetchsub_long(volatile unsigned long long* p, unsigned long long v)
{
	std::atomic<unsigned long long> value(*p);
	unsigned long long retvalue = value.fetch_sub(v);
	*p = value.load();
	return retvalue;
}
u_int atomic_fetchadd_int(volatile u_int* p, u_int v)
{
	std::atomic<u_int> value(*p);
	u_int retvalue= value.fetch_add(v);
	*p = value.load();
	return retvalue;
}
unsigned long long	atomic_fetchadd_long(volatile unsigned long long* p, unsigned long long v)
{
	std::atomic<unsigned long long> value(*p);
	unsigned long long retvalue = value.fetch_add(v);
	*p = value.load();
	return retvalue;
}

int	atomic_cmpset_char(volatile u_char* object, u_char expect, u_char desired)
{
	std::atomic<u_char> value(*object);

	bool issuccess = value.compare_exchange_strong(expect, desired);
	if (issuccess)
	{
		*object = value.load();
	}
	return issuccess;
}
int	atomic_cmpset_short(volatile u_short* dst, u_short expect, u_short desired)
{
	std::atomic<u_short> value(*dst);

	bool issuccess = value.compare_exchange_strong(expect, desired);
	if (issuccess)
	{
		*dst = value.load();
	}
	return issuccess;
}
int	atomic_cmpset_int(volatile u_int* dst, u_int expect, u_int desired)
{
	std::atomic<u_int> value(*dst);

	bool issuccess = value.compare_exchange_strong(expect, desired);
	if (issuccess)
	{
		*dst = value.load();
	}
	return issuccess;
}
int	atomic_cmpset_long(volatile unsigned long long* dst, unsigned long long expect, unsigned long long desired)
{
	std::atomic<unsigned long long> value(*dst);

	bool issuccess = value.compare_exchange_strong(expect, desired);
	if (issuccess)
	{
		*dst = value.load();
	}
	return issuccess;
}
int	atomic_fcmpset_char(volatile u_char* object, u_char* expected, u_char desired)
{
	std::atomic<u_char> value(*object);
	u_char oldexpected = *expected;
	bool issuccess = value.compare_exchange_strong(oldexpected, desired);
	if (issuccess == false)
	{
		*expected = *object;
	}
	else
	{
		*object = value.load();
	}
	return issuccess;
}
int	atomic_fcmpset_short(volatile u_short* object, u_short* expected, u_short desired)
{
	std::atomic<u_short> value(*object);
	u_short oldexpected = *expected;
	bool issuccess = value.compare_exchange_strong(oldexpected, desired);
	if (issuccess == false)
	{
		*expected = *object;
	}
	else
	{
		*object = value.load();
	}
	return issuccess;
}
int	atomic_fcmpset_int(volatile u_int* object, u_int* expected, u_int desired)
{
	std::atomic<u_int> value(*object);
	u_int oldexpected = *expected;
	bool issuccess = value.compare_exchange_strong(oldexpected, desired);
	if (issuccess == false)
	{
		*expected = *object;
	}
	else
	{
		*object = value.load();
	}
	return issuccess;
}
int	atomic_fcmpset_long(volatile unsigned long long* dst, unsigned long long* old, unsigned long long newvalue)
{
	std::atomic<unsigned long long> value(*dst);
	unsigned long long oldexpected = *old;
	bool issuccess=value.compare_exchange_weak(oldexpected, newvalue);
	if (issuccess == false)
	{
		*old = *dst;
	}
	else
	{
		*dst = value.load();
	}
	return issuccess;
#if 0
	if (*dst == *old) {
		*dst = newvalue;
		return (1);
	}
	else {
		*old = *dst;
		return (0);
	}
#endif
}
int	atomic_fcmpset_pointer(volatile unsigned long long* object, unsigned long long* expected, unsigned long long desired)
{
	std::atomic<unsigned long long> value(*object);
	unsigned long long oldexpected = *expected;
	bool issuccess = value.compare_exchange_strong(oldexpected, desired);
	if (issuccess == false)
	{
		*expected = *object;
	}
	else
	{
		*object = value.load();
	}
	return issuccess;
}

void
atomic_thread_fence_acq(void)
{
	std::atomic_thread_fence(std::memory_order_acquire);
}

void
atomic_thread_fence_rel(void)
{
	std::atomic_thread_fence(std::memory_order_release);

}

void
atomic_thread_fence_acq_rel(void)
{

	std::atomic_thread_fence(std::memory_order_acq_rel);
}

void
atomic_thread_fence_seq_cst(void)
{
	std::atomic_thread_fence(std::memory_order_seq_cst);

}
void atomic_add_barr_long(unsigned long long* P, unsigned long long V)
{
	std::atomic<unsigned long long> value(*P);
	value += V;
	*P = value.load(std::memory_order_acquire);
}
void atomic_add_barr_int(u_int* P, u_int V) {
	std::atomic<u_int> value(*P);
	value += V;
	*P = value.load(std::memory_order_acquire);
}
void
atomic_store_rel_int(volatile u_int* p, u_int v)
{
	std::atomic<u_int> value(*p);
	value.store(v, std::memory_order_release);
	*p = value.load();
}
void
atomic_store_rel_long(volatile unsigned long long* p, unsigned long long v)
{
	std::atomic<unsigned long long> value(*p);
	value.store(v, std::memory_order_release);
	*p = value.load();
}
unsigned long long
atomic_load_acq_long(volatile unsigned long long* p)
{
	std::atomic<unsigned long long> value(*p);
	return value.load(std::memory_order_acquire);
}
u_int
atomic_load_acq_int(volatile u_int* p)
{
	std::atomic<u_int> value(*p);
	return value.load(std::memory_order_acquire);
}
void atomic_subtract_barr_int(volatile u_int* p, u_int val)
{
	std::atomic<u_int> value(*p);
	value = value - val;
	*p = value.load(std::memory_order_acquire);
}

u_int atomic_swap_int(volatile u_int* p, u_int v)
{
	std::atomic<u_int> value(*p);
	u_int returnvalue = value.exchange(v);
	*p = value.load();
	return returnvalue;
}
unsigned long long atomic_swap_long(volatile unsigned long long* p, unsigned long long v)
{
	std::atomic<unsigned long long> value(*p);
	unsigned long long returnvalue = value.exchange(v);
	*p = value.load();
	return returnvalue;
}
void atomic_set_long(volatile unsigned long long* p, unsigned long long v)
{
	std::atomic<unsigned long long> value(*p);
	value |= v;
	*p = value.load();
}
void atomic_clear_long(unsigned long long* p, unsigned long long v)
{
	std::atomic<unsigned long long> value(*p);
	value &= ~(v);
	*p = value.load();
}
void atomic_set_char(u_char* P, u_char V)
{
	std::atomic<u_char> value(*P);
	value |= V;
	*P = value.load();
}
void atomic_clear_char(u_char* P, u_char V)
{
	std::atomic<u_char> value(*P);
	value &= ~(V);
	*P = value.load();
}
void atomic_subtract_barr_long(volatile unsigned long long* p, unsigned long long val)
{
	std::atomic<unsigned long long> value(*p);
	value = value - val;
	*p=value.load(std::memory_order_acquire);
}


char sync_atomic_fetchadd_char(volatile char* p, char v)
{
	std::atomic<char> value(*p);
	char nret = value.fetch_add(v);
	*p = value.load();
	return nret;
}
int sync_atomic_fetchadd_int(volatile int* p, int v) {
	std::atomic<int> value(*p);
	int nret = value.fetch_add(v);
	*p = value.load();
	return nret;
}
u_int sync_atomic_fetchadd_uint(volatile u_int* p, u_int v) {
	std::atomic<u_int> value(*p);
	u_int nret = value.fetch_add(v);
	*p = value.load();
	return nret;
}
unsigned long long sync_atomic_fetchadd_long(volatile unsigned long long* p, unsigned long long v) {
	std::atomic<unsigned long long> value(*p);
	unsigned long long nret = value.fetch_add(v);
	*p = value.load();
	return nret;
}
uint8_t sync_atomic_fetchadd_8(volatile uint8_t* p, uint8_t v) {
	std::atomic<uint8_t> value(*p);
	uint8_t nret = value.fetch_add(v);
	*p = value.load();
	return nret;
}
uint16_t sync_atomic_fetchadd_16(volatile uint16_t* p, uint16_t v) {
	std::atomic<uint16_t> value(*p);
	uint16_t nret = value.fetch_add(v);
	*p = value.load();
	return nret;
}
uint32_t sync_atomic_fetchadd_32_pfn(volatile uint32_t* p, uint32_t v) {
	std::atomic<uint32_t> value(*p);
	uint32_t nret = value.fetch_add(v);
	*p = value.load();
	return nret;
}
uint64_t sync_atomic_fetchadd_64_pfn(volatile uint64_t* p, uint64_t v) {
	std::atomic<uint64_t> value(*p);
	uint64_t nret = value.fetch_add(v);
	*p = value.load();
	return nret;
}
void* sync_atomic_fetchadd_ptr(volatile void* p, void* v) {
	std::atomic<uint64_t> value((uint64_t) & p);
	uint64_t nret=value.fetch_add((uint64_t)v);

	*(uint64_t *)p = value.load();
	return (void *)nret;
}

char sync_atomic_fetchsub_char(volatile char* p, char v) {
	std::atomic<char> value(*p);
	char nret = value.fetch_sub(v);
	*p = value.load();
	return nret;
}
int sync_atomic_fetchsub_int(volatile int* p, int v) {
	std::atomic<int> value(*p);
	int nret = value.fetch_sub(v);
	*p = value.load();
	return nret;
}
u_int sync_atomic_fetchsub_uint(volatile u_int* p, u_int v) {
	std::atomic<u_int> value(*p);
	u_int nret = value.fetch_sub(v);
	*p = value.load();
	return nret;
}
unsigned long long sync_atomic_fetchsub_long(volatile unsigned long long* p, unsigned long long v) {
	std::atomic<unsigned long long> value(*p);
	unsigned long long nret = value.fetch_sub(v);
	*p = value.load();
	return nret;
}
uint8_t sync_atomic_fetchsub_8(volatile uint8_t* p, uint8_t v) {
	std::atomic<uint8_t> value(*p);
	uint8_t nret = value.fetch_sub(v);
	*p = value.load();
	return nret;
}
uint16_t sync_atomic_fetchsub_16(volatile uint16_t* p, uint16_t v) {
	std::atomic<uint16_t> value(*p);
	uint16_t nret = value.fetch_sub(v);
	*p = value.load();
	return nret;
}
uint32_t sync_atomic_fetchsub_32_pfn(volatile uint32_t* p, uint32_t v) {
	std::atomic<uint32_t> value(*p);
	uint32_t nret = value.fetch_sub(v);
	*p = value.load();
	return nret;
}
uint64_t sync_atomic_fetchsub_64_pfn(volatile uint64_t* p, uint64_t v) {
	std::atomic<uint64_t> value(*p);
	uint64_t nret = value.fetch_sub(v);
	*p = value.load();
	return nret;
}
void* sync_atomic_fetchsub_ptr(volatile void* p, void* v) {
	std::atomic<uint64_t> value((uint64_t)&p);
	uint64_t nret = value.fetch_sub((uint64_t)v);

	*(uint64_t*)p = value.load();
	return (void*)nret;
}


char sync_atomic_fetchand_char(volatile char* p, char v) {
	std::atomic<char> value(*p);
	char nret = value.fetch_and(v);
	*p = value.load();
	return nret;
}
int sync_atomic_fetchand_int(volatile int* p, int v) {
	std::atomic<int> value(*p);
	int nret = value.fetch_and(v);
	*p = value.load();
	return nret;
}
u_int sync_atomic_fetchand_uint(volatile u_int* p, u_int v) {
	std::atomic<u_int> value(*p);
	u_int nret = value.fetch_and(v);
	*p = value.load();
	return nret;
}
unsigned long long sync_atomic_fetchand_long(volatile unsigned long long* p, unsigned long long v) {
	std::atomic<unsigned long long> value(*p);
	unsigned long long nret = value.fetch_and(v);
	*p = value.load();
	return nret;
}
uint8_t sync_atomic_fetchand_8(volatile uint8_t* p, uint8_t v) {
	std::atomic<uint8_t> value(*p);
	uint8_t nret = value.fetch_and(v);
	*p = value.load();
	return nret;
}
uint16_t sync_atomic_fetchand_16(volatile uint16_t* p, uint16_t v) {
	std::atomic<uint16_t> value(*p);
	uint16_t nret = value.fetch_and(v);
	*p = value.load();
	return nret;
}
uint32_t sync_atomic_fetchand_32_pfn(volatile uint32_t* p, uint32_t v) {
	std::atomic<uint32_t> value(*p);
	uint32_t nret = value.fetch_and(v);
	*p = value.load();
	return nret;
}
uint64_t sync_atomic_fetchand_64_pfn(volatile uint64_t* p, uint64_t v) {
	std::atomic<uint64_t> value(*p);
	uint64_t nret = value.fetch_and(v);
	*p = value.load();
	return nret;
}
void* sync_atomic_fetchand_ptr(volatile void* p, void* v) {
	std::atomic<uint64_t> value((uint64_t)&p);
	uint64_t nret = value.fetch_and((uint64_t)v);

	*(uint64_t*)p = value.load();
	return (void*)nret;
}

char sync_atomic_fetchor_char(volatile char* p, char v) {
	std::atomic<char> value(*p);
	char nret = value.fetch_or(v);
	*p = value.load();
	return nret;
}
int sync_atomic_fetchor_int(volatile int* p, int v) {
	std::atomic<int> value(*p);
	int nret = value.fetch_or(v);
	*p = value.load();
	return nret;
}
u_int sync_atomic_fetchor_uint(volatile u_int* p, u_int v) {
	std::atomic<u_int> value(*p);
	u_int nret = value.fetch_or(v);
	*p = value.load();
	return nret;
}
unsigned long long sync_atomic_fetchor_long(volatile unsigned long long* p, unsigned long long v) {
	std::atomic<unsigned long long> value(*p);
	unsigned long long nret = value.fetch_or(v);
	*p = value.load();
	return nret;
}
uint8_t sync_atomic_fetchor_8(volatile uint8_t* p, uint8_t v) {
	std::atomic<uint8_t> value(*p);
	uint8_t nret = value.fetch_or(v);
	*p = value.load();
	return nret;
}
uint16_t sync_atomic_fetchor_16(volatile uint16_t* p, uint16_t v) {
	std::atomic<uint16_t> value(*p);
	uint16_t nret = value.fetch_or(v);
	*p = value.load();
	return nret;
}
uint32_t sync_atomic_fetchor_32_pfn(volatile uint32_t* p, uint32_t v) {
	std::atomic<uint32_t> value(*p);
	uint32_t nret = value.fetch_or(v);
	*p = value.load();
	return nret;
}
uint64_t sync_atomic_fetchor_64_pfn(volatile uint64_t* p, uint64_t v) {
	std::atomic<uint64_t> value(*p);
	uint64_t nret = value.fetch_or(v);
	*p = value.load();
	return nret;
}
void* sync_atomic_fetchor_ptr(volatile void* p, void* v) {
	std::atomic<uint64_t> value((uint64_t)&p);
	uint64_t nret = value.fetch_or((uint64_t)v);

	*(uint64_t*)p = value.load();
	return (void*)nret;
}

char sync_atomic_fetchxor_char(volatile char* p, char v) {
	std::atomic<char> value(*p);
	char nret = value.fetch_xor(v);
	*p = value.load();
	return nret;
}
int sync_atomic_fetchxor_int(volatile int* p, int v) {
	std::atomic<int> value(*p);
	int nret = value.fetch_xor(v);
	*p = value.load();
	return nret;
}
u_int sync_atomic_fetchxor_uint(volatile u_int* p, u_int v) {
	std::atomic<u_int> value(*p);
	u_int nret = value.fetch_xor(v);
	*p = value.load();
	return nret;
}
unsigned long long sync_atomic_fetchxor_long(volatile unsigned long long* p, unsigned long long v) {
	std::atomic<unsigned long long> value(*p);
	unsigned long long nret = value.fetch_xor(v);
	*p = value.load();
	return nret;
}
uint8_t sync_atomic_fetchxor_8(volatile uint8_t* p, uint8_t v) {
	std::atomic<uint8_t> value(*p);
	uint8_t nret = value.fetch_xor(v);
	*p = value.load();
	return nret;
}
uint16_t sync_atomic_fetchxor_16(volatile uint16_t* p, uint16_t v) {
	std::atomic<uint16_t> value(*p);
	uint16_t nret = value.fetch_xor(v);
	*p = value.load();
	return nret;
}
uint32_t sync_atomic_fetchxor_32_pfn(volatile uint32_t* p, uint32_t v) {
	std::atomic<uint32_t> value(*p);
	uint32_t nret = value.fetch_xor(v);
	*p = value.load();
	return nret;
}
uint64_t sync_atomic_fetchxor_64_pfn(volatile uint64_t* p, uint64_t v) {
	std::atomic<uint64_t> value(*p);
	uint64_t nret = value.fetch_xor(v);
	*p = value.load();
	return nret;
}
void* sync_atomic_fetchxor_ptr(volatile void* p, void* v) {
	std::atomic<uint64_t> value((uint64_t)&p);
	uint64_t nret = value.fetch_xor((uint64_t)v);

	*(uint64_t*)p = value.load();
	return (void*)nret;
}

int sync_bool_compare_and_swap_char(char* ptr, char compare, char set) {
	std::atomic<char> value(*ptr);
	bool isok = value.compare_exchange_strong(compare, set);
	*ptr = value.load();
	return isok;
}
char sync_val_compare_and_swap_char(char* ptr, char compare, char set) {
	std::atomic<uint64_t> value(*ptr);
	uint64_t ccompare = (uint64_t)compare;
	uint64_t nset = (uint64_t)set;
	bool isok = value.compare_exchange_strong(ccompare, nset);
	*((uint64_t*)ptr) = (uint64_t)value.load();
	return isok;
}
int sync_bool_compare_and_swap_int(int* ptr, int compare, int set) {
	std::atomic<int> value(*ptr);
	bool isok = value.compare_exchange_strong(compare, set);
	*ptr = value.load();
	return isok;
}
char sync_val_compare_and_swap_int(int* ptr, int compare, int set) {
	std::atomic<int> value(*ptr);
	int oldvalue = *ptr;
	value.compare_exchange_strong(compare, set);
	*ptr = value.load();
	return oldvalue;
}
int sync_bool_compare_and_swap_uint(unsigned int* ptr, unsigned int compare, unsigned int set) {
	std::atomic<unsigned int> value(*ptr);
	bool isok = value.compare_exchange_strong(compare, set);
	*ptr = value.load();
	return isok;
}
unsigned int sync_val_compare_and_swap_uint(unsigned int* ptr, unsigned int compare, unsigned int set) {
	std::atomic<unsigned int> value(*ptr);
	unsigned int oldvalue = *ptr;
	value.compare_exchange_strong(compare, set);
	*ptr = value.load();
	return oldvalue;
}
int sync_bool_compare_and_swap_8(uint8_t* ptr, uint8_t compare, uint8_t set) {
	std::atomic<uint8_t> value(*ptr);
	bool isok = value.compare_exchange_strong(compare, set);
	*ptr = value.load();
	return isok;
}
uint8_t sync_val_compare_and_swap_8(uint8_t* ptr, uint8_t compare, uint8_t set) {
	std::atomic<uint8_t> value(*ptr);
	uint8_t oldvalue = *ptr;
	value.compare_exchange_strong(compare, set);
	*ptr = value.load();
	return oldvalue;
}
int sync_bool_compare_and_swap_16(uint16_t* ptr, uint16_t compare, uint16_t set) {
	std::atomic<uint16_t> value(*ptr);
	bool isok = value.compare_exchange_strong(compare, set);
	*ptr = value.load();
	return isok;
}
uint16_t sync_val_compare_and_swap_16(uint16_t* ptr, uint16_t compare, uint16_t set) {
	std::atomic<uint16_t> value(*ptr);
	uint16_t oldvalue = *ptr;
	value.compare_exchange_strong(compare, set);
	*ptr = value.load();
	return oldvalue;
}
int sync_bool_compare_and_swap_32(uint32_t* ptr, uint32_t compare, uint32_t set) {
	std::atomic<uint32_t> value(*ptr);
	bool isok = value.compare_exchange_strong(compare, set);
	*ptr = value.load();
	return isok;
}
uint32_t sync_val_compare_and_swap_32(uint32_t* ptr, uint32_t compare, uint32_t set) {
	std::atomic<uint32_t> value(*ptr);
	uint32_t oldvalue = *ptr;
	value.compare_exchange_strong(compare, set);
	*ptr = value.load();
	return oldvalue;
}
int sync_bool_compare_and_swap_64(uint64_t* ptr, uint64_t compare, uint64_t set) {
	std::atomic<uint64_t> value(*ptr);
	bool isok = value.compare_exchange_strong(compare, set);
	*ptr = value.load();
	return isok;
}
uint64_t sync_val_compare_and_swap_64(uint64_t* ptr, uint64_t compare, uint64_t set) {
	std::atomic<uint64_t> value(*ptr);
	uint64_t oldvalue = *ptr;
	value.compare_exchange_strong(compare, set);
	*ptr = value.load();
	return oldvalue;
}
int  sync_bool_compare_and_swap_ptr(void* ptr, void* compare, void* set) {
	std::atomic<uint64_t> value((uint64_t)(&ptr));
	uint64_t ccompare = (uint64_t)compare;
	uint64_t nset = (uint64_t)set;
	bool isok = value.compare_exchange_strong(ccompare, nset);
	*((uint64_t *)ptr) = (uint64_t)value.load();
	return isok;
}
void* sync_val_compare_and_swap_ptr(void* ptr, void* compare, void* set) {
	return (void *)sync_val_compare_and_swap_64((uint64_t*)ptr, (uint64_t)compare, (uint64_t)set);
}