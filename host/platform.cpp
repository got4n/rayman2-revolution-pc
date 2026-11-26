#include <SDL.h>

#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <ctime>
#include <string>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#endif

#include <SDL_vulkan.h>
#include "mods.h"
#include "settings.h"
#include "update.h"
#include "ui.h"

extern "C" {
#include "runtime.h"
void gli_present(int width, int height);
typedef bool (*VkSurfaceMaker)(void *ctx, void *instance, void *surface);
const char *gli_vulkan_library(void);
bool gli_open_vulkan_window(const char *const *extensions, unsigned n, VkSurfaceMaker make, void *ctx);
void gli_screenshot(const char *name);
void gli_end_present(int width, int height);
}

namespace {
bool g_headless = false;
std::atomic<bool> g_quit{false};
SDL_Window *g_window = nullptr;
std::atomic<uint32_t> g_pad{0xFFFF};
std::atomic<uint32_t> g_ctrl_buttons{0xFFFF};
std::atomic<uint32_t> g_ctrl_sticks{0x80808080u};
std::atomic<uint32_t> g_key_dirs{0};
std::atomic<bool> g_skip_key{false}, g_skip_ctrl{false};
SDL_GameController *g_ctrl = nullptr;
std::atomic<int> g_ctrl_style{REVO_ICONS_PS2};
std::atomic<bool> g_keyboard_last{true};
std::atomic<int> g_rumble_ms{0}, g_rumble_strength{0};
}

extern "C" void revo_pause(int on);

extern "C" void revo_rumble(int strength, int ms) {
    g_rumble_strength = strength;
    g_rumble_ms = ms;
}

static SDL_AudioDeviceID g_audio;
static void open_audio(void) {
    const char *m = getenv("REVO_MUTE");
    if (m && *m == '1') return;
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) { revo_log("[audio] SDL audio: %s", SDL_GetError()); return; }
    SDL_AudioSpec want, have;
    SDL_zero(want);
    want.freq = 48000;
    want.format = AUDIO_S16LSB;
    want.channels = 2;
    want.samples = 512;
    g_audio = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if (!g_audio) { revo_log("[audio] no audio device: %s", SDL_GetError()); return; }
    SDL_PauseAudioDevice(g_audio, 0);
    revo_log("[audio] 48 kHz stereo output");
}

static std::atomic<int> g_audio_muted;

extern "C" void revo_audio_mute(int on) {
    if (g_audio_muted.exchange(on) != on && on && g_audio) SDL_ClearQueuedAudio(g_audio);
}

extern "C" void revo_audio_push(const s16 *frames, int n) {
    if (g_audio && !g_audio_muted.load()) {
        /* At most 64 ms ahead of the speakers. After a hitch the sound thread catches up at once,
           and what the device didn't play meanwhile would stay queued for good: with a bigger
           queue, each hitch made the sound later, up to a quarter of a second. */
        if (SDL_GetQueuedAudioSize(g_audio) < 48000 * 4 * 64 / 1000) {
            const int volume = revo_set(REVO_SET_VOLUME);
            if (volume >= 100) {
                SDL_QueueAudio(g_audio, frames, (Uint32)n * 4);
            } else {
                static s16 scaled[2 * 4096];
                for (int at = 0; at < n;) {
                    const int count = n - at < 4096 ? n - at : 4096;
                    for (int k = 0; k < 2 * count; k++) scaled[k] = (s16)((int)frames[2 * at + k] * volume / 100);
                    SDL_QueueAudio(g_audio, scaled, (Uint32)count * 4);
                    at += count;
                }
            }
        }
    }
}

static Uint32 window_flags(void) {
    return SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE |
           (revo_settings_fullscreen() ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}

static bool make_surface(void *ctx, void *instance, void *surface) {
    return SDL_Vulkan_CreateSurface((SDL_Window *)ctx, (VkInstance)instance, (VkSurfaceKHR *)surface) == SDL_TRUE;
}

static bool open_vulkan_window(const char *title, int width, int height) {
    const char *library = gli_vulkan_library();
    if (!library || SDL_Vulkan_LoadLibrary(library) != 0) return false;
    g_window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height,
                                SDL_WINDOW_VULKAN | window_flags());
    unsigned n = 0;
    const char *ext[16];
    if (g_window && SDL_Vulkan_GetInstanceExtensions(g_window, &n, nullptr) && n <= 16 &&
        SDL_Vulkan_GetInstanceExtensions(g_window, &n, ext) && gli_open_vulkan_window(ext, n, make_surface, g_window)) {
        revo_log("[platform] Vulkan window %dx%d", width, height);
        return true;
    }
    revo_log("[platform] no Vulkan window (%s)", SDL_GetError());
    if (g_window) SDL_DestroyWindow(g_window);
    g_window = nullptr;
    SDL_Vulkan_UnloadLibrary();
    return false;
}

