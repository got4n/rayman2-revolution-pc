#ifndef REVO_PS2_KERNEL_TYPES_H
#define REVO_PS2_KERNEL_TYPES_H

#include "ps2/types.h"

typedef struct ee_sema_t {
    s32 count;
    s32 max_count;
    s32 init_count;
    s32 wait_threads;
    s32 attr;
    s32 option;
} ee_sema_t;

#endif
