#ifndef REVO_PS2_R5900_H
#define REVO_PS2_R5900_H

#define R5900_STR_INNER(x) #x

#define R5900_STR(x) R5900_STR_INNER(x)

#define R5900_MTC1_ZERO_F32(out) __asm__ volatile("mtc1 $0, %0" : "=f"(out))

#define R5900_MMI_UN(OP, out, a) __asm__(OP " %0, %1" : "=r"(out) : "r"(a))

#define R5900_MMI_SHIFT(OP, out, a, sa) __asm__(OP " %0, %1, %2" : "=r"(out) : "r"(a), "i"(sa))

#define R5900_MFC0(out, cp0reg) __asm__ volatile("mfc0 %0, $" R5900_STR(cp0reg) : "=r"(out))

#define R5900_EI() __asm__ volatile("ei" : : : "memory")

#define R5900_DI() __asm__ volatile("di" : : : "memory")

#define R5900_SYNC() __asm__ volatile("sync" : : : "memory")

#define R5900_SYNC_P() __asm__ volatile("sync.p" : : : "memory")

#define R5900_TRANSFORM_POINT_inv(out, in, m)                                                         \
    __asm__("lwc1 $f0, 0(%1)\n\tlwc1 $f3, 0(%2)\n\tlwc1 $f1, 4(%1)\n\tlwc1 $f4, 16(%2)\n\t"        \
            "lwc1 $f2, 8(%1)\n\tlwc1 $f5, 32(%2)\n\tlwc1 $f6, 48(%2)\n\t"                          \
            "mula.s $f3, $f0\n\tmadda.s $f4, $f1\n\tmadd.s $f7, $f5, $f2\n\t"                      \
            "lwc1 $f3, 4(%2)\n\tlwc1 $f4, 20(%2)\n\tadd.s $f7, $f7, $f6\n\t"                       \
            "lwc1 $f5, 36(%2)\n\tlwc1 $f6, 52(%2)\n\tswc1 $f7, 0(%0)\n\t"                          \
            "mula.s $f3, $f0\n\tmadda.s $f4, $f1\n\tmadd.s $f7, $f5, $f2\n\t"                      \
            "lwc1 $f3, 8(%2)\n\tlwc1 $f4, 24(%2)\n\tadd.s $f7, $f7, $f6\n\t"                       \
            "lwc1 $f5, 40(%2)\n\tlwc1 $f6, 56(%2)\n\tswc1 $f7, 4(%0)\n\t"                          \
            "mula.s $f3, $f0\n\tmadda.s $f4, $f1\n\tmadd.s $f7, $f5, $f2\n\t"                      \
            "add.s $f7, $f7, $f6\n\tswc1 $f7, 8(%0)"                                               \
            :                                                                                      \
            : "r"(out), "r"(in), "r"(m)                                                            \
            : "$f0", "$f1", "$f2", "$f3", "$f4", "$f5", "$f6", "$f7")

#endif