extern "C" void revo_platform_init(void) {
#ifdef _WIN32
    SDL_SetMainReady();
#endif
    SDL_SetHint(SDL_HINT_GAMECONTROLLER_USE_BUTTON_LABELS, "0");
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS) != 0) {
        revo_log("[platform] SDL_Init failed: %s; running headless", SDL_GetError());
        g_headless = true;
        return;
    }
    open_audio();
    revo_settings_load();
    int width, height;
    {
        int across, down;
        revo_view_aspect(&across, &down);
        int h = 960;
        SDL_Rect r;
        if (SDL_GetDisplayUsableBounds(0, &r) == 0) {
            int hmax = r.h - 48;
            if (r.h * 2 / 3 > h) h = r.h * 2 / 3;
            if (hmax < h) h = hmax;
            if (r.w - 32 < h * across / down) h = (r.w - 32) * down / across;
        }
        if (h < 336) h = 336;
        h -= h % (down * 4);
        width = h * across / down;
        height = h;
        int ww, wh;
        if (getenv("REVO_WINDOW") && sscanf(getenv("REVO_WINDOW"), "%dx%d", &ww, &wh) == 2 && ww >= 64 && wh >= 64) {
            width = ww - ww % 16;
            height = wh;
        }
    }
    if (!open_vulkan_window("Rayman Re-Revolution", width, height)) {
        static const char *const no_vulkan[REVO_LANG_COUNT] = {
            "No graphics card with Vulkan was found.\n\n"
            "Rayman Re-Revolution needs a graphics card and driver with Vulkan 1.1. Updating your graphics driver "
            "may fix it.",
            "Aucune carte graphique compatible Vulkan n'a été trouvée.\n\n"
            "Rayman Re-Revolution a besoin d'une carte graphique et d'un pilote compatibles Vulkan 1.1. Mettre à jour "
            "votre pilote graphique peut régler le problème.",
            "Es wurde keine Grafikkarte mit Vulkan gefunden.\n\n"
            "Rayman Re-Revolution benötigt eine Grafikkarte und einen Treiber mit Vulkan 1.1. Ein Update des "
            "Grafiktreibers kann helfen.",
            "Non è stata trovata nessuna scheda grafica con Vulkan.\n\n"
            "Rayman Re-Revolution richiede una scheda grafica e un driver con Vulkan 1.1. Aggiornare il driver grafico "
            "può risolvere il problema.",
            "No se encontró ninguna tarjeta gráfica con Vulkan.\n\n"
            "Rayman Re-Revolution necesita una tarjeta gráfica y un controlador con Vulkan 1.1. Actualizar el "
            "controlador gráfico puede solucionarlo.",
        };
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Rayman Re-Revolution", no_vulkan[revo_language()], nullptr);
        exit(1);
    }
    revo_ui_init(g_window);
    revo_discord_start();
    revo_update_start();
}

static void key(SDL_Keycode k, bool down) {
    for (int a = 0; a < REVO_ACT_COUNT; a++) {
        if (revo_key_binding(a, 0) != (int)k && revo_key_binding(a, 1) != (int)k) continue;
        if (a == REVO_ACT_SKIP) {
            g_skip_key = down;
            continue;
        }
        const int dir = a == REVO_ACT_LEFT ? 0 : a == REVO_ACT_RIGHT ? 1 : a == REVO_ACT_UP ? 2 : a == REVO_ACT_DOWN ? 3 : -1;
        if (dir >= 0) {
            if (down) g_key_dirs.fetch_or(1u << dir);
            else g_key_dirs.fetch_and(~(1u << dir));
        }
        const int bit = revo_action_bit(a);
        if (down) g_pad.fetch_and(~(1u << bit));
        else g_pad.fetch_or(1u << bit);
    }
}

