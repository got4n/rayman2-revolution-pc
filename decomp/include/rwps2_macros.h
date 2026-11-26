#ifndef REVO_RWPS2_MACROS_H
#define REVO_RWPS2_MACROS_H

#include "rwps2.h"

#ifndef RWPS2_MAKE_QWORD
#define RWPS2_MAKE_QWORD(result, msb, lsb)                                                         \
    __asm__ volatile("pcpyld %0, %1, %2" : "=r"(result) : "r"((u64) (msb)), "r"((u64) (lsb)))
#endif

#endif
