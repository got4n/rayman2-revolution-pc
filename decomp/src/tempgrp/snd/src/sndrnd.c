#include "common.h"

#include "ps2/libc.h"
#include "snd/snd.h"

s32 SND_fn_rGetRandomSnd(s32 range)
{
    return SND_fn_rMulRealRealQuickSnd(SND_fn_rDivRealRealSnd(rand(), RAND_MAX), range);
}