static bool g_shot_asked;
static void screenshot_now(void) {
    char dir[1024], name[1200];
    revo_settings_dir(dir, sizeof dir);
    std::string folder = std::string(dir) + "/screenshots";
#ifdef _WIN32
    _mkdir(dir);
    _mkdir(folder.c_str());
#else
    mkdir(dir, 0755);
    mkdir(folder.c_str(), 0755);
#endif
    time_t t = time(nullptr);
    struct tm tm;
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    char stamp[64];
    strftime(stamp, sizeof stamp, "%Y%m%d_%H%M%S", &tm);
    static int same_second;
    static time_t last;
    same_second = t == last ? same_second + 1 : 0;
    last = t;
    snprintf(name, sizeof name, "%s/rayman_%s%s%.0d.bmp", folder.c_str(), stamp, same_second ? "_" : "", same_second);
    gli_screenshot(name);
    revo_log("[platform] screenshot %s", name);
}

extern "C" u64 revo_frame_count(void);

static void present_once(void);

extern "C" void revo_vblank(void) {}

extern "C" void revo_platform_run(void) {
    if (g_headless) {
#ifdef _WIN32
        for (;;) SDL_Delay(1000);
#else
        for (;;) pause();
#endif
    }
    while (!g_quit) {
        present_once();
    }
    exit(0);
}

static bool g_ctrl_hold;

static void poll_controller(void) {
    if (!g_ctrl || g_ctrl_hold) { g_ctrl_buttons = 0xFFFF; g_ctrl_sticks = 0x80808080u; g_skip_ctrl = false; return; }
    u32 b = 0xFFFF;
    for (int a = 0; a < REVO_ACT_COUNT; a++) {
        const int p = revo_pad_binding(a);
        bool held = false;
        if (p == REVO_PAD_TRIGGER_LEFT) held = SDL_GameControllerGetAxis(g_ctrl, SDL_CONTROLLER_AXIS_TRIGGERLEFT) > 12000;
        else if (p == REVO_PAD_TRIGGER_RIGHT) held = SDL_GameControllerGetAxis(g_ctrl, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) > 12000;
        else if (p >= 0) held = SDL_GameControllerGetButton(g_ctrl, (SDL_GameControllerButton)p) != 0;
        if (a == REVO_ACT_SKIP) g_skip_ctrl = held;
        else if (held) b &= ~(1u << revo_action_bit(a));
    }
    g_ctrl_buttons = b;
    auto ax = [](SDL_GameControllerAxis a, int percent) {
        int v = SDL_GameControllerGetAxis(g_ctrl, a) * percent / 100;
        int u = (v + 32768) >> 8;
        if (u > 0x70 && u < 0x90) u = 0x80;
        return (u32)(u < 0 ? 0 : u > 255 ? 255 : u);
    };
    g_ctrl_sticks = ax(SDL_CONTROLLER_AXIS_RIGHTX, 100) | ax(SDL_CONTROLLER_AXIS_RIGHTY, 100) << 8 |
                    ax(SDL_CONTROLLER_AXIS_LEFTX, 133) << 16 | ax(SDL_CONTROLLER_AXIS_LEFTY, 133) << 24;
}

void revo_window_set_fullscreen(bool on) {
    if (!g_window || on == revo_window_fullscreen()) return;
    if (SDL_SetWindowFullscreen(g_window, on ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0) != 0) {
        revo_log("[platform] full screen: %s", SDL_GetError());
        return;
    }
    revo_log("[platform] %s", on ? "full screen" : "windowed");
    if (!on) revo_window_fit_aspect();
}

bool revo_window_fullscreen(void) {
    return g_window && (SDL_GetWindowFlags(g_window) & SDL_WINDOW_FULLSCREEN) != 0;
}

void revo_window_fit_aspect(void) {
    if (!g_window || getenv("REVO_WINDOW")) return;
    if (SDL_GetWindowFlags(g_window) & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_MAXIMIZED)) return;
    int across, down, w = 0, h = 0;
    revo_view_aspect(&across, &down);
    SDL_GetWindowSize(g_window, &w, &h);
    if (h <= 0) return;
    const int display = SDL_GetWindowDisplayIndex(g_window);
    int nw = h * across / down;
    SDL_Rect r;
    if (SDL_GetDisplayUsableBounds(display < 0 ? 0 : display, &r) == 0 && nw > r.w - 32) {
        nw = r.w - 32;
        h = nw * down / across;
    }
    nw -= nw % 16;
    if (nw == w) return;
    SDL_SetWindowSize(g_window, nw, h);
    SDL_SetWindowPosition(g_window, SDL_WINDOWPOS_CENTERED_DISPLAY(display < 0 ? 0 : display),
                          SDL_WINDOWPOS_CENTERED_DISPLAY(display < 0 ? 0 : display));
    revo_log("[platform] window %dx%d for %d:%d", nw, h, across, down);
}

