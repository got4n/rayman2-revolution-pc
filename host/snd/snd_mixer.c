#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "snd.h"
#include "snd_pc.h"

#define CHANNELS 48

#define RATE 48000
#define MAX_STEP (4u << 16)
#define GLIDE (1.0f / 72.0f)

#define FIRST_RATE (0x3FFF * RATE / 4096)

#define LEVEL 0.5f

typedef struct {
    int on;
    int stopping;
    int last;
    int ended;
    u32 next;
    u32 loop;
    u32 step, frac;
    float left, right;
    float gain_l, gain_r, fade;
    s16 pcm[28];
    int pos;
    s32 s1, s2;
    float h[4];
} Channel;

static Channel g_c[CHANNELS];

static int g_notify_ch = -1;
static u32 g_notify_at;
static int g_notify_task;

static Channel *channel(u32 i) { return i < CHANNELS ? &g_c[i] : NULL; }

static u32 step_of(u32 rate) {
    const u64 step = ((u64)rate << 16) / RATE;
    return step > MAX_STEP ? MAX_STEP : (u32)step;
}

static float level(u32 v) {
    v &= 0xFFFF;
    if (v & 0x8000) return 1.0f;
    return (float)(s16)(u16)(v << 1) / 32766.0f;
}

void snd_pc_mixer_open(void) {
    memset(g_c, 0, sizeof g_c);
    for (int i = 0; i < CHANNELS; i++) g_c[i].step = step_of(FIRST_RATE);
    g_notify_ch = -1;
    g_notify_task = 0;
}

void snd_pc_channel_play(u32 i, u32 sound) {
    Channel *c = channel(i);
    if (!c) return;
    const float l = c->left, r = c->right;
    const u32 step = c->step;
    const int playing = c->on && !c->stopping;
    const float gl = c->gain_l, gr = c->gain_r, fade = c->fade;
    memset(c, 0, sizeof *c);
    c->left = l;
    c->right = r;
    c->next = c->loop = sound & ~15u;
    c->step = step;
    c->pos = 28;
    c->on = 1;
    if (playing) {
        c->gain_l = gl;
        c->gain_r = gr;
        c->fade = fade;
    } else {
        c->gain_l = l;
        c->gain_r = r;
    }
}

void snd_pc_channel_stop(u32 i) {
    Channel *c = channel(i);
    if (c && c->on) c->stopping = 1;
}

void snd_pc_channel_rate(u32 i, u32 rate) {
    Channel *c = channel(i);
    if (c) c->step = step_of(rate);
}

void snd_pc_channel_volume(u32 i, u32 left, u32 right) {
    Channel *c = channel(i);
    if (c) {
        c->left = level(left);
        c->right = level(right);
    }
}

u32 snd_pc_channel_ended(u32 i) {
    const Channel *c = channel(i);
    return c && c->ended;
}

u32 snd_pc_channel_position(u32 i) {
    const Channel *c = channel(i);
    return c ? c->next : 0;
}

void snd_pc_channel_notify(u32 i, u32 at, u32 task) {
    g_notify_ch = task && i < CHANNELS ? (int)i : -1;
    g_notify_at = at & ~15u;
    g_notify_task = (int)task;
}

static const int k_filter[5][2] = {{0, 0}, {60, 0}, {115, -52}, {98, -55}, {122, -60}};

static void decode(Channel *c) {
    if (c->last) {
        c->on = 0;
        return;
    }
    const u8 *b = snd_mem(c->next, 16);
    if (!b) {
        c->on = 0;
        return;
    }
    int shift = b[0] & 0xF, filter = (b[0] >> 4) & 7;
    const int flags = b[1];
    if (shift > 12) shift = 9;
    if (filter > 4) filter = 4;
    if (flags & 4) c->loop = c->next;
    if (g_notify_ch == (int)(c - g_c) && c->next == g_notify_at) {
        const int task = g_notify_task;
        g_notify_ch = -1;
        g_notify_task = 0;
        snd_task_due(task);
    }
    const int f0 = k_filter[filter][0], f1 = k_filter[filter][1];
    for (int i = 0; i < 28; i++) {
        const int nibble = (b[2 + i / 2] >> ((i & 1) * 4)) & 0xF;
        s32 s = (s32)(s16)(u16)(nibble << 12) >> shift;
        s += (c->s1 * f0 + c->s2 * f1 + 32) >> 6;
        if (s > 32767) s = 32767;
        if (s < -32768) s = -32768;
        c->s2 = c->s1;
        c->s1 = s;
        c->pcm[i] = (s16)s;
    }
    c->pos = 0;
    if (flags & 1) {
        c->ended = 1;
        if (flags & 2) {
            c->next = c->loop;
        } else {
            c->next += 16;
            c->last = 1;
        }
    } else {
        c->next += 16;
    }
}

static float play(Channel *c) {
    c->frac += c->step;
    while (c->frac >= 0x10000u) {
        c->frac -= 0x10000u;
        if (c->pos >= 28) decode(c);
        if (!c->on) return 0.0f;
        c->h[0] = c->h[1];
        c->h[1] = c->h[2];
        c->h[2] = c->h[3];
        c->h[3] = (float)c->pcm[c->pos++] * (1.0f / 32768.0f);
    }
    const float t = (float)c->frac * (1.0f / 65536.0f);
    const float a = c->h[0], b = c->h[1], d = c->h[2], e = c->h[3];
    return b + 0.5f * t * (d - a + t * (2.0f * a - 5.0f * b + 4.0f * d - e + t * (3.0f * (b - d) + e - a)));
}

void snd_mixer_generate(s16 *out, int frames) {
    for (int f = 0; f < frames; f++) {
        float l = 0, r = 0;
        for (int i = 0; i < CHANNELS; i++) {
            Channel *c = &g_c[i];
            if (!c->on) continue;
            const float s = play(c);
            c->fade += ((c->stopping ? 0.0f : 1.0f) - c->fade) * GLIDE * 2.0f;
            c->gain_l += (c->left - c->gain_l) * GLIDE;
            c->gain_r += (c->right - c->gain_r) * GLIDE;
            if (c->stopping && c->fade < 1e-3f) c->on = 0;
            l += s * c->gain_l * c->fade;
            r += s * c->gain_r * c->fade;
        }
        const float L = l * (LEVEL * 32768.0f), R = r * (LEVEL * 32768.0f);
        out[f * 2] = (s16)(L > 32767.0f ? 32767 : L < -32768.0f ? -32768 : (int)lrintf(L));
        out[f * 2 + 1] = (s16)(R > 32767.0f ? 32767 : R < -32768.0f ? -32768 : (int)lrintf(R));
    }
}

int snd_mixer_channels_on(void) {
    int n = 0;
    for (int i = 0; i < CHANNELS; i++) n += g_c[i].on;
    return n;
}

void snd_mixer_dump(void) {
    if (g_notify_ch >= 0)
        fprintf(stderr, "[snd mixer] notice: channel %d at %06X, task %d\n", g_notify_ch, g_notify_at, g_notify_task);
    for (int i = 0; i < CHANNELS; i++) {
        const Channel *c = &g_c[i];
        if (!c->on) continue;
        fprintf(stderr, "[snd mixer]  channel %d next %06X loop %06X step %05X volume %.3f/%.3f%s\n", i, c->next,
                c->loop, c->step, c->left, c->right, c->stopping ? " (stopping)" : "");
    }
}
