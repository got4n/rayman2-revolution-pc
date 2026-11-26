#include "common.h"

#include "TMR/TMR.h"
#ifdef PC_PORT
#include "PC/PC.h"
#endif

#ifdef PC_PORT
static s32 fn_vTimerWait_game(s32 ulTicksToWait);

s32 fn_vTimerWait(s32 ulTicksToWait)
{
    if (pc_mod_skip_wait(ulTicksToWait)) {
        return 0;
    }
    return fn_vTimerWait_game(ulTicksToWait);
}

#define fn_vTimerWait fn_vTimerWait_game
#endif
s32 fn_vTimerWait(s32 ulTicksToWait)
{
    return TMR_fn_wWait(inv_GAM_engineClockState, ulTicksToWait);
}
#ifdef PC_PORT
#undef fn_vTimerWait
#endif