static int controller_style(SDL_GameController *c) {
    const char *deck = getenv("SteamDeck");
    const char *name = SDL_GameControllerName(c);
    if ((deck && *deck == '1') || (name && strstr(name, "Steam Deck")) ||
        (SDL_GameControllerGetVendor(c) == 0x28DE && SDL_GameControllerGetProduct(c) == 0x1205))
        return REVO_ICONS_DECK;
    switch (SDL_GameControllerGetType(c)) {
    case SDL_CONTROLLER_TYPE_PS3:
    case SDL_CONTROLLER_TYPE_PS4:
    case SDL_CONTROLLER_TYPE_PS5: return REVO_ICONS_PS2;
    case SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_PRO:
    case SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_LEFT:
    case SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_RIGHT:
    case SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_PAIR: return REVO_ICONS_SWITCH;
    default: return REVO_ICONS_XBOX;
    }
}

int revo_controller_style(void) { return g_ctrl_style.load(); }
bool revo_keyboard_last(void) { return g_keyboard_last.load(); }

static bool controller_at_rest(void) {
    if (!g_ctrl) return true;
    for (int b = 0; b < SDL_CONTROLLER_BUTTON_MAX; b++)
        if (SDL_GameControllerGetButton(g_ctrl, (SDL_GameControllerButton)b)) return false;
    return SDL_GameControllerGetAxis(g_ctrl, SDL_CONTROLLER_AXIS_TRIGGERLEFT) <= 12000 &&
           SDL_GameControllerGetAxis(g_ctrl, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) <= 12000;
}

static void present_once(void) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_KEYDOWN && !e.key.repeat) g_keyboard_last = true;
        if (e.type == SDL_CONTROLLERBUTTONDOWN ||
            (e.type == SDL_CONTROLLERAXISMOTION && (e.caxis.value > 16000 || e.caxis.value < -16000)))
            g_keyboard_last = false;
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_F12) {
            if (!e.key.repeat) g_shot_asked = true;
            continue;
        }
        if (e.type == SDL_KEYDOWN &&
            (e.key.keysym.sym == SDLK_F11 ||
             ((e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_KP_ENTER) && (e.key.keysym.mod & KMOD_ALT)))) {
            if (!e.key.repeat) revo_settings_toggle_fullscreen();
            continue;
        }
        if (revo_settings_event(e)) {
            if (revo_settings_open()) {
                g_pad = 0xFFFF;
                g_key_dirs = 0;
                g_skip_key = false;
            }
            continue;
        }
        if (e.type == SDL_CONTROLLERDEVICEADDED && !g_ctrl) {
            g_ctrl = SDL_GameControllerOpen(e.cdevice.which);
            if (g_ctrl) {
                g_ctrl_style = controller_style(g_ctrl);
                g_keyboard_last = false;
                static const char *const styles[] = {"PS2", "Xbox", "Switch", "Steam Deck"};
                revo_log("[platform] controller: %s (%s buttons)", SDL_GameControllerName(g_ctrl), styles[g_ctrl_style.load()]);
            }
        }
        if (e.type == SDL_CONTROLLERDEVICEREMOVED && g_ctrl &&
            e.cdevice.which == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(g_ctrl))) {
            SDL_GameControllerClose(g_ctrl);
            g_ctrl = nullptr;
            g_ctrl_style = REVO_ICONS_PS2;
            g_keyboard_last = true;
        }
        if (e.type == SDL_QUIT) g_quit = true;
        if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) {
            if (e.key.keysym.sym == SDLK_ESCAPE && e.type == SDL_KEYDOWN) g_quit = true;
            key(e.key.keysym.sym, e.type == SDL_KEYDOWN);
        }
    }
    {
        const bool unfocused = !(SDL_GetWindowFlags(g_window) & SDL_WINDOW_INPUT_FOCUS);
        static int paused = -1;
        const int pause = (revo_set(REVO_SET_PAUSE_FOCUS) && unfocused) || (revo_set(REVO_SET_PAUSE_MENU) && revo_settings_open());
        if (pause != paused) revo_pause(paused = pause);
    }
    if (int ms = g_rumble_ms.exchange(0)) {
        const int strength = g_rumble_strength.load();
        const Uint16 force = (Uint16)(strength * 65535 / 100);
        if (g_ctrl) SDL_GameControllerRumble(g_ctrl, force, force, (Uint32)ms);
    }
    if (revo_settings_open()) g_ctrl_hold = true;
    else if (g_ctrl_hold && controller_at_rest()) g_ctrl_hold = false;
    poll_controller();
    {
        static int shown = -1;
        const int want = !revo_window_fullscreen() || revo_settings_open();
        if (want != shown) SDL_ShowCursor((shown = want) ? SDL_ENABLE : SDL_DISABLE);
    }
    int w = 0, h = 0;
    SDL_GetWindowSizeInPixels(g_window, &w, &h);
    gli_present(w, h);
    if (g_shot_asked) {
        g_shot_asked = false;
        screenshot_now();
    }
    revo_settings_draw(w, h);
    gli_end_present(w, h);
}

