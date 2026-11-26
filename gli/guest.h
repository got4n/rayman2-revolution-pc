#ifndef GLI_GUEST_H
#define GLI_GUEST_H

#include <stdbool.h>
#include <stddef.h>

#include "game_api.h"
#include "rt.h"
#include "runtime.h"

#define GUEST(type, a) ((type *)(RAM + (u32)(a)))

#endif
