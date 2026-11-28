#include "PC.h"
#include "mods.h"
#include "pc_host.h"

enum { QUARTERS = 240 };

static uint64_t frame_start;

static int sixty = -1;

int revo_script_at_30hz(void);

int revo_sixty(void)
{
    return sixty < 0 ? revo_mod(REVO_MOD_60FPS) : sixty;
}

int32_t pc_frame_vblanks(void)
{
    const uint64_t passed = revo_frame_count() - frame_start;

    return (int32_t) (revo_sixty() ? passed * 2 : passed);
}

int32_t pc_frame_units_per_tick(void)
{
    return revo_sixty() ? 2 : 1;
}

int32_t pc_frame_tick_parts(void)
{
    return 1;
}

int revo_time_jump_next(uint32_t frequency);

static uint64_t half_end_ns, full_draw_ns;

void pc_frame_wait(int32_t vblanks)
{
    if (half_end_ns) {
        const uint64_t took = revo_now_ns() - half_end_ns;

        if (took < 20000000u) {
            full_draw_ns = full_draw_ns ? (full_draw_ns * 7 + took) / 8 : took;
        }
        half_end_ns = 0;
    }
    while (pc_frame_vblanks() <= vblanks) {
        if (revo_time_jump_next(0)) {
            continue;
        }
        revo_kernel_poll();
        revo_time_pause(0);
    }
}

static uint64_t frame_start_quarters;
static uint64_t frame_vblank_ns;

void pc_frame_start(void)
{
    sixty = revo_mod(REVO_MOD_60FPS) && !revo_script_at_30hz();
    frame_start = revo_frame_count();
    frame_start_quarters = revo_time_ticks(QUARTERS);
    frame_vblank_ns = revo_vblank_ns();
}

void pc_frame_wait_half(void)
{
    const uint64_t half = revo_sixty() ? 2 : 4;

    if (revo_fast_forwarding()) {
        while (revo_time_ticks(QUARTERS) - frame_start_quarters < half) {
            if (revo_time_jump_next(QUARTERS)) {
                continue;
            }
            revo_kernel_poll();
            revo_time_pause(QUARTERS);
        }
        return;
    }
    uint64_t due = frame_vblank_ns + revo_vblank_period_ns() * half / 4;

    if (half == 2) {
        const uint64_t now = revo_now_ns(), even = now + revo_vblank_period_ns() / 2 - full_draw_ns;

        due = even > now ? even : now;
    }
    while (revo_now_ns() < due && !revo_fast_forwarding()) {
        revo_kernel_poll();
        revo_time_pause_until(due);
    }
    half_end_ns = revo_now_ns();
}

void pc_frame_wait_vblank(void)
{
    revo_thread_wait_vblank();
}

void pc_frame_vblank_signal(int32_t sema)
{
    revo_frame_vblank_signal(sema);
}