extern "C" int revo_platform_should_quit(void) { return g_quit; }
extern "C" u64 revo_frame_count(void);

extern "C" u32 revo_input_now(void) {
    u32 held = ~(g_pad.load() & g_ctrl_buttons.load()) & 0xFFFF;
    const u32 c = g_ctrl_sticks.load();
    auto dirs = [](u32 x, u32 y) {
        return (x < 0x40 ? 1u : 0u) | (x > 0xC0 ? 2u : 0u) | (y < 0x40 ? 4u : 0u) | (y > 0xC0 ? 8u : 0u);
    };
    held |= (dirs(c >> 16 & 0xFF, c >> 24 & 0xFF) | g_key_dirs.load()) << 16;
    held |= dirs(c & 0xFF, c >> 8 & 0xFF) << 20;
    return held;
}

static u32 g_pad_reads;
extern "C" u32 revo_pad_reads(void) { return g_pad_reads; }

extern "C" int revo_skip_pressed(void) { return g_skip_key.load() || g_skip_ctrl.load(); }

static u32 skip_shared_bits(void) {
    u32 bits = 0;
    const int pad = revo_pad_binding(REVO_ACT_SKIP);
    for (int a = 0; a < REVO_ACT_COUNT; a++) {
        if (a == REVO_ACT_SKIP) continue;
        bool shared = g_skip_ctrl.load() && pad >= 0 && revo_pad_binding(a) == pad;
        for (int s = 0; s < 2 && g_skip_key.load(); s++) {
            const int k = revo_key_binding(REVO_ACT_SKIP, s);
            shared = shared || (k && (revo_key_binding(a, 0) == k || revo_key_binding(a, 1) == k));
        }
        if (shared) bits |= 1u << revo_action_bit(a);
    }
    return bits;
}

extern "C" int revo_skipping(void);
extern "C" u32 revo_camera_shoulders(void);
extern "C" u32 revo_pad_buttons(void) {
    u32 pressed = revo_camera_shoulders();
    g_pad_reads++;
    static bool skip_owned;
    if (!revo_skip_pressed()) skip_owned = false;
    else if (revo_skipping()) skip_owned = true;
    const u32 released = skip_owned ? skip_shared_bits() : 0;
    return ((g_pad.load() & g_ctrl_buttons.load() & ~pressed) | released) & 0xFFFF;
}

extern "C" void revo_pad_sticks(u8 *s) {
    u32 c = g_ctrl_sticks.load();
    s[0] = (u8)c;
    s[1] = (u8)(c >> 8);
    s[2] = (u8)(c >> 16);
    s[3] = (u8)(c >> 24);
    u32 held = g_key_dirs.load() << 16;
    bool l = held & 0x10000, r = held & 0x20000, u = held & 0x40000, d = held & 0x80000;
    if ((l || r) && s[2] > 0x60 && s[2] < 0xA0) s[2] = l ? 0x00 : 0xFF;
    if ((u || d) && s[3] > 0x60 && s[3] < 0xA0) s[3] = u ? 0x00 : 0xFF;
    if (held & 0xF00000) {
        const int a = 127;
        if (held & 0x300000) s[0] = (u8)(0x80 + (held & 0x100000 ? -a : a));
        if (held & 0xC00000) s[1] = (u8)(0x80 + (held & 0x400000 ? -a : a));
    }
    if (revo_mod(REVO_MOD_DIAGONAL)) {
        float x = (float)((int)s[2] - 0x80), y = (float)((int)s[3] - 0x80);
        float len = sqrtf(x * x + y * y);
        if (len > 128.0f) {
            float k = 128.0f / len;
            int nx = (int)lrintf(x * k), ny = (int)lrintf(y * k);
            s[2] = (u8)(0x80 + (nx < -128 ? -128 : nx > 127 ? 127 : nx));
            s[3] = (u8)(0x80 + (ny < -128 ? -128 : ny > 127 ? 127 : ny));
        }
    }
}
