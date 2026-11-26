#include <math.h>

#include "../crt.h"

float crt_sinf(float x) { return sinf(x); }
float crt_cosf(float x) { return cosf(x); }
float crt_tanf(float x) { return tanf(x); }
float crt_atanf(float x) { return atanf(x); }
float crt_acosf(float x) { return acosf(x > 1.0f ? 1.0f : x < -1.0f ? -1.0f : x); }
float crt_sqrtf(float x) { return sqrtf(fabsf(x)); }
float crt_atan2f(float y, float x) { return atan2f(y, x); }
float crt_powf(float x, float y) { return powf(x, y); }
