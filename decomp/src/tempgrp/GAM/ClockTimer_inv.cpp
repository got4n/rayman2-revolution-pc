#include "common.h"

extern "C" {
#include "TMR/TMR.h"

extern inv_tdstTimer0* inv_TMR_p_stTimer0;
extern s32 inv_TMR_lTimer0RefCount;
}

#ifdef PC_PORT
extern "C" void* __builtin_new(u32 size);

extern "C" inv_tdstTimer0* func_0014B830(u32 requested)
{
    if (inv_TMR_p_stTimer0 == 0) {
        inv_TMR_p_stTimer0 = fn_iTimerInit((inv_tdstTimer0*) __builtin_new(8), requested);
        inv_TMR_lTimer0RefCount = 0;
    }
    inv_TMR_lTimer0RefCount++;
    return inv_TMR_p_stTimer0;
}
#else
extern "C" inv_tdstTimer0* func_0014B830(u32 requested)
{
    if (inv_TMR_p_stTimer0 == 0) {
        inv_TMR_p_stTimer0 = new inv_tdstTimer0(requested);
        inv_TMR_lTimer0RefCount = 0;
    }
    inv_TMR_lTimer0RefCount++;
    return inv_TMR_p_stTimer0;
}
#endif

extern "C" void func_0014B8B8(void)
{
    if (--inv_TMR_lTimer0RefCount == 0 && inv_TMR_p_stTimer0 != 0) {
        inv_TMR_fn_vTimer0Shutdown(inv_TMR_p_stTimer0, 3);
        inv_TMR_p_stTimer0 = 0;
    }
}
