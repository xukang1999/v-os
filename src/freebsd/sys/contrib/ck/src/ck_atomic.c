#include <ck_atomic.h>
#include "compat/context.h"

extern so_atomic_hooks global_atomic_hooks;


char ck_atomi_sync_fetch_and_add_char(char* ptr, char value)
{
	return global_atomic_hooks.atomic_fetchadd_char(ptr, value);
}
char ck_atomi_sync_fetch_and_sub_char(char *ptr, char value)
{
	return global_atomic_hooks.atomic_fetchsub_char(ptr, value);
}
char ck_atomi_sync_fetch_and_or_char(char *ptr, char value)
{
	return global_atomic_hooks.atomic_fetchor_char(ptr, value);
}
char ck_atomi_sync_fetch_and_and_char(char *ptr, char value)
{
	return global_atomic_hooks.atomic_fetchand_char(ptr, value);
}
char ck_atomi_sync_fetch_and_xor_char(char *ptr, char value)
{
	return global_atomic_hooks.atomic_fetchxor_char(ptr, value);
}

bool ck_atomi_sync_bool_compare_and_swap_char(char* ptr, char compare, char set)
{
	return global_atomic_hooks.sync_bool_compare_and_swap_char(ptr, compare,set);
}
char ck_atomi_sync_val_compare_and_swap_char(char* ptr, char compare, char set)
{
	return global_atomic_hooks.sync_val_compare_and_swap_char((u_char*)ptr, (u_char)compare, (u_char)set);
}

unsigned int ck_atomi_sync_fetch_and_add_uint(unsigned int *ptr, unsigned int value)
{
	return global_atomic_hooks.atomic_fetchadd_uint(ptr, value);
}
unsigned int ck_atomi_sync_fetch_and_sub_uint(unsigned int *ptr, unsigned int value)
{
	return global_atomic_hooks.atomic_fetchsub_uint(ptr, value);
}
unsigned int ck_atomi_sync_fetch_and_or_uint(unsigned int *ptr, unsigned int value)
{
	return global_atomic_hooks.atomic_fetchor_uint(ptr, value);
}
unsigned int ck_atomi_sync_fetch_and_and_uint(unsigned int *ptr, unsigned int value)
{
	return global_atomic_hooks.atomic_fetchand_uint(ptr, value);
}
unsigned int ck_atomi_sync_fetch_and_xor_uint(unsigned int *ptr, unsigned int value)
{
	return global_atomic_hooks.atomic_fetchxor_uint(ptr, value);
}

bool ck_atomi_sync_bool_compare_and_swap_uint(unsigned int* ptr, unsigned int compare, unsigned int set)
{
	return global_atomic_hooks.sync_bool_compare_and_swap_uint(ptr, compare, set);
}
unsigned int ck_atomi_sync_val_compare_and_swap_uint(unsigned int* ptr, unsigned int compare, unsigned int set)
{
	return global_atomic_hooks.sync_val_compare_and_swap_uint(ptr, compare, set);
}


int ck_atomi_sync_fetch_and_add_int(int *ptr, int value)
{
	return global_atomic_hooks.atomic_fetchand_int(ptr, value);
}
int ck_atomi_sync_fetch_and_sub_int(int *ptr, int value)
{
	return global_atomic_hooks.atomic_fetchsub_int(ptr, value);
}
int ck_atomi_sync_fetch_and_or_int(int *ptr, int value)
{
	return global_atomic_hooks.atomic_fetchor_int(ptr, value);
}
int ck_atomi_sync_fetch_and_and_int(int *ptr, int value)
{
	return global_atomic_hooks.atomic_fetchand_int(ptr, value);
}
int ck_atomi_sync_fetch_and_xor_int(int *ptr, int value)
{
	return global_atomic_hooks.atomic_fetchxor_int(ptr, value);
}
bool ck_atomi_sync_bool_compare_and_swap_int(int* ptr, int compare, int set)
{
	return global_atomic_hooks.sync_bool_compare_and_swap_int(ptr, compare, set);
}

int ck_atomi_sync_val_compare_and_swap_int(int* ptr, int compare, int set)
{
	return global_atomic_hooks.sync_val_compare_and_swap_int(ptr, compare, set);
}

uint64_t ck_atomi_sync_fetch_and_add_64(uint64_t *ptr, uint64_t value)
{
	return global_atomic_hooks.atomic_fetchadd_64_pfn(ptr, value);
}
uint64_t ck_atomi_sync_fetch_and_sub_64(uint64_t *ptr, uint64_t value)
{
	return global_atomic_hooks.atomic_fetchsub_64(ptr, value);
}
uint64_t ck_atomi_sync_fetch_and_or_64(uint64_t *ptr, uint64_t value)
{
	return global_atomic_hooks.atomic_fetchor_64(ptr, value);
}
uint64_t ck_atomi_sync_fetch_and_and_64(uint64_t *ptr, uint64_t value)
{
	return global_atomic_hooks.atomic_fetchand_64(ptr, value);
}
uint64_t ck_atomi_sync_fetch_and_xor_64(uint64_t *ptr, uint64_t value)
{
	return global_atomic_hooks.atomic_fetchxor_64(ptr, value);
}

bool ck_atomi_sync_bool_compare_and_swap_64(uint64_t* ptr, uint64_t compare, uint64_t set)
{
	return global_atomic_hooks.sync_bool_compare_and_swap_64(ptr, compare, set);
}
uint64_t ck_atomi_sync_val_compare_and_swap_64(uint64_t* ptr, uint64_t compare, uint64_t set)
{
	return global_atomic_hooks.sync_val_compare_and_swap_64(ptr, compare, set);
}

