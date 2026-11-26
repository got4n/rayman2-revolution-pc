#include "common.h"

#include "TMR/TMR.h"

#include "MTH/MTH.h"

s32 TMR_fn_wSetTimerValue(TimerReadStateView* timer, f32 period);

s32 fn_vActualizeEngineClock(void)
{
    return TMR_fn_wReadTimer(inv_GAM_engineClockState);
}

s32 func_0014B9F8(f32 units)
{
    s32 ticks;

    ticks = inv_MTH_fn_lToIntegerOrdered(units * inv_GAM_engineClockState->ticksPerUnit);
    return ticks;
}

s32 func_0014BA50(void)
{
    return func_0014BFD8(inv_GAM_engineClockState);
}

s32 func_0014BA70(f32 period)
{
    return TMR_fn_wSetTimerValue(inv_GAM_engineClockState, period);
}

u64 func_0014BA90(void)
{
    return func_0014C050(inv_GAM_engineClockState);
}
