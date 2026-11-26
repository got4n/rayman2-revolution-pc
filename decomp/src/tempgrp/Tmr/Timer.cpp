extern "C" {
#include "common.h"

#include "TMR/TMR.h"

#include "MTH/MTH.h"
#ifdef PC_PORT
#include "PC/PC.h"
#endif

extern f32 D_00392B6C;
extern f32 D_00392B70;
extern u8 inv_DEMO_bPlaying;

#ifdef PC_PORT
extern "C" void func_0014B8B8(void);
#else
s32 func_0014B8B8(TimerReadStateView* timer);
#endif
s32 __builtin_delete(void* block);
u32 inv_DEMO_fn_uGetCurrentRecordFlags(void);
void TMR_fn_wSetTimerValue(TimerReadStateView* timer, f32 scale);
void TMR_fn_wStartTimer(TimerReadStateView* timer);

#ifdef PC_PORT
void* __builtin_new(u32 size);
TimerReadStateView* TMR_fn_wCreateTimer(TimerReadStateView* timer, s32 ticksPerUnit, f32 scale);

void TMR_fn_wInitLibrary(void)
{
    if (inv_GAM_engineClockState == 0) {
        inv_GAM_engineClockState =
            TMR_fn_wCreateTimer((TimerReadStateView*) __builtin_new(0x38), 1000, D_00392B6C);
    }
}
#else
void TMR_fn_wInitLibrary(void)
{
    if (inv_GAM_engineClockState == 0) {
        inv_GAM_engineClockState = new TimerReadStateView(1000, D_00392B6C);
    }
}
#endif

s32 func_0014BB40(TimerReadStateView* timer, u32 flags)
{
#ifdef PC_PORT
    s32 result = 0;

    func_0014B8B8();
#else
    s32 result = func_0014B8B8(timer);
#endif

    timer->source = 0;
    if (flags & 1) {
        result = __builtin_delete(timer);
    }
    return result;
}

TimerReadStateView* TMR_fn_wCreateTimer(TimerReadStateView* timer, s32 ticksPerUnit, f32 scale)
{
    timer->ticksPerUnit = ticksPerUnit;
    timer->source = func_0014B830(ticksPerUnit);
    TMR_fn_wSetTimerValue(timer, scale);
    TMR_fn_wStartTimer(timer);
    return timer;
}

void TMR_fn_wSetTimerValue(TimerReadStateView* timer, f32 scale)
{
    timer->scale = scale;
    timer->rate = (f32) timer->ticksPerUnit * scale;
}

void TMR_fn_wStartTimer(TimerReadStateView* timer)
{
    s32 frameTicks;

    timer->lastTime = fn_uiTimer0GetTime(timer->source);
    timer->totalTicks = 0;
    timer->scaledStep = timer->scale;
    frameTicks = inv_MTH_fn_lToInteger(timer->rate);
    timer->frameTicks = frameTicks;
    timer->ulPauseTime = 0;
    timer->normalized = 0.0f;
    timer->stepCount = 1;
    timer->reads = 1;
}

void inv_fn_vCopyOffset14ToOffset04(TimerReadStateView* timer)
{
    timer->ulPauseTime = timer->totalTicks;
}

void func_0014BC98(TimerReadStateView* timer)
{
    u32 value = timer->ulPauseTime;

    timer->totalTicks = value;
    timer->normalized = (f32) value / (f32) timer->ticksPerUnit;
}

#ifdef PC_PORT
f32 inv_TMR_engineClockTicks = 1.0f;

s32 TMR_fn_wReadTimer(TimerReadStateView* timer)
{
    u64 now;
    u32 period;
    f32 rate;
    u32 step;
    f32 count;
    f32 scale;
    f32 frame;
    f32 scaled;
    u32 oldTotal;
    u32 total;
    s32 rounded0;
    s32 rounded1;
    s32 parts = 1;

    now = fn_uiTimer0GetTime(timer->source);

    if (inv_DEMO_bPlaying) {
        timer->stepCount = inv_DEMO_fn_uGetCurrentRecordFlags();
        period = timer->ticksPerUnit;
        rate = timer->rate;
    } else {
        u32 frequency = inv_TMR_fn_ulGetTimer0Frequency(timer->source);
        u64 ticks;

        if (timer == inv_GAM_engineClockState) {
            parts = pc_frame_tick_parts();
        }
        period = timer->ticksPerUnit;
        ticks = (now - timer->lastTime) * timer->ticksPerUnit / frequency;
        if (parts == 1) {
            rounded0 = inv_MTH_fn_lToIntegerOrdered(
                (((f32) (u32) ticks * 2.0f) / (rate = timer->rate) + 1.0f) * 0.5f);
        } else {
            rounded0 = inv_MTH_fn_lToIntegerOrdered(
                (((f32) (u32) ticks * 2.0f * (f32) parts) / (rate = timer->rate) + 1.0f) * 0.5f);
        }
        timer->stepCount = rounded0;
    }

    step = timer->stepCount;
    if (step == 0) {
        timer->stepCount = 1;
        step = 1;
    }

    if (parts == 1) {
        count = step;
    } else {
        count = (f32) step / (f32) parts;
        timer->stepCount = (step + parts - 1) / parts;
    }
    if (timer == inv_GAM_engineClockState) {
        inv_TMR_engineClockTicks = count;
    }
    scale = timer->scale;
    frame = count * rate;
    oldTotal = timer->totalTicks;
    rounded1 = inv_MTH_fn_lToIntegerOrdered(frame);
    scaled = count * scale;
    total = rounded1;
    timer->frameTicks = total;
    total = oldTotal + rounded1;
    timer->totalTicks = total;
    timer->scaledStep = scaled;
    timer->normalized = (f32) total / (f32) period;
    timer->lastTime = now;
    timer->reads++;
    return timer->reads;
}
#else
s32 TMR_fn_wReadTimer(TimerReadStateView* timer)
{
    u64 now;
    u32 period;
    f32 rate;
    u32 step;
    f32 count;
    u32 total;
    s32 rounded0;
    s32 rounded1;

    now = fn_uiTimer0GetTime(timer->source);

    if (inv_DEMO_bPlaying) {
        timer->stepCount = inv_DEMO_fn_uGetCurrentRecordFlags();
        period = timer->ticksPerUnit;
        rate = timer->rate;
    } else {
        u32 frequency = inv_TMR_fn_ulGetTimer0Frequency(timer->source);
        u64 ticks;

        period = timer->ticksPerUnit;
        ticks = (now - timer->lastTime) * timer->ticksPerUnit / frequency;
        rounded0 = inv_MTH_fn_lToIntegerOrdered(
            (((f32) (u32) ticks * 2.0f) / (rate = timer->rate) + 1.0f) * 0.5f);
        timer->stepCount = rounded0;
    }

    step = timer->stepCount;
    if (step == 0) {
        timer->stepCount = 1;
        step = 1;
    }

    count = step;
    rounded1 = inv_MTH_fn_lToInteger(count * rate);
    timer->frameTicks = rounded1;
    total = timer->totalTicks + rounded1;
    timer->totalTicks = total;
    timer->scaledStep = count * timer->scale;
    timer->normalized = (f32) total / (f32) period;
    timer->lastTime = now;
    timer->reads++;
    return timer->reads;
}
#endif

s32 TMR_fn_wWait(TimerReadStateView* timer, u32 ulTimeToWait)
{
    u64 start;
    u32 frequency;
    f32 duration;
    u64 deadline;
    s32 ticks;

    start = fn_uiTimer0GetTime(timer->source);
    frequency = inv_TMR_fn_ulGetTimer0Frequency(timer->source);
    duration = ulTimeToWait;
    duration *= *(volatile f32*) &D_00392B70;
    ticks = inv_MTH_fn_lToIntegerOrdered(duration * (f32) frequency);
    deadline = start + ticks;
    while (fn_uiTimer0GetTime(timer->source) < deadline) {
    }
}

s32 func_0014BFD8(TimerReadStateView* timer)
{
    u64 elapsed = fn_uiTimer0GetTime(timer->source) - timer->lastTime;
    u32 frequency = inv_TMR_fn_ulGetTimer0Frequency(timer->source);
    u32 ticksPerUnit = timer->ticksPerUnit;

    return timer->totalTicks + (s32) (elapsed * ticksPerUnit / frequency);
}

u64 func_0014C050(TimerReadStateView* timer)
{
    return fn_uiTimer0GetTime(timer->source);
}
}
