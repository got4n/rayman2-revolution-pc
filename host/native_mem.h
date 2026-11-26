#ifndef REVO_NATIVE_MEM_H
#define REVO_NATIVE_MEM_H
#include <stddef.h>
#include <stdint.h>

uint8_t *revo_native_base(void);
void *revo_low_alloc(size_t size);
void revo_low_free(void *p);
void revo_shadow_stack(size_t size);
void revo_shadow_stack_ensure(void);

#endif
