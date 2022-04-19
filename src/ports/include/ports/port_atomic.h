#ifndef INTERNAL_ATOMIC_C11_H
#define INTERNAL_ATOMIC_C11_H
#include "staros/so_init.h"
#ifdef __cplusplus
extern "C" {
#endif
SO_EXPORT void atomic_add_barr_int(u_int* P, u_int V);
SO_EXPORT void atomic_add_barr_long(unsigned long long* P, unsigned long long V);

SO_EXPORT void atomic_set_int(u_int* P, u_int V);
SO_EXPORT void atomic_clear_int(u_int* P, u_int V);
SO_EXPORT void atomic_add_int(u_int* P, u_int V);
SO_EXPORT void atomic_subtract_int(u_int* P, u_int V);
SO_EXPORT void atomic_add_long(unsigned long long* P, unsigned long long V);
SO_EXPORT void atomic_subtract_long(unsigned long long* P, unsigned long long V);

SO_EXPORT int	atomic_cmpset_char(volatile u_char* object, u_char expect, u_char desired);
SO_EXPORT int	atomic_cmpset_short(volatile u_short* dst, u_short expect, u_short desired);
SO_EXPORT int	atomic_cmpset_int(volatile u_int* dst, u_int expect, u_int desired);
SO_EXPORT int	atomic_cmpset_long(volatile unsigned long long* dst, unsigned long long expect, unsigned long long desired);
SO_EXPORT int	atomic_fcmpset_char(volatile u_char* object, u_char* expected, u_char desired);
SO_EXPORT int	atomic_fcmpset_short(volatile u_short* object, u_short* expected, u_short desired);
SO_EXPORT int	atomic_fcmpset_int(volatile u_int* object, u_int* expected, u_int desired);
SO_EXPORT int	atomic_fcmpset_long(volatile unsigned long long* object, unsigned long long* expected, unsigned long long desired);
SO_EXPORT int	atomic_fcmpset_pointer(volatile unsigned long long* object, unsigned long long* expected, unsigned long long desired);
SO_EXPORT void atomic_thread_fence_acq(void);
SO_EXPORT void atomic_thread_fence_rel(void);
SO_EXPORT void atomic_thread_fence_acq_rel(void);
SO_EXPORT void atomic_thread_fence_seq_cst(void);
SO_EXPORT void atomic_add_barr_int(u_int* P, u_int V);
SO_EXPORT void atomic_add_barr_long(unsigned long long* P, unsigned long long V);
SO_EXPORT void atomic_store_rel_int(volatile u_int* p, u_int v);
SO_EXPORT void atomic_store_rel_long(volatile unsigned long long* p, unsigned long long v);
SO_EXPORT unsigned long long atomic_load_acq_long(volatile unsigned long long* p);
SO_EXPORT u_int atomic_load_acq_int(volatile u_int* p);
SO_EXPORT void atomic_subtract_barr_int(volatile u_int* p, u_int val);
SO_EXPORT u_int atomic_swap_int(volatile u_int* p, u_int v);
SO_EXPORT unsigned long long atomic_swap_long(volatile unsigned long long* p, unsigned long long v);
SO_EXPORT void  atomic_set_long(volatile unsigned long long* p, unsigned long long v);
SO_EXPORT void atomic_clear_long(unsigned long long* p, unsigned long long v);
SO_EXPORT void atomic_set_char(u_char* P, u_char V);
SO_EXPORT void atomic_clear_char(u_char* P, u_char V);
SO_EXPORT void atomic_subtract_barr_long(volatile unsigned long long* p, unsigned long long val);

/*sync atomi*/
SO_EXPORT char sync_atomic_fetchadd_char(volatile char* p, char v);
SO_EXPORT int sync_atomic_fetchadd_int(volatile int* p, int v);
SO_EXPORT u_int sync_atomic_fetchadd_uint(volatile u_int* p, u_int v);
SO_EXPORT unsigned long long sync_atomic_fetchadd_long(volatile unsigned long long* p, unsigned long long v);
SO_EXPORT uint8_t sync_atomic_fetchadd_8(volatile uint8_t* p, uint8_t v);
SO_EXPORT uint16_t sync_atomic_fetchadd_16(volatile uint16_t* p, uint16_t v);
SO_EXPORT uint32_t sync_atomic_fetchadd_32_pfn(volatile uint32_t* p, uint32_t v);
SO_EXPORT uint64_t sync_atomic_fetchadd_64_pfn(volatile uint64_t* p, uint64_t v);
SO_EXPORT void* sync_atomic_fetchadd_ptr(volatile void* p, void* v);

SO_EXPORT char sync_atomic_fetchsub_char(volatile char* p, char v);
SO_EXPORT int sync_atomic_fetchsub_int(volatile int* p, int v);
SO_EXPORT u_int sync_atomic_fetchsub_uint(volatile u_int* p, u_int v);
SO_EXPORT unsigned long long sync_atomic_fetchsub_long(volatile unsigned long long* p, unsigned long long v);
SO_EXPORT uint8_t sync_atomic_fetchsub_8(volatile uint8_t* p, uint8_t v);
SO_EXPORT uint16_t sync_atomic_fetchsub_16(volatile uint16_t* p, uint16_t v);
SO_EXPORT uint32_t sync_atomic_fetchsub_32_pfn(volatile uint32_t* p, uint32_t v);
SO_EXPORT uint64_t sync_atomic_fetchsub_64_pfn(volatile uint64_t* p, uint64_t v);
SO_EXPORT void* sync_atomic_fetchsub_ptr(volatile void* p, void* v);


SO_EXPORT char sync_atomic_fetchand_char(volatile char* p, char v);
SO_EXPORT int sync_atomic_fetchand_int(volatile int* p, int v);
SO_EXPORT u_int sync_atomic_fetchand_uint(volatile u_int* p, u_int v);
SO_EXPORT unsigned long long sync_atomic_fetchand_long(volatile unsigned long long* p, unsigned long long v);
SO_EXPORT uint8_t sync_atomic_fetchand_8(volatile uint8_t* p, uint8_t v);
SO_EXPORT uint16_t sync_atomic_fetchand_16(volatile uint16_t* p, uint16_t v);
SO_EXPORT uint32_t sync_atomic_fetchand_32_pfn(volatile uint32_t* p, uint32_t v);
SO_EXPORT uint64_t sync_atomic_fetchand_64_pfn(volatile uint64_t* p, uint64_t v);
SO_EXPORT void* sync_atomic_fetchand_ptr(volatile void* p, void* v);

SO_EXPORT char sync_atomic_fetchor_char(volatile char* p, char v);
SO_EXPORT int sync_atomic_fetchor_int(volatile int* p, int v);
SO_EXPORT u_int sync_atomic_fetchor_uint(volatile u_int* p, u_int v);
SO_EXPORT unsigned long long sync_atomic_fetchor_long(volatile unsigned long long* p, unsigned long long v);
SO_EXPORT uint8_t sync_atomic_fetchor_8(volatile uint8_t* p, uint8_t v);
SO_EXPORT uint16_t sync_atomic_fetchor_16(volatile uint16_t* p, uint16_t v);
SO_EXPORT uint32_t sync_atomic_fetchor_32_pfn(volatile uint32_t* p, uint32_t v);
SO_EXPORT uint64_t sync_atomic_fetchor_64_pfn(volatile uint64_t* p, uint64_t v);
SO_EXPORT void* sync_atomic_fetchor_ptr(volatile void* p, void* v);

SO_EXPORT char sync_atomic_fetchxor_char(volatile char* p, char v);
SO_EXPORT int sync_atomic_fetchxor_int(volatile int* p, int v);
SO_EXPORT u_int sync_atomic_fetchxor_uint(volatile u_int* p, u_int v);
SO_EXPORT unsigned long long sync_atomic_fetchxor_long(volatile unsigned long long* p, unsigned long long v);
SO_EXPORT uint8_t sync_atomic_fetchxor_8(volatile uint8_t* p, uint8_t v);
SO_EXPORT uint16_t sync_atomic_fetchxor_16(volatile uint16_t* p, uint16_t v);
SO_EXPORT uint32_t sync_atomic_fetchxor_32_pfn(volatile uint32_t* p, uint32_t v);
SO_EXPORT uint64_t sync_atomic_fetchxor_64_pfn(volatile uint64_t* p, uint64_t v);
SO_EXPORT void* sync_atomic_fetchxor_ptr(volatile void* p, void* v);

SO_EXPORT int sync_bool_compare_and_swap_char(char* ptr, char compare, char set);
SO_EXPORT char sync_val_compare_and_swap_char(char* ptr, char compare, char set);
SO_EXPORT int sync_bool_compare_and_swap_int(int* ptr, int compare, int set);
SO_EXPORT char sync_val_compare_and_swap_int(int* ptr, int compare, int set);
SO_EXPORT int sync_bool_compare_and_swap_uint(unsigned int* ptr, unsigned int compare, unsigned int set);
SO_EXPORT unsigned int sync_val_compare_and_swap_uint(unsigned int* ptr, unsigned int compare, unsigned int set);
SO_EXPORT int sync_bool_compare_and_swap_8(uint8_t* ptr, uint8_t compare, uint8_t set);
SO_EXPORT uint8_t sync_val_compare_and_swap_8(uint8_t* ptr, uint8_t compare, uint8_t set);
SO_EXPORT int sync_bool_compare_and_swap_16(uint16_t* ptr, uint16_t compare, uint16_t set);
SO_EXPORT uint16_t sync_val_compare_and_swap_16(uint16_t* ptr, uint16_t compare, uint16_t set);
SO_EXPORT int sync_bool_compare_and_swap_32(uint32_t* ptr, uint32_t compare, uint32_t set);
SO_EXPORT uint32_t sync_val_compare_and_swap_32(uint32_t* ptr, uint32_t compare, uint32_t set);
SO_EXPORT int sync_bool_compare_and_swap_64(uint64_t* ptr, uint64_t compare, uint64_t set);
SO_EXPORT uint64_t sync_val_compare_and_swap_64(uint64_t* ptr, uint64_t compare, uint64_t set);
SO_EXPORT int  sync_bool_compare_and_swap_ptr(void* ptr, void* compare, void* set);
SO_EXPORT void* sync_val_compare_and_swap_ptr(void* ptr, void* compare, void* set);
#ifdef __cplusplus
}
#endif
#endif /* INTERNAL_ATOMIC_C11_H */
