#ifndef REVO_PS2_LIBC_H
#define REVO_PS2_LIBC_H

#include "ps2/types.h"

extern void* memset(void* s, int c, u32 n);
extern void* memcpy(void* dst, const void* src, u32 n);
extern void* memmove(void* dst, const void* src, u32 n);
extern int memcmp(const void* a, const void* b, u32 n);

extern char* strcpy(char* dst, const char* src);
extern char* strncpy(char* dst, const char* src, u32 n);
extern char* strncat(char* dst, const char* src, u32 n);
extern int strcmp(const char* a, const char* b);
extern int strncmp(const char* a, const char* b, u32 n);
extern u32 strlen(const char* s);
extern char* strstr(const char* haystack, const char* needle);

extern int printf(const char* fmt, ...);
extern int sprintf(char* dst, const char* fmt, ...);

extern void* malloc(u32 size);
extern void* realloc(void* p, u32 size);
extern int rand(void);
extern void srand(u32 seed);

extern u8 inv_FMN_pathCharacterFlags[];

#define RAND_MAX 0x7FFFFFFF
#define RAND_RANGE_inv 2147483648.0f
#define RAND_SCALE_inv (1.0f / RAND_RANGE_inv)

#endif
