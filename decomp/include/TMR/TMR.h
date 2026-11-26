#ifndef REVO_TMR_TMR_H
#define REVO_TMR_TMR_H

#include "ps2/types.h"

typedef struct inv_tdstTimer0 {
    u64 frequency;
#ifdef __cplusplus
    inv_tdstTimer0(u32 requested) __asm__("fn_iTimerInit");
#endif
} inv_tdstTimer0;

typedef struct TimerReadStateView {
    inv_tdstTimer0* source;
    u32 ulPauseTime;
    f32 scale;
    f32 rate;
    u32 ticksPerUnit;
    u32 totalTicks;
    u64 lastTime;
    u32 frameTicks;
    u32 stepCount;
    u32 reads;
    f32 scaledStep;
    f32 normalized;
#ifdef __cplusplus
    TimerReadStateView(s32 ticksPerUnit, f32 scale) __asm__("TMR_fn_wCreateTimer");
#endif
} TimerReadStateView;

extern TimerReadStateView* inv_GAM_engineClockState;

extern inv_tdstTimer0* func_0014B830(u32 requested);

extern s32 func_0014B9F8(f32 units);
extern s32 func_0014BA50(void);
extern s32 fn_vActualizeEngineClock(void);
extern s32 func_0014BA70(f32 period);
extern u64 func_0014BA90(void);

extern s32 fn_vTimerWait(s32 ulTicksToWait);

extern s32 TMR_fn_wReadTimer(TimerReadStateView* timer);
extern s32 TMR_fn_wWait(TimerReadStateView* timer, u32 ulTimeToWait);
extern s32 func_0014BFD8(TimerReadStateView* timer);
extern u64 func_0014C050(TimerReadStateView* timer);

extern inv_tdstTimer0* fn_iTimerInit(inv_tdstTimer0* timer, u32 requested);
extern void inv_TMR_fn_vTimer0Shutdown(inv_tdstTimer0* timer, u32 flags);
extern s32 inv_TMR_fn_ulGetTimer0Frequency(const inv_tdstTimer0* timer);
extern u64 fn_uiTimer0GetTime(inv_tdstTimer0* timer);

#endif
