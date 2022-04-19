#ifndef CK_ATOMIC_H
#define CK_ATOMIC_H
#include <ck_stdbool.h>
#include <ck_stddef.h>

char ck_atomi_sync_fetch_and_add_char(char *ptr, char value);
char ck_atomi_sync_fetch_and_sub_char(char *ptr, char value);
char ck_atomi_sync_fetch_and_or_char(char *ptr, char value);
char ck_atomi_sync_fetch_and_and_char(char *ptr, char value);
char ck_atomi_sync_fetch_and_xor_char(char *ptr, char value);
bool ck_atomi_sync_bool_compare_and_swap_char(char* ptr, char compare, char set);
char ck_atomi_sync_val_compare_and_swap_char(char* ptr, char compare, char set);

unsigned int ck_atomi_sync_fetch_and_add_uint(unsigned int *ptr, unsigned int value);
unsigned int ck_atomi_sync_fetch_and_sub_uint(unsigned int *ptr, unsigned int value);
unsigned int ck_atomi_sync_fetch_and_or_uint(unsigned int *ptr, unsigned int value);
unsigned int ck_atomi_sync_fetch_and_and_uint(unsigned int *ptr, unsigned int value);
unsigned int ck_atomi_sync_fetch_and_xor_uint(unsigned int *ptr, unsigned int value);
bool ck_atomi_sync_bool_compare_and_swap_uint(unsigned int* ptr, unsigned int compare, unsigned int set);
unsigned int ck_atomi_sync_val_compare_and_swap_uint(unsigned int* ptr, unsigned int compare, unsigned int set);

int ck_atomi_sync_fetch_and_add_int(int *ptr, int value);
int ck_atomi_sync_fetch_and_sub_int(int *ptr, int value);
int ck_atomi_sync_fetch_and_or_int(int *ptr, int value);
int ck_atomi_sync_fetch_and_and_int(int *ptr, int value);
int ck_atomi_sync_fetch_and_xor_int(int *ptr, int value);
bool ck_atomi_sync_bool_compare_and_swap_int(int* ptr, int compare, int set);
int ck_atomi_sync_val_compare_and_swap_int(int* ptr, int compare, int set);

uint64_t ck_atomi_sync_fetch_and_add_64(uint64_t *ptr, uint64_t value);
uint64_t ck_atomi_sync_fetch_and_sub_64(uint64_t *ptr, uint64_t value);
uint64_t ck_atomi_sync_fetch_and_or_64(uint64_t *ptr, uint64_t value);
uint64_t ck_atomi_sync_fetch_and_and_64(uint64_t *ptr, uint64_t value);
uint64_t ck_atomi_sync_fetch_and_xor_64(uint64_t *ptr, uint64_t value);
bool ck_atomi_sync_bool_compare_and_swap_64(uint64_t* ptr, uint64_t compare, uint64_t set);
uint64_t ck_atomi_sync_val_compare_and_swap_64(uint64_t* ptr, uint64_t compare, uint64_t set);

uint32_t ck_atomi_sync_fetch_and_add_32(uint32_t *ptr, uint32_t value);
uint32_t ck_atomi_sync_fetch_and_sub_32(uint32_t *ptr, uint32_t value);
uint32_t ck_atomi_sync_fetch_and_or_32(uint32_t *ptr, uint32_t value);
uint32_t ck_atomi_sync_fetch_and_and_32(uint32_t *ptr, uint32_t value);
uint32_t ck_atomi_sync_fetch_and_xor_32(uint32_t *ptr, uint32_t value);
bool ck_atomi_sync_bool_compare_and_swap_32(uint32_t* ptr, uint32_t compare, uint32_t set);
uint32_t ck_atomi_sync_val_compare_and_swap_32(uint32_t* ptr, uint32_t compare, uint32_t set);

uint16_t ck_atomi_sync_fetch_and_add_16(uint16_t *ptr, uint16_t value);
uint16_t ck_atomi_sync_fetch_and_sub_16(uint16_t *ptr, uint16_t value);
uint16_t ck_atomi_sync_fetch_and_or_16(uint16_t *ptr, uint16_t value);
uint16_t ck_atomi_sync_fetch_and_and_16(uint16_t *ptr, uint16_t value);
uint16_t ck_atomi_sync_fetch_and_xor_16(uint16_t *ptr, uint16_t value);
bool ck_atomi_sync_bool_compare_and_swap_16(uint16_t* ptr, uint16_t compare, uint16_t set);
uint16_t ck_atomi_sync_val_compare_and_swap_16(uint16_t* ptr, uint16_t compare, uint16_t set);

uint8_t ck_atomi_sync_fetch_and_add_8(uint8_t *ptr, uint8_t value);
uint8_t ck_atomi_sync_fetch_and_sub_8(uint8_t *ptr, uint8_t value);
uint8_t ck_atomi_sync_fetch_and_or_8(uint8_t *ptr, uint8_t value);
uint8_t ck_atomi_sync_fetch_and_and_8(uint8_t *ptr, uint8_t value);
uint8_t ck_atomi_sync_fetch_and_xor_8(uint8_t *ptr, uint8_t value);
bool ck_atomi_sync_bool_compare_and_swap_8(uint8_t* ptr, uint8_t compare, uint8_t set);
uint8_t ck_atomi_sync_val_compare_and_swap_8(uint8_t* ptr, uint8_t compare, uint8_t set);

void * ck_atomi_sync_fetch_and_add_ptr(void *ptr, void * value);
void * ck_atomi_sync_fetch_and_sub_ptr(void *ptr, void * value);
void * ck_atomi_sync_fetch_and_or_ptr(void *ptr, void * value);
void * ck_atomi_sync_fetch_and_and_ptr(void *ptr, void * value);
void * ck_atomi_sync_fetch_and_xor_ptr(void *ptr, void * value);
bool ck_atomi_sync_bool_compare_and_swap_ptr(void* ptr, void* compare, void* set);
void* ck_atomi_sync_val_compare_and_swap_ptr(void* ptr, void* compare, void* set);
#endif