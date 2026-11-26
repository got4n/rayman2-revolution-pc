#ifndef REVO_PS2_EE_F32_MINMAX_H
#define REVO_PS2_EE_F32_MINMAX_H

#define R5900_FMAX_VOLATILE(out, a, b)                                                             \
    __asm__ volatile("max.s %0,%1,%2" : "=f"(out) : "f"(a), "f"(b))

#endif
