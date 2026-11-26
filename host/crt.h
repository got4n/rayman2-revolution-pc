#ifndef REVO_CRT_H
#define REVO_CRT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CRT_API

CRT_API uint32_t crt_memcpy(uint32_t dst, uint32_t src, uint32_t n);
CRT_API uint32_t crt_memmove(uint32_t dst, uint32_t src, uint32_t n);
CRT_API uint32_t crt_memset(uint32_t dst, int32_t c, uint32_t n);
CRT_API int32_t crt_memcmp(uint32_t a, uint32_t b, uint32_t n);
CRT_API uint32_t crt_strlen(uint32_t s);
CRT_API uint32_t crt_strcpy(uint32_t dst, uint32_t src);
CRT_API uint32_t crt_strncpy(uint32_t dst, uint32_t src, uint32_t n);
CRT_API uint32_t crt_strncat(uint32_t dst, uint32_t src, uint32_t n);
CRT_API int32_t crt_strcmp(uint32_t a, uint32_t b);
CRT_API int32_t crt_strncmp(uint32_t a, uint32_t b, uint32_t n);
CRT_API int32_t crt_strcasecmp(uint32_t a, uint32_t b);
CRT_API uint32_t crt_index(uint32_t s, int32_t c);
CRT_API uint32_t crt_rindex(uint32_t s, int32_t c);
CRT_API uint32_t crt_strstr(uint32_t haystack, uint32_t needle);
CRT_API uint32_t crt_strlwr(uint32_t s);
CRT_API uint32_t crt_strupr(uint32_t s);
CRT_API float crt_fabsf(float x);

CRT_API float crt_sinf(float x);
CRT_API float crt_cosf(float x);
CRT_API float crt_tanf(float x);
CRT_API float crt_atanf(float x);
CRT_API float crt_acosf(float x);
CRT_API float crt_sqrtf(float x);
CRT_API float crt_atan2f(float y, float x);
CRT_API float crt_powf(float x, float y);

CRT_API uint32_t crt_malloc(uint32_t size);
CRT_API void crt_freemem(uint32_t p);
CRT_API uint32_t crt_realloc(uint32_t p, uint32_t size);
CRT_API uint32_t crt_memalign(uint32_t alignment, uint32_t size);

CRT_API void crt___main(void);
CRT_API void crt_inv___register_frame_info_table(uint32_t table, uint32_t object);
CRT_API uint32_t crt_inv___deregister_frame_info(uint32_t begin);
CRT_API uint32_t crt___builtin_new(uint32_t size);
CRT_API uint32_t crt___builtin_delete(uint32_t p);
CRT_API float crt___floatdisf(int32_t v);
CRT_API double crt_dpadd(double a, double b);
CRT_API double crt_dpmul(double a, double b);
CRT_API double crt_dpdiv(double a, double b);
CRT_API double crt_litodp(int32_t v);
CRT_API int32_t crt_dptoli(double d);
CRT_API uint32_t crt_fptoui(float f);
CRT_API void crt_srand(uint32_t seed);
CRT_API int32_t crt_rand(void);
CRT_API int32_t crt_atol(uint32_t s);
CRT_API int32_t crt_atoi(uint32_t s);
CRT_API uint32_t crt_strtok(uint32_t s, uint32_t delim);
CRT_API void crt_qsort(uint32_t base, uint32_t n, uint32_t size, uint32_t compare);
CRT_API int32_t crt_sprintf(uint32_t dst, uint32_t fmt, uint32_t args);

uint32_t crt_sbrk_r(uint32_t reent, int32_t increment);

#ifdef __cplusplus
}
#endif

#endif
