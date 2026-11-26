#include "common.h"

#include "GAM/GAM.h"
#include "TMR/TMR.h"

s32 TMR_fn_wInitLibrary(void);
s32 TMR_fn_wStartTimer(TimerReadStateView* timer);
s32 func_0014BC98(TimerReadStateView* timer);
s32 inv_fn_vCopyOffset14ToOffset04(TimerReadStateView* timer);

s32 inv_fn_vInitEngineClockLibrary(void)
{
    return TMR_fn_wInitLibrary();
}

f32 inv_fn_xGetEngineClockTime(void)
{
    return inv_GAM_engineClockState->normalized;
}

u32 inv_fn_ulGetEngineClock(void)
{
    return inv_GAM_engineClockState->totalTicks;
}

f32 inv_fn_xGetEngineClockDelta(void)
{
    return inv_GAM_engineClockState->scaledStep;
}

f32 inv_fn_xGetEngineClockScale(void)
{
    return inv_GAM_engineClockState->scale;
}

u32 inv_fn_ulGetEngineTrameNumber(void)
{
    return inv_GAM_engineClockState->reads;
}

s32 fn_lGetTemporalCurveType(void)
{
    return inv_GAM_engineClockState->stepCount;
}

#ifdef PC_PORT
extern f32 inv_TMR_engineClockTicks;

f32 inv_GAM_fn_xGetEngineClockTicks(void)
{
    return inv_TMR_engineClockTicks;
}

#endif
s32 fn_vSaveEngineClock(void)
{
    return inv_fn_vCopyOffset14ToOffset04(inv_GAM_engineClockState);
}

s32 fn_vLoadEngineClock(void)
{
    return func_0014BC98(inv_GAM_engineClockState);
}

s32 fn_vReinitEngineClock(void)
{
    return TMR_fn_wStartTimer(inv_GAM_engineClockState);
}
