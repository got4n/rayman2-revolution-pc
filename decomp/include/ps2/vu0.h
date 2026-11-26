#ifndef REVO_PS2_VU0_H
#define REVO_PS2_VU0_H

#include "ps2/r5900.h"

#ifdef PC_PORT
#include "PC/PC.h"

#define VU0_LQC2(n, base, off) __builtin_memcpy(pc_vu0.vf[n], (const char *)(base) + (off), 16)
#define VU0_SQC2(n, base, off) __builtin_memcpy((char *)(base) + (off), pc_vu0.vf[n], 16)
#define VU0_CFC2_NI(out, n) ((out) = pc_vu0.vi[n])
#define VU0_CFC2_I(out, n) ((out) = pc_vu0.vi[n])
#define VU0_VCALLMS(addr) pc_vu0_call((addr) >> 3, &pc_vu0)
#else
#define VU0_LQC2(vf, base, off)                                                                    \
    __asm__ volatile("lqc2 $vf" R5900_STR(vf) ", %1(%0)" : : "r"(base), "i"(off) : "memory")

#define VU0_SQC2(vf, base, off)                                                                    \
    __asm__ volatile("sqc2 $vf" R5900_STR(vf) ", %1(%0)" : : "r"(base), "i"(off) : "memory")

#define VU0_CFC2_NI(out, vi) __asm__ volatile("cfc2.ni %0, $vi" R5900_STR(vi) : "=r"(out))

#define VU0_CFC2_I(out, vi) __asm__ volatile("cfc2.i %0, $vi" R5900_STR(vi) : "=r"(out))

#define VU0_VCALLMS(addr) __asm__ volatile("vcallms %0" : : "i"(addr))
#endif

#endif
