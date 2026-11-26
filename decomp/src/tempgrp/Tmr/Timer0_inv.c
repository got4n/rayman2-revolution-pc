#include "common.h"

#include "TMR/TMR.h"

#include "ps2/r5900.h"
#ifdef PC_PORT
#include "PC/PC.h"
#endif

s32 RemoveIntcHandler(s32 cause, s32 handlerId);

#define TIMER_COUNT_REGISTER ((volatile u32*) 0x10000000)
#define TIMER_MODE_REGISTER ((volatile u32*) 0x10000010)

typedef s32 (*InterruptHandler)(void);

extern s32 inv_TMR_lTimer0HandlerId;
extern u32 inv_TMR_ulTimer0Mode;
extern u64 inv_PS2_lastTimer0Value;
extern u64 inv_PS2_timer0OverflowCount;

s32 AddIntcHandler(s32 cause, InterruptHandler handler, s32 next);
void EnableIntc(s32 cause);
s32 __builtin_delete(void* block);

#ifndef PC_PORT
s32 inv_TMR_fn_iTimer0InterruptHandler(void)
{
    inv_PS2_timer0OverflowCount++;
    *TIMER_MODE_REGISTER |= 0xA00;
    R5900_SYNC();
    R5900_EI();
    return 0;
}
#endif

inv_tdstTimer0* fn_iTimerInit(inv_tdstTimer0* timer, u32 requested)
{
    if (requested <= 0x8CA00) {
        inv_TMR_ulTimer0Mode = 2;
        timer->frequency = 0x8CA00;
    } else if (requested <= 0x8CA000) {
        inv_TMR_ulTimer0Mode = 1;
        timer->frequency = 0x8CA000;
    } else {
        inv_TMR_ulTimer0Mode = 0;
        timer->frequency = 0x8CA0000;
    }

    inv_PS2_lastTimer0Value = 0;
    inv_PS2_timer0OverflowCount = 0;
#ifdef PC_PORT
    inv_TMR_lTimer0HandlerId = 0;
#else
    inv_TMR_lTimer0HandlerId = AddIntcHandler(9, inv_TMR_fn_iTimer0InterruptHandler, 0);
    *TIMER_MODE_REGISTER = inv_TMR_ulTimer0Mode | 0x280;
    EnableIntc(9);
    inv_TMR_ulTimer0Mode = *TIMER_MODE_REGISTER;
#endif
    return timer;
}

void inv_TMR_fn_vTimer0Shutdown(inv_tdstTimer0* timer, u32 flags)
{
#ifndef PC_PORT
    RemoveIntcHandler(9, inv_TMR_lTimer0HandlerId);
#endif
    inv_TMR_lTimer0HandlerId = 0;
    inv_PS2_timer0OverflowCount = 0;
    inv_PS2_lastTimer0Value = 0;
    inv_TMR_ulTimer0Mode = 0;
#ifndef PC_PORT
    *TIMER_MODE_REGISTER = 0;
    *TIMER_COUNT_REGISTER = 0;
#endif
    if (flags & 1) {
        __builtin_delete(timer);
        timer = NULL;
    }
}

s32 inv_TMR_fn_ulGetTimer0Frequency(const inv_tdstTimer0* timer)
{
    return timer->frequency;
}

#ifdef PC_PORT
u64 fn_uiTimer0GetTime(inv_tdstTimer0* timer)
{
    u64 time = pc_time_ticks((u32) * (const u64*) timer);

    inv_PS2_lastTimer0Value = time;
    return time;
}
#else
u64 fn_uiTimer0GetTime(inv_tdstTimer0* timer)
{
    u64 time = inv_PS2_timer0OverflowCount << 16;
    u32 count = *TIMER_COUNT_REGISTER;
    u64 previous = inv_PS2_lastTimer0Value;

    time += count;
    if (time < previous) {
        do {
            time += 0x10000;
        } while (time < previous);
    }
    inv_PS2_lastTimer0Value = time;
    return time;
}
#endif

u64 inv_TMR_fn_ulGetTimer0Seconds(inv_tdstTimer0* timer)
{
    return fn_uiTimer0GetTime(timer) / timer->frequency;
}

#ifdef PC_PORT
void inv_TMR_fn_vResetTimer0(void)
{
    inv_PS2_timer0OverflowCount = 0;
    inv_PS2_lastTimer0Value = 0;
}
#else
void inv_TMR_fn_vResetTimer0(void)
{
    volatile u32* mode = TIMER_MODE_REGISTER;
    u32 savedMode;

    *mode |= 7;
    inv_PS2_timer0OverflowCount = 0;
    savedMode = inv_TMR_ulTimer0Mode;
    inv_PS2_lastTimer0Value = 0;
    *mode = savedMode;
    *TIMER_COUNT_REGISTER = 0;
    *mode = savedMode | 0x280;
}
#endif