uint32_t ck_atomi_sync_fetch_and_add_32(uint32_t *ptr, uint32_t value)
{
	return global_atomic_hooks.atomic_fetchadd_32_pfn(ptr, value);
}
uint32_t ck_atomi_sync_fetch_and_sub_32(uint32_t *ptr, uint32_t value)
{
	return global_atomic_hooks.atomic_fetchand_32(ptr, value);
}
uint32_t ck_atomi_sync_fetch_and_or_32(uint32_t *ptr, uint32_t value)
{
	return global_atomic_hooks.atomic_fetchor_32(ptr, value);
}
uint32_t ck_atomi_sync_fetch_and_and_32(uint32_t *ptr, uint32_t value)
{
	return global_atomic_hooks.atomic_fetchand_32(ptr, value);
}
uint32_t ck_atomi_sync_fetch_and_xor_32(uint32_t *ptr, uint32_t value)
{
	return global_atomic_hooks.atomic_fetchxor_32(ptr, value);
}

bool ck_atomi_sync_bool_compare_and_swap_32(uint32_t* ptr, uint32_t compare, uint32_t set)
{
	return global_atomic_hooks.sync_bool_compare_and_swap_32(ptr, compare, set);
}
uint32_t ck_atomi_sync_val_compare_and_swap_32(uint32_t* ptr, uint32_t compare, uint32_t set)
{
	return global_atomic_hooks.sync_val_compare_and_swap_32(ptr, compare, set);
}
uint16_t ck_atomi_sync_fetch_and_add_16(uint16_t *ptr, uint16_t value)
{
	return global_atomic_hooks.atomic_fetchadd_16(ptr, value);
}
uint16_t ck_atomi_sync_fetch_and_sub_16(uint16_t *ptr, uint16_t value)
{
	return global_atomic_hooks.atomic_fetchsub_16(ptr, value);
}
uint16_t ck_atomi_sync_fetch_and_or_16(uint16_t *ptr, uint16_t value)
{
	return global_atomic_hooks.atomic_fetchor_16(ptr, value);
}
uint16_t ck_atomi_sync_fetch_and_and_16(uint16_t *ptr, uint16_t value)
{
	return global_atomic_hooks.atomic_fetchand_16(ptr, value);
}
uint16_t ck_atomi_sync_fetch_and_xor_16(uint16_t *ptr, uint16_t value)
{
	return global_atomic_hooks.atomic_fetchxor_16(ptr, value);
}

bool ck_atomi_sync_bool_compare_and_swap_16(uint16_t* ptr, uint16_t compare, uint16_t set)
{
	return global_atomic_hooks.sync_bool_compare_and_swap_16(ptr, compare, set);
}
uint16_t ck_atomi_sync_val_compare_and_swap_16(uint16_t* ptr, uint16_t compare, uint16_t set)
{
	return global_atomic_hooks.sync_val_compare_and_swap_16(ptr, compare, set);
}


uint8_t ck_atomi_sync_fetch_and_add_8(uint8_t *ptr, uint8_t value)
{
	return global_atomic_hooks.atomic_fetchadd_8(ptr, value);
}
uint8_t ck_atomi_sync_fetch_and_sub_8(uint8_t *ptr, uint8_t value)
{
	return global_atomic_hooks.atomic_fetchsub_8(ptr, value);
}
uint8_t ck_atomi_sync_fetch_and_or_8(uint8_t *ptr, uint8_t value)
{
	return global_atomic_hooks.atomic_fetchor_8(ptr, value);
}
uint8_t ck_atomi_sync_fetch_and_and_8(uint8_t *ptr, uint8_t value)
{
	return global_atomic_hooks.atomic_fetchand_8(ptr, value);
}
uint8_t ck_atomi_sync_fetch_and_xor_8(uint8_t *ptr, uint8_t value)
{
	return global_atomic_hooks.atomic_fetchxor_8(ptr, value);
}

bool ck_atomi_sync_bool_compare_and_swap_8(uint8_t* ptr, uint8_t compare, uint8_t set)
{
	return global_atomic_hooks.sync_bool_compare_and_swap_8(ptr, compare, set);
}
uint8_t ck_atomi_sync_val_compare_and_swap_8(uint8_t* ptr, uint8_t compare, uint8_t set)
{
	return global_atomic_hooks.sync_val_compare_and_swap_8(ptr, compare, set);
}
void * ck_atomi_sync_fetch_and_add_ptr(void *ptr, void * value)
{
	return global_atomic_hooks.atomic_fetchadd_ptr(ptr, value);
}
void * ck_atomi_sync_fetch_and_sub_ptr(void *ptr, void * value)
{
	return global_atomic_hooks.atomic_fetchsub_ptr(ptr, value);
}
void * ck_atomi_sync_fetch_and_or_ptr(void *ptr, void * value)
{
	return global_atomic_hooks.atomic_fetchor_ptr(ptr, value);
}
void * ck_atomi_sync_fetch_and_and_ptr(void *ptr, void * value)
{
	return global_atomic_hooks.atomic_fetchand_ptr(ptr, value);
}
void * ck_atomi_sync_fetch_and_xor_ptr(void *ptr, void * value)
{
	return global_atomic_hooks.atomic_fetchxor_ptr(ptr, value);
}
bool ck_atomi_sync_bool_compare_and_swap_ptr(void* ptr, void* compare, void* set)
{
	return global_atomic_hooks.sync_bool_compare_and_swap_ptr(ptr, compare, set);
}

void* ck_atomi_sync_val_compare_and_swap_ptr(void* ptr, void* compare, void* set)
{
	return global_atomic_hooks.sync_val_compare_and_swap_ptr(ptr, compare, set);
}