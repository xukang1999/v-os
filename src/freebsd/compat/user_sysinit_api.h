#ifndef USER_SYSINIT_API_H
#define USER_SYSINIT_API_H

extern void mallocinit(void *dummy);

extern void msgbufinit(void* ptr, int size);

extern void kmem_init(vm_offset_t start, vm_offset_t end);
extern void vm_mem_init(void*);
extern void vm_stats_init(void *arg);
extern void vmcounter_startup(void);
extern void proc0_init(void* dummy);
extern void pcpu_zones_startup(void);
extern void initclocks(void* dummy);
#endif//USER_SYSINIT_API_H