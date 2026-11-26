#ifndef REVO_GLUE_RT_H
#define REVO_GLUE_RT_H

#include <stdbool.h>

#include "rt.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int revo_trace;
void revo_trace_enter(u32 addr);
extern u32 revo_ring[256];
extern u32 revo_break_addr;
extern unsigned revo_ring_i;
#define REVO_TRACE_ENTER(a) do { revo_ring[(revo_ring_i++) & 255] = (a); \
    if (__builtin_expect(revo_trace > 1 || revo_break_addr == (a), 0)) revo_trace_enter(a); } while (0)

extern int revo_count_calls;
void revo_ci_seen(const char *type, u32 target);
void revo_fatal_unresolved(const char *name);
const char *revo_name_of(u32 addr);

#ifdef __cplusplus
}
#endif

#endif
