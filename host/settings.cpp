#include <SDL.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#define revo_mkdir(path) _mkdir(path)
#else
#define revo_mkdir(path) mkdir((path), 0755)
#endif

#include <algorithm>
#include <atomic>
#include <cctype>
#include <cfloat>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <strings.h>
#include <string>
#include <vector>

#include "imgui.h"
#include "mods.h"
#include "settings.h"
#include "ui.h"
#include "update.h"

extern "C" {
#include "runtime.h"
void gli_set_resolution(int percent);
int gli_resolution(void);
unsigned long long render_frames_shown(void);
bool gli_vulkan_window(void);
const char *revo_current_level(void);
void revo_timer(u64 *run_ms, u64 *level_ms);
void revo_timer_reset(void);
}

namespace {

std::string g_path;
int g_saved_resolution = 100;

const int k_resolutions[] = {50, 67, 75, 100, 125, 150, 200, 300, 400};
const int k_nresolutions = (int)(sizeof k_resolutions / sizeof k_resolutions[0]);

const char *const k_mod_keys[REVO_MOD_COUNT] = {
    "diagonal",      "right_stick", "60fps",          "widescreen", "widescreen_1610", "interpolation", "skip_cutscenes",
    "short_loading", "skip_dialogues", "smooth",    "autosave",
};
bool g_mod[REVO_MOD_COUNT];
bool g_saved_mod[REVO_MOD_COUNT];
bool g_fps_counter;
bool g_fullscreen_saved;

struct SetInfo { const char *key; int def, min, max; };
const SetInfo k_sets[REVO_SET_COUNT] = {
    {"fov", 100, 70, 130},       {"hud_edges", 0, 0, 1},     {"no_bars", 0, 0, 1},
    {"scaling", 0, 0, 2},        {"post", 0, 0, 6},
    {"pause_unfocused", 0, 0, 1}, {"pause_menu", 0, 0, 1},   {"timer", 0, 0, 1},
    {"rumble", 0, 0, 100},       {"camera_invert_x", 0, 0, 1}, {"camera_invert_y", 0, 0, 1},
    {"camera_speed", 100, 25, 200}, {"volume", 100, 0, 100},
    {"draw_distance", 0, 0, 4},  {"button_icons", 0, 0, 5},
    {"texture_filter", 0, 0, 4}, {"wider", 0, 0, 2},
    {"discord", 1, 0, 1},        {"update_check", 1, 0, 1},
    {"cheat_speed", 0, 0, 2},    {"cheat_health", 0, 0, 1},  {"cheat_noclip", 0, 0, 1},
};
const float k_draw_distances[5] = {1.0f, 1.5f, 2.0f, 3.0f, 4.0f};
std::atomic<int> g_set[REVO_SET_COUNT];

struct Action { const char *name, *key; int bit; int keys[2]; int pad; };
const Action k_actions[REVO_ACT_COUNT] = {
    {"Cross", "cross", 14, {SDLK_k, SDLK_SPACE}, SDL_CONTROLLER_BUTTON_A},
    {"Square", "square", 15, {SDLK_j, 0}, SDL_CONTROLLER_BUTTON_X},
    {"Triangle", "triangle", 12, {SDLK_i, 0}, SDL_CONTROLLER_BUTTON_Y},
    {"Circle", "circle", 13, {SDLK_l, 0}, SDL_CONTROLLER_BUTTON_B},
    {"L1", "l1", 10, {SDLK_a, 0}, SDL_CONTROLLER_BUTTON_LEFTSHOULDER},
    {"R1", "r1", 11, {SDLK_d, 0}, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER},
    {"L2", "l2", 8, {SDLK_q, 0}, REVO_PAD_TRIGGER_LEFT},
    {"R2", "r2", 9, {SDLK_e, 0}, REVO_PAD_TRIGGER_RIGHT},
    {"Start", "start", 3, {SDLK_RETURN, 0}, SDL_CONTROLLER_BUTTON_START},
    {"Select", "select", 0, {SDLK_BACKSPACE, 0}, SDL_CONTROLLER_BUTTON_BACK},
    {"L3", "l3", 1, {0, 0}, SDL_CONTROLLER_BUTTON_LEFTSTICK},
    {"R3", "r3", 2, {0, 0}, SDL_CONTROLLER_BUTTON_RIGHTSTICK},
    {"Up", "up", 4, {SDLK_UP, 0}, SDL_CONTROLLER_BUTTON_DPAD_UP},
    {"Down", "down", 6, {SDLK_DOWN, 0}, SDL_CONTROLLER_BUTTON_DPAD_DOWN},
    {"Left", "left", 7, {SDLK_LEFT, 0}, SDL_CONTROLLER_BUTTON_DPAD_LEFT},
    {"Right", "right", 5, {SDLK_RIGHT, 0}, SDL_CONTROLLER_BUTTON_DPAD_RIGHT},
    {"Skip (held)", "skip", -1, {SDLK_i, 0}, SDL_CONTROLLER_BUTTON_Y},
};
std::atomic<int> g_keys[REVO_ACT_COUNT][2], g_pads[REVO_ACT_COUNT];

void default_controls() {
    for (int a = 0; a < REVO_ACT_COUNT; a++) {
        g_keys[a][0] = k_actions[a].keys[0];
        g_keys[a][1] = k_actions[a].keys[1];
        g_pads[a] = k_actions[a].pad;
    }
}

void save() {
    size_t slash = g_path.rfind('/');
    if (slash != std::string::npos && slash > 0) revo_mkdir(g_path.substr(0, slash).c_str());
    std::string tmp = g_path + ".new";
    FILE *f = fopen(tmp.c_str(), "w");
    bool ok = f != nullptr;
    if (f) {
        fprintf(f, "# Rayman Re-Revolution: changed by the in-game menu (F1)\n");
        fprintf(f, "resolution = %d\n", g_saved_resolution);
        fprintf(f, "fps_counter = %d\n", g_fps_counter ? 1 : 0);
        fprintf(f, "fullscreen = %d\n", g_fullscreen_saved ? 1 : 0);
        for (int m = 0; m < REVO_MOD_COUNT; m++) fprintf(f, "mod_%s = %d\n", k_mod_keys[m], g_saved_mod[m] ? 1 : 0);
        for (int s = 0; s < REVO_SET_COUNT; s++) fprintf(f, "%s = %d\n", k_sets[s].key, g_set[s].load());
        for (int a = 0; a < REVO_ACT_COUNT; a++) {
            fprintf(f, "key_%s = %d,%d\n", k_actions[a].key, g_keys[a][0].load(), g_keys[a][1].load());
            fprintf(f, "pad_%s = %d\n", k_actions[a].key, g_pads[a].load());
        }
        ok = fclose(f) == 0;
#ifdef _WIN32
        remove(g_path.c_str());
#endif
        ok = ok && rename(tmp.c_str(), g_path.c_str()) == 0;
    }
    if (ok) revo_log("[settings] saved in %s", g_path.c_str());
    else revo_log("[settings] cannot write %s", g_path.c_str());
}

void mods_from_environment() {
    const char *e = getenv("REVO_MODS");
    if (!e) return;
    std::string list = std::string(",") + e + ",";
    for (int m = 0; m < REVO_MOD_COUNT; m++)
        g_mod[m] = list.find(std::string(",") + k_mod_keys[m] + ",") != std::string::npos;
    if (g_mod[REVO_MOD_WIDESCREEN]) g_mod[REVO_MOD_WIDESCREEN_1610] = false;
    revo_log("[settings] REVO_MODS=%s", e);
}

std::string g_folder_note;

void folder_chosen(const char *to) {
    int had_saves = 0;
    if (!revo_user_dir_set(to, &had_saves))
        g_folder_note = "That folder cannot be used.";
    else if (had_saves)
        g_folder_note = "That folder has saves and settings of its own: they are used from the next start.";
    else
        g_folder_note = "Your saves and settings are copied there at the next start (the folder used until now is kept as it "
                        "is). Requires a restart.";
}

void game_copy(void) {
    ImGui::Spacing();
    ImGui::SeparatorText("Copy of the game");
    ImGui::TextDisabled("%s", revo_disc_image[0] ? revo_disc_image : revo_assets_dir);
    if (ImGui::Button("Choose another copy...")) revo_game_choose_again();
    ImGui::SameLine();
    ImGui::TextDisabled("The game starts again; what was not saved is lost.");
}

void data_folder(const char *dir) {
    ImGui::Spacing();
    ImGui::SeparatorText("Saves and settings folder");
    ImGui::TextDisabled("%s", dir);
#ifdef _WIN32
    if (ImGui::Button("Change the folder...")) {
        char to[1100];
        if (revo_user_dir_pick(to, sizeof to)) folder_chosen(to);
    }
    ImGui::SameLine();
    if (ImGui::Button("Next to the game (portable)")) folder_chosen(nullptr);
    ImGui::SameLine();
#endif
    if (ImGui::Button("Default folder")) folder_chosen("");
    if (!g_folder_note.empty()) {
        ImGui::PushTextWrapPos(0.0f);
        ImGui::TextUnformatted(g_folder_note.c_str());
        ImGui::PopTextWrapPos();
    }
}

enum { TAB_GRAPHICS, TAB_GAMEPLAY, TAB_CONTROLS, TAB_SYSTEM, TAB_EXTRAS, TABS };
const char *const k_tab_names[TABS] = {"Graphics", "Gameplay", "Controls", "System", "Extras"};

enum Opt {
    O_WINDOW, O_ASPECT, O_FOV, O_HUD_EDGES, O_NO_BARS, O_SCALE, O_SCALING, O_POST, O_TEXTURE_FILTER, O_DRAW_DISTANCE,
    O_FRAME_RATE,
    O_INTERPOLATION, O_FPS_COUNTER,
    O_DIAGONAL, O_RIGHT_STICK, O_CAM_INVERT_X, O_CAM_INVERT_Y, O_CAM_SPEED, O_RUMBLE, O_BUTTON_ICONS, O_SKIP_CUTSCENES,
    O_SKIP_DIALOGUES,
    O_PAUSE_FOCUS, O_PAUSE_MENU, O_VOLUME, O_SHORT_LOADING, O_AUTOSAVE, O_DISCORD, O_UPDATE_CHECK,
    O_TIMER, O_SPEED, O_INF_HEALTH, O_NOCLIP,
    O_COUNT
};
enum Kind { TOGGLE, LIST, SLIDER };
struct OptInfo { int tab; const char *group, *label, *help; Kind kind; int mod, set; bool vulkan; };
const OptInfo k_opts[O_COUNT] = {
    {TAB_GRAPHICS, "Display", "Window mode", "Full screen at the desktop's resolution, press F11 or Alt+Enter.", LIST, -1, -1, false},
    {TAB_GRAPHICS, nullptr, "Aspect ratio",
     "16:9, 16:10, 21:9. " , LIST, -1, -1, false},
    {TAB_GRAPHICS, nullptr, "Field of view (FOV)",
     "FOV percentage.", SLIDER, -1,
     REVO_SET_FOV, false},
    {TAB_GRAPHICS, nullptr, "HUD at the sides",
     "Puts the HUD on the side, I recommend activating it if you're using something else than 4:3.",
     TOGGLE, -1, REVO_SET_HUD_EDGES, false},
    {TAB_GRAPHICS, nullptr, "No black bars",
     "Removes the black bars that appear in widescreen mode.", TOGGLE, -1,
     REVO_SET_NO_BARS, false},
    {TAB_GRAPHICS, "Rendering", "Resolution", "", LIST, -1, -1, false},
    {TAB_GRAPHICS, nullptr, "Scaling filter",
     "Bicubic or SSAA.",
     LIST, -1, REVO_SET_SCALING, false},
    {TAB_GRAPHICS, nullptr, "Post effect",
     "Sharpen, FXAA, vivid colours, or a TV look (CRT, curved CRT or composite)", LIST, -1,
     REVO_SET_POST, false},
    {TAB_GRAPHICS, nullptr, "Texture filtering",
     "Your average texture filtering option.",
     LIST, -1, REVO_SET_TEXTURE_FILTER, true},
    {TAB_GRAPHICS, nullptr, "Draw distance",
     "Increase the distance at which objects are rendered, I personally recommend staying at x2, it tends "
     "to show some of the \"hacks\" the game uses otherwise.",
     LIST, -1, REVO_SET_DRAW_DISTANCE, false},
    {TAB_GRAPHICS, "Frame rate", "Frame rate",
     "Changes the framerate to either 30fps (the original game's framerate) or 60. You can tick frame intepolation to get above 60fps.",
     LIST, -1, -1, false},
    {TAB_GRAPHICS, nullptr, "Frame interpolation (to go above 60fps)",
     "Uses frame interpolation to make the game go above 60fps.", TOGGLE,
     REVO_MOD_INTERPOLATION, -1, true},
    {TAB_GRAPHICS, nullptr, "FPS counter", "Shown in the top left. (Works with F2 too)", TOGGLE, -1, -1,
     false},
    {TAB_GAMEPLAY, "Controls", "Diagonal speed fix",
     "Fixes the infamous bug of Revolution where Rayman goes faster when running diagonally (please "
     "activate that).",
     TOGGLE, REVO_MOD_DIAGONAL, -1, false},
    {TAB_GAMEPLAY, nullptr, "Right stick camera (experimental)",
     "Tries putting the camera to the right stick and at 360°, compared to Revolution's original L1/R1 commands.",
     TOGGLE, REVO_MOD_RIGHT_STICK, -1, false},
    {TAB_GAMEPLAY, nullptr, "Invert camera X", "", TOGGLE, -1,
     REVO_SET_CAM_INVERT_X, false},
    {TAB_GAMEPLAY, nullptr, "Invert camera Y", "", TOGGLE, -1,
     REVO_SET_CAM_INVERT_Y, false},
    {TAB_GAMEPLAY, nullptr, "Camera speed", "How fast the right stick camera moves.", SLIDER, -1,
     REVO_SET_CAM_SPEED, false},
    {TAB_GAMEPLAY, nullptr, "Vibration (experimental)",
     "Right now it only vibrates when Rayman gets hit or dies.",
     SLIDER, -1, REVO_SET_RUMBLE, false},
    {TAB_GAMEPLAY, nullptr, "Button icons",
     "Changes the buttons prompts to the current selection. They follow the Controls tab's bindings. "
     "(Automatic = detects the prompts from the controller plugged in)",
     LIST, -1, REVO_SET_BUTTON_ICONS, false},
    {TAB_GAMEPLAY, "Cutscenes and dialogues", "Fast-forward cutscenes",
     "Hold Skip (Triangle by default, see Controls) in a cutscene to run it fast/muted.", TOGGLE, REVO_MOD_SKIP_CUTSCENES, -1, false},
    {TAB_GAMEPLAY, nullptr, "Fast-forward dialogues",
     "Hold Skip (Triangle by default, see Controls) to move on the lines that wait for a button.",
     TOGGLE, REVO_MOD_SKIP_DIALOGUES, -1, false},
    {TAB_SYSTEM, "Pause", "Pause when in the background",
     "", TOGGLE, -1, REVO_SET_PAUSE_FOCUS, false},
    {TAB_SYSTEM, nullptr, "Pause in this menu", "", TOGGLE, -1,
     REVO_SET_PAUSE_MENU, false},
    {TAB_SYSTEM, "Sound", "Master volume",
     "The game's volume. (SFX/Music separation is in the game's regular settings)", SLIDER, -1,
     REVO_SET_VOLUME, false},
    {TAB_SYSTEM, "Loading and saves", "No loadings", "Removes all the fuss about the loadings (as now they instant-load).",
     TOGGLE, REVO_MOD_SHORT_LOADING, -1, false},
    {TAB_SYSTEM, nullptr, "Autosave",
     "Removes the annoying save prompts and automatically saves.",
     TOGGLE, REVO_MOD_AUTOSAVE, -1, false},
    {TAB_SYSTEM, "Discord", "Rich Presence",
     "Shows on your Discord profile that you are playing along with the current level, your lums and "
     "cages.",
     TOGGLE, -1, REVO_SET_DISCORD, false},
    {TAB_SYSTEM, "Updates", "Check for updates at start",
     "Keeps the settings.",
     TOGGLE, -1, REVO_SET_UPDATE_CHECK, false},
    {TAB_EXTRAS, "Speedrun", "Timer",
     "Adds a timer and the level at the Bottom right.",
     TOGGLE, -1, REVO_SET_TIMER, false},
    {TAB_EXTRAS, "Cheats (experimental)", "Rayman's speed",
     "Rayman moves two or three times as fast, right now the jump/fall speed is also increased.", LIST, -1,
     REVO_SET_SPEED, false},
    {TAB_EXTRAS, nullptr, "Infinite health", "Infinite health.", TOGGLE,
     -1, REVO_SET_INF_HEALTH, false},
    {TAB_EXTRAS, nullptr, "Noclip",
     "Rayman flies through walls. F3 or L1 + R1 + L2 + R2 turns it on and off, Cross / Square go up and down, "
     "R2 goes faster.",
     TOGGLE, -1, REVO_SET_NOCLIP, false},
};

bool g_open;
int g_tab;
int g_tab_asked = -1;
bool g_focus;

struct { int action = -1, slot; } g_capture;
int g_pad_held;
bool g_pad_quiet;

void picture_size(int width, int height, int *pw, int *ph) {
    int across, down;
    revo_view_aspect(&across, &down);
    *pw = width;
    *ph = width * down / across;
    if (*ph > height) { *ph = height; *pw = height * across / down; }
}

bool widescreen() { return g_mod[REVO_MOD_WIDESCREEN] || g_mod[REVO_MOD_WIDESCREEN_1610]; }

void screen_shape(int *across, int *down) {
    static int w, h;
    if (!w) {
        SDL_DisplayMode m;
        if (SDL_GetDesktopDisplayMode(0, &m) == 0 && m.w > 0 && m.h > 0) w = m.w, h = m.h;
        else w = 16, h = 9;
        if (w * 3 < h * 4) w = 4, h = 3;
        int a = w, b = h;
        while (b) { const int r = a % b; a = b; b = r; }
        w /= a;
        h /= a;
        while (w > 4096 || h > 4096) w /= 2, h /= 2;
    }
    *across = w;
    *down = h;
}

int values(int o) {
    switch (o) {
    case O_ASPECT: return 5;
    case O_FRAME_RATE: return 2;
    case O_SCALING: return 3;
    case O_POST: return 7;
    case O_SCALE: return k_nresolutions;
    case O_DRAW_DISTANCE: case O_TEXTURE_FILTER: return 5;
    case O_BUTTON_ICONS: return 6;
    case O_SPEED: return 3;
    default: return k_opts[o].kind == SLIDER ? k_sets[k_opts[o].set].max + 1 : 2;
    }
}

int value(int o) {
    switch (o) {
    case O_WINDOW: return revo_window_fullscreen() ? 1 : 0;
    case O_ASPECT:
        if (g_mod[REVO_MOD_WIDESCREEN]) return g_set[REVO_SET_WIDER] == 1 ? 3 : g_set[REVO_SET_WIDER] == 2 ? 4 : 1;
        return g_mod[REVO_MOD_WIDESCREEN_1610] ? 2 : 0;
    case O_SCALE: {
        int best = 0;
        for (int k = 1; k < k_nresolutions; k++)
            if (std::abs(k_resolutions[k] - gli_resolution()) < std::abs(k_resolutions[best] - gli_resolution())) best = k;
        return best;
    }
    case O_SCALING: return g_set[REVO_SET_SCALING] == 2 ? 2 : g_mod[REVO_MOD_SMOOTH] ? 1 : 0;
    case O_FRAME_RATE: return g_mod[REVO_MOD_60FPS] ? 1 : 0;
    case O_FPS_COUNTER: return g_fps_counter ? 1 : 0;
    default: return k_opts[o].set >= 0 ? g_set[k_opts[o].set].load() : g_mod[k_opts[o].mod] ? 1 : 0;
    }
}

std::string value_name(int o, int v, int width, int height) {
    char t[64];
    switch (o) {
    case O_WINDOW: return v ? "Full screen" : "Windowed";
    case O_ASPECT:
        if (v == 4) {
            int across, down;
            screen_shape(&across, &down);
            snprintf(t, sizeof t, "The screen's (%.2f:1)", (double)across / (double)down);
            return t;
        }
        return v == 1 ? "16:9" : v == 2 ? "16:10" : v == 3 ? "21:9" : "4:3 (original)";
    case O_SCALE: {
        int pw, ph;
        picture_size(width, height, &pw, &ph);
        const int pc = k_resolutions[v];
        snprintf(t, sizeof t, "%d%%  (%d x %d)%s", pc, (pw * pc + 50) / 100, (ph * pc + 50) / 100,
                 pc == 100 ? "  - the window's" : "");
        return t;
    }
    case O_SCALING: return v == 2 ? "Bicubic" : v == 1 ? "Smooth (SSAA)" : "Sharp";
    case O_POST: {
        static const char *const names[7] = {"None", "Sharpen", "CRT", "FXAA (smooths edges)", "Vivid colours",
                                             "CRT (curved)", "Composite TV"};
        return names[v];
    }
    case O_FRAME_RATE: return v == 1 ? "60 fps" : "30 fps (original)";
    case O_BUTTON_ICONS: {
        static const char *const names[6] = {"Automatic", "PS2 (original)", "Xbox", "Nintendo Switch", "Steam Deck",
                                             "Keyboard"};
        if (v != 0) return names[v];
        static const char *const styles[5] = {"PS2", "Xbox", "Nintendo Switch", "Steam Deck", "Keyboard"};
        return std::string("Automatic (") + styles[revo_keyboard_last() ? REVO_ICONS_KEYBOARD : revo_controller_style()] + ")";
    }
    case O_TEXTURE_FILTER: {
        static const char *const names[5] = {"Original", "Trilinear", "Anisotropic 4x", "Anisotropic 8x",
                                             "Anisotropic 16x"};
        return names[v];
    }
    case O_DRAW_DISTANCE: return v == 0 ? "x1 (original)" : v == 1 ? "x1.5" : v == 2 ? "x2" : v == 3 ? "x3" : "x4";
    case O_SPEED: return v == 0 ? "x1 (original)" : v == 1 ? "x2" : "x3";
    default: return v ? "On" : "Off";
    }
}

void set_mod(int m, bool on) {
    if (g_mod[m] == on) return;
    g_mod[m] = g_saved_mod[m] = on;
    revo_log("[settings] mod %s %s", k_mod_keys[m], on ? "on" : "off");
}

void choose_resolution(int percent) {
    gli_set_resolution(percent);
    if (g_saved_resolution == percent) return;
    g_saved_resolution = percent;
    save();
}

void toggle_fps_counter() {
    g_fps_counter = !g_fps_counter;
    revo_log("[settings] frame rate counter %s", g_fps_counter ? "on" : "off");
    save();
}

void set_fullscreen(bool on) {
    revo_window_set_fullscreen(on);
    g_fullscreen_saved = revo_window_fullscreen();
    save();
}

void set_value(int o, int v) {
    if (v == value(o)) return;
    const OptInfo &info = k_opts[o];
    switch (o) {
    case O_WINDOW: set_fullscreen(v == 1); return;
    case O_SCALE: choose_resolution(k_resolutions[v]); return;
    case O_FPS_COUNTER: toggle_fps_counter(); return;
    case O_ASPECT:
        set_mod(REVO_MOD_WIDESCREEN, v == 1 || v >= 3);
        set_mod(REVO_MOD_WIDESCREEN_1610, v == 2);
        g_set[REVO_SET_WIDER] = v == 3 ? 1 : v == 4 ? 2 : 0;
        revo_window_fit_aspect();
        break;
    case O_FRAME_RATE:
        set_mod(REVO_MOD_60FPS, v == 1);
        break;
    case O_SCALING:
        set_mod(REVO_MOD_SMOOTH, v >= 1);
        g_set[REVO_SET_SCALING] = v;
        break;
    default:
        if (info.set >= 0) {
            const SetInfo &s = k_sets[info.set];
            g_set[info.set] = std::max(s.min, std::min(s.max, v));
            revo_log("[settings] %s = %d", s.key, g_set[info.set].load());
        } else {
            set_mod(info.mod, v == 1);
        }
        break;
    }
    save();
}

// L1 + R1 + L2 + R2 held together turn noclip on or off, as F3 does.
static void noclip_combo(SDL_GameController *c) {
    static bool held;
    const Sint16 pulled = 16000;
    const bool all = c && !g_open && SDL_GameControllerGetButton(c, SDL_CONTROLLER_BUTTON_LEFTSHOULDER) &&
                     SDL_GameControllerGetButton(c, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) &&
                     SDL_GameControllerGetAxis(c, SDL_CONTROLLER_AXIS_TRIGGERLEFT) > pulled &&
                     SDL_GameControllerGetAxis(c, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) > pulled;
    if (all && !held) set_value(O_NOCLIP, value(O_NOCLIP) ? 0 : 1);
    held = all;
}

void open_menu(bool on) {
    if (g_open != on) revo_log("[settings] menu %s", on ? "open" : "closed");
    g_open = on;
    g_focus = on;
    if (!on) g_capture.action = -1;
}

void switch_tab(int by) {
    g_tab_asked = ((g_tab + by) % TABS + TABS) % TABS;
}

bool popup_open() {
    return ImGui::GetCurrentContext() && ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel);
}

std::string key_name(int k) {
    if (!k) return "-";
    const char *n = SDL_GetKeyName((SDL_Keycode)k);
    return n && *n ? n : "?";
}

std::string pad_name(int p) {
    if (revo_controller_style() == REVO_ICONS_SWITCH) {
        switch (p) {
        case SDL_CONTROLLER_BUTTON_A: return "B (bottom)";
        case SDL_CONTROLLER_BUTTON_B: return "A (right)";
        case SDL_CONTROLLER_BUTTON_X: return "Y (left)";
        case SDL_CONTROLLER_BUTTON_Y: return "X (top)";
        case SDL_CONTROLLER_BUTTON_BACK: return "-";
        case SDL_CONTROLLER_BUTTON_START: return "+";
        case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: return "L";
        case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: return "R";
        case REVO_PAD_TRIGGER_LEFT: return "ZL";
        case REVO_PAD_TRIGGER_RIGHT: return "ZR";
        }
    }
    switch (p) {
    case -1: return "-";
    case REVO_PAD_TRIGGER_LEFT: return "Left trigger";
    case REVO_PAD_TRIGGER_RIGHT: return "Right trigger";
    case SDL_CONTROLLER_BUTTON_A: return "A / Cross";
    case SDL_CONTROLLER_BUTTON_B: return "B / Circle";
    case SDL_CONTROLLER_BUTTON_X: return "X / Square";
    case SDL_CONTROLLER_BUTTON_Y: return "Y / Triangle";
    case SDL_CONTROLLER_BUTTON_BACK: return "Back / Share";
    case SDL_CONTROLLER_BUTTON_START: return "Start / Options";
    case SDL_CONTROLLER_BUTTON_LEFTSTICK: return "Left stick click";
    case SDL_CONTROLLER_BUTTON_RIGHTSTICK: return "Right stick click";
    case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: return "Left shoulder";
    case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: return "Right shoulder";
    case SDL_CONTROLLER_BUTTON_DPAD_UP: return "D-pad up";
    case SDL_CONTROLLER_BUTTON_DPAD_DOWN: return "D-pad down";
    case SDL_CONTROLLER_BUTTON_DPAD_LEFT: return "D-pad left";
    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: return "D-pad right";
    default: {
        const char *n = SDL_GameControllerGetStringForButton((SDL_GameControllerButton)p);
        return n ? n : "?";
    }
    }
}

bool reserved_key(SDL_Keycode k) {
    return k == SDLK_F1 || k == SDLK_F2 || k == SDLK_F3 || k == SDLK_F11 || k == SDLK_F12 || k == SDLK_ESCAPE;
}

void bind_key(int action, int slot, int k) {
    for (int a = 0; a < REVO_ACT_COUNT; a++)
        for (int s = 0; s < 2; s++)
            if (g_keys[a][s] == k && (a == REVO_ACT_SKIP) == (action == REVO_ACT_SKIP)) g_keys[a][s] = 0;
    g_keys[action][slot] = k;
    revo_log("[settings] %s: key %s", k_actions[action].name, key_name(k).c_str());
    save();
}

void bind_pad(int action, int p) {
    for (int a = 0; a < REVO_ACT_COUNT; a++)
        if (g_pads[a] == p && (a == REVO_ACT_SKIP) == (action == REVO_ACT_SKIP)) g_pads[a] = -1;
    g_pads[action] = p;
    revo_log("[settings] %s: controller %s", k_actions[action].name, pad_name(p).c_str());
    save();
}

int frame_rate() {
    static Uint64 since;
    static unsigned long long shown_since;
    static int rate;
    const Uint64 now = SDL_GetPerformanceCounter();
    const unsigned long long shown = render_frames_shown();
    if (!since) {
        since = now;
        shown_since = shown;
    }
    const double secs = (double)(now - since) / (double)SDL_GetPerformanceFrequency();
    if (secs >= 0.5) {
        rate = (int)((double)(shown - shown_since) / secs + 0.5);
        since = now;
        shown_since = shown;
    }
    return rate;
}

int shown_rate() {
    static Uint64 since;
    static int count, rate;
    const Uint64 now = SDL_GetPerformanceCounter();
    if (!since) since = now;
    count++;
    const double secs = (double)(now - since) / (double)SDL_GetPerformanceFrequency();
    if (secs >= 0.5) {
        rate = (int)(count / secs + 0.5);
        count = 0;
        since = now;
    }
    return rate;
}

void badge(const char *t, ImVec2 at, bool right, ImU32 color) {
    ImDrawList *d = ImGui::GetForegroundDrawList();
    const float s = revo_ui_scale(), pad = 4 * s;
    const ImVec2 size = ImGui::CalcTextSize(t);
    if (right) at.x -= size.x;
    d->AddRectFilled(ImVec2(at.x - pad, at.y - pad), ImVec2(at.x + size.x + pad, at.y + size.y + pad),
                     IM_COL32(0, 0, 0, 150), 4 * s);
    d->AddText(at, color, t);
}

void fps_counter(int rate, int shown) {
    char t[48];
    if (revo_mod(REVO_MOD_INTERPOLATION) && shown > rate) snprintf(t, sizeof t, "%d FPS (%d shown)", rate, shown);
    else snprintf(t, sizeof t, "%d FPS", rate);
    const float s = revo_ui_scale();
    badge(t, ImVec2(8 * s, 8 * s), false, IM_COL32(255, 204, 64, 255));
}

std::string clock_text(u64 ms) {
    char t[32];
    const unsigned h = (unsigned)(ms / 3600000), m = (unsigned)(ms / 60000 % 60), sec = (unsigned)(ms / 1000 % 60),
                   cs = (unsigned)(ms / 10 % 100);
    if (h) snprintf(t, sizeof t, "%u:%02u:%02u.%02u", h, m, sec, cs);
    else snprintf(t, sizeof t, "%u:%02u.%02u", m, sec, cs);
    return t;
}

void speedrun_timer() {
    u64 run, level;
    revo_timer(&run, &level);
    const float s = revo_ui_scale(), line = ImGui::GetTextLineHeight() + 10 * s;
    const ImVec2 at(ImGui::GetIO().DisplaySize.x - 8 * s, ImGui::GetIO().DisplaySize.y - 8 * s - 2 * line);
    const std::string top = clock_text(run);
    badge(top.c_str(), at, true, IM_COL32(255, 255, 255, 255));
    const char *name = revo_current_level();
    if (name[0] && strncasecmp(name, "menu", 4)) {
        const std::string under = std::string(name) + "  " + clock_text(level);
        badge(under.c_str(), ImVec2(at.x, at.y + line), true, IM_COL32(190, 190, 200, 255));
    }
}

/* Bottom right while a cheat is on, above the timer when it is shown. */
void extras_badge(bool timer) {
    const float s = revo_ui_scale(), line = ImGui::GetTextLineHeight() + 10 * s;
    const float bottom = ImGui::GetIO().DisplaySize.y - 8 * s - ImGui::GetTextLineHeight();
    badge("Extras activated", ImVec2(ImGui::GetIO().DisplaySize.x - 8 * s, timer ? bottom - 2 * line : bottom), true,
          IM_COL32(255, 200, 90, 255));
}

extern "C" u32 revo_autosave_count(const char **words);
extern "C" int gli_hud_words_draw(float opacity, int right, int bottom, int height);
float saving_opacity(const char **words) {
    static u32 seen;
    static Uint64 since;
    const u32 saves = revo_autosave_count(words);
    const Uint64 now = SDL_GetPerformanceCounter();
    if (saves != seen) {
        seen = saves;
        since = now;
    }
    if (!since) return 0.0f;
    const double secs = (double)(now - since) / (double)SDL_GetPerformanceFrequency();
    if (secs >= 2.5) {
        since = 0;
        return 0.0f;
    }
    return secs < 2.0 ? 1.0f : (float)((2.5 - secs) / 0.5);
}

void saving_badge(const char *words, float opacity, int above) {
    ImDrawList *d = ImGui::GetForegroundDrawList();
    const float s = revo_ui_scale(), pad = 4 * s, line = ImGui::GetTextLineHeight() + 10 * s;
    const ImVec2 size = ImGui::CalcTextSize(words);
    const float bottom = ImGui::GetIO().DisplaySize.y - 8 * s;
    const ImVec2 at(ImGui::GetIO().DisplaySize.x - 8 * s - size.x, above ? bottom - (float)(above + 1) * line : bottom - size.y);
    const ImVec2 px = ImGui::GetIO().DisplayFramebufferScale;
    const float tall = ImGui::GetIO().DisplaySize.y * 0.055f;
    const float under = above ? at.y + size.y : bottom;
    if (gli_hud_words_draw(opacity, (int)((ImGui::GetIO().DisplaySize.x - 8 * s) * px.x + 0.5f),
                           (int)(under * px.y + 0.5f), (int)(tall * px.y + 0.5f)))
        return;
    d->AddRectFilled(ImVec2(at.x - pad, at.y - pad), ImVec2(at.x + size.x + pad, at.y + size.y + pad),
                     IM_COL32(0, 0, 0, (int)(150 * opacity)), 4 * s);
    d->AddText(at, IM_COL32(255, 255, 255, (int)(255 * opacity)), words);
}

// Where the values start: past the widest label, so that none runs under its value.
static float value_column() {
    float widest = 0.0f;
    for (const OptInfo &info : k_opts) widest = std::max(widest, ImGui::CalcTextSize(info.label).x);
    return std::max(270.0f * revo_ui_scale(), widest + 2.0f * ImGui::GetStyle().ItemSpacing.x);
}

bool option(int o, int width, int height) {
    const OptInfo &info = k_opts[o];
    if (o == O_DISCORD && !revo_discord_available()) return false;
    if (info.group) {
        if (o != O_WINDOW && o != O_DIAGONAL && o != O_PAUSE_FOCUS && o != O_TIMER) ImGui::Spacing();
        ImGui::SeparatorText(info.group);
    }
    const bool unable = (info.vulkan && !gli_vulkan_window()) || ((o == O_HUD_EDGES || o == O_NO_BARS) && !widescreen()) ||
                        ((o == O_CAM_INVERT_X || o == O_CAM_INVERT_Y || o == O_CAM_SPEED) && !g_mod[REVO_MOD_RIGHT_STICK]);
    ImGui::BeginDisabled(unable);
    ImGui::PushID(o);
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(info.label);
    ImGui::SameLine(value_column());
    ImGui::SetNextItemWidth(-FLT_MIN);
    const int v = value(o);
    bool on = false;
    if (info.kind == TOGGLE) {
        bool b = v == 1;
        if (ImGui::Checkbox("##v", &b)) set_value(o, b ? 1 : 0);
        on = ImGui::IsItemHovered() || ImGui::IsItemFocused();
        if (unable && info.vulkan) {
            ImGui::SameLine();
            ImGui::TextDisabled("(Vulkan only)");
        }
    } else if (info.kind == SLIDER) {
        const SetInfo &s = k_sets[info.set];
        int x = v;
        const char *format = o == O_RUMBLE && x == 0 ? "Off" : "%d %%";
        if (ImGui::SliderInt("##v", &x, s.min, s.max, format, ImGuiSliderFlags_AlwaysClamp)) set_value(o, x);
        on = ImGui::IsItemHovered() || ImGui::IsItemFocused();
    } else {
        if (ImGui::BeginCombo("##v", value_name(o, v, width, height).c_str())) {
            for (int k = 0; k < values(o); k++) {
                const bool chosen = k == v;
                if (ImGui::Selectable(value_name(o, k, width, height).c_str(), chosen)) set_value(o, k);
                if (chosen) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        on = ImGui::IsItemHovered() || ImGui::IsItemFocused();
    }
    ImGui::PopID();
    ImGui::EndDisabled();
    if (info.mod >= 0 && getenv("REVO_MODS") && on) ImGui::SetTooltip("REVO_MODS set it for this run");
    return on;
}

std::string help(int o, int width, int height) {
    if (o != O_SCALE) return k_opts[o].help;
    char t[256];
    (void)width;
    (void)height;
    snprintf(t, sizeof t,
             "Resolution the game is drawn at.%s",
             getenv("REVO_RESOLUTION") ? " REVO_RESOLUTION set it at start." : "");
    return t;
}

const char *controls_tab() {
    const float s = revo_ui_scale();
    const char *helped = nullptr;
    if (g_capture.action >= 0) {
        ImGui::TextColored(ImVec4(0.95f, 0.70f, 0.15f, 1.0f), "%s: %s", k_actions[g_capture.action].name,
                           g_capture.slot == 2 ? "press a controller button (Guide: cancel, the same button again: none)"
                                               : "press a key (Esc: cancel, Delete: none)");
    } else {
        ImGui::TextDisabled("Choose a box, then press the key or the button to bind.");
    }
    if (ImGui::BeginTable("controls", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH)) {
        ImGui::TableSetupColumn("PS2 button", ImGuiTableColumnFlags_WidthFixed, 110 * s);
        ImGui::TableSetupColumn("Key", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Second key", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Controller", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();
        for (int a = 0; a < REVO_ACT_COUNT; a++) {
            ImGui::PushID(a);
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted(k_actions[a].name);
            for (int slot = 0; slot < 3; slot++) {
                ImGui::TableNextColumn();
                const bool capturing = g_capture.action == a && g_capture.slot == slot;
                std::string label = capturing ? "..." : slot < 2 ? key_name(g_keys[a][slot]) : pad_name(g_pads[a]);
                label += "##" + std::to_string(slot);
                if (ImGui::Button(label.c_str(), ImVec2(-FLT_MIN, 0)) && !g_pad_quiet) g_capture = {a, slot};
                if (ImGui::IsItemHovered() || ImGui::IsItemFocused()) helped = k_actions[a].name;
            }
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    ImGui::Spacing();
    if (ImGui::Button("Back to the defaults")) {
        default_controls();
        g_capture.action = -1;
        save();
    }
    return helped;
}

void menu(int width, int height) {
    const float s = revo_ui_scale();
    const ImGuiViewport *vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(vp->WorkPos.x + vp->WorkSize.x * 0.5f, vp->WorkPos.y + vp->WorkSize.y * 0.5f),
                            ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    const float w = std::min(vp->WorkSize.x * 0.95f, 760.0f * s);
    ImGui::SetNextWindowSizeConstraints(ImVec2(w, 0), ImVec2(w, FLT_MAX));
    static float last_scale;
    if (s != last_scale) g_focus = true;
    last_scale = s;
    if (g_focus) {
        ImGui::SetNextWindowFocus();
        g_focus = false;
    }
    char title[64];
    snprintf(title, sizeof title, "Rayman Re-Revolution - Settings###menu%d", (int)(s * 100.0f + 0.5f));
    bool open = true;
    if (ImGui::Begin(title, &open,
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar)) {
        static int helped = O_WINDOW;
        const char *controls_help = nullptr;
        if (ImGui::BeginTabBar("tabs")) {
            for (int t = 0; t < TABS; t++) {
                const ImGuiTabItemFlags f = g_tab_asked == t ? ImGuiTabItemFlags_SetSelected : 0;
                if (ImGui::BeginTabItem(k_tab_names[t], nullptr, f)) {
                    if (g_tab != t) {
                        helped = -1;
                        for (int o = 0; o < O_COUNT && helped < 0; o++)
                            if (k_opts[o].tab == t) helped = o;
                        g_capture.action = -1;
                    }
                    g_tab = t;
                    if (t == TAB_CONTROLS) {
                        controls_help = controls_tab();
                    } else {
                        for (int o = 0; o < O_COUNT; o++)
                            if (k_opts[o].tab == t && option(o, width, height)) helped = o;
                        if (t == TAB_EXTRAS) {
                            if (ImGui::Button("Reset the timer")) revo_timer_reset();
                        }
                        if (t == TAB_SYSTEM) {
                            game_copy();
                            revo_update_menu();
                            char dir[1024];
                            revo_settings_dir(dir, sizeof dir);
                            ImGui::Spacing();
                            ImGui::SeparatorText("Screenshots");
                            ImGui::TextDisabled("F12: the game's picture in %s/screenshots", dir);
                            data_folder(dir);
                        }
                    }
                    ImGui::EndTabItem();
                }
            }
            g_tab_asked = -1;
            ImGui::EndTabBar();
        }
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::PushTextWrapPos(0.0f);
        if (g_tab == TAB_CONTROLS) {
            ImGui::TextUnformatted(controls_help ? controls_help : "Controls");
            ImGui::TextDisabled("You can remap two keys.");
        } else if (helped >= 0 && k_opts[helped].tab == g_tab) {
            ImGui::TextUnformatted(k_opts[helped].label);
            ImGui::TextDisabled("%s", help(helped, width, height).c_str());
        }
        ImGui::PopTextWrapPos();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextDisabled("F1 / L3+R3: close    Q/E, LB/RB: tab    F11: full screen    F12: screenshot");
    }
    ImGui::End();
    if (!open) open_menu(false);
}

}

void revo_settings_dir(char *out, size_t size) {
    const size_t slash = g_path.find_last_of("/\\");
    const std::string dir = slash == std::string::npos ? std::string(".") : g_path.substr(0, slash);
    snprintf(out, size, "%s", dir.c_str());
}

void revo_settings_load(void) {
    const char *e = getenv("REVO_SETTINGS");
    for (int s = 0; s < REVO_SET_COUNT; s++) g_set[s] = k_sets[s].def;
    default_controls();
    char dir[1024];
    revo_user_dir(dir, sizeof dir);
    g_path = e && *e ? e : std::string(dir) + "/settings.ini";
    if (FILE *f = fopen(g_path.c_str(), "r")) {
        char line[256], key[64];
        int value, second;
        while (fgets(line, sizeof line, f)) {
            const int got = sscanf(line, " %63[a-z0-9_] = %d , %d", key, &value, &second);
            if (got < 2) continue;
            if (!strcmp(key, "resolution")) g_saved_resolution = std::max(25, std::min(value, 400));
            if (!strcmp(key, "fps_counter")) g_fps_counter = value != 0;
            if (!strcmp(key, "fullscreen")) g_fullscreen_saved = value != 0;
            for (int m = 0; m < REVO_MOD_COUNT; m++)
                if (!strncmp(key, "mod_", 4) && !strcmp(key + 4, k_mod_keys[m])) g_saved_mod[m] = value != 0;
            for (int s = 0; s < REVO_SET_COUNT; s++)
                if (!strcmp(key, k_sets[s].key)) g_set[s] = std::max(k_sets[s].min, std::min(k_sets[s].max, value));
            for (int a = 0; a < REVO_ACT_COUNT; a++) {
                if (!strncmp(key, "key_", 4) && !strcmp(key + 4, k_actions[a].key)) {
                    g_keys[a][0] = value;
                    g_keys[a][1] = got == 3 ? second : 0;
                }
                if (!strncmp(key, "pad_", 4) && !strcmp(key + 4, k_actions[a].key)) g_pads[a] = value;
            }
        }
        fclose(f);
        std::string on;
        for (int m = 0; m < REVO_MOD_COUNT; m++)
            if (g_saved_mod[m]) on += std::string(on.empty() ? "" : ", ") + k_mod_keys[m];
        revo_log("[settings] %s: resolution %d%%, %s, mods %s", g_path.c_str(), g_saved_resolution,
                 g_fullscreen_saved ? "full screen" : "windowed", on.empty() ? "off" : on.c_str());
    }
    if (!getenv("REVO_RESOLUTION")) gli_set_resolution(g_saved_resolution);
    if (g_saved_mod[REVO_MOD_WIDESCREEN]) g_saved_mod[REVO_MOD_WIDESCREEN_1610] = false;
    if (!g_saved_mod[REVO_MOD_SMOOTH] && g_set[REVO_SET_SCALING] > 0) g_set[REVO_SET_SCALING] = 0;
    for (int m = 0; m < REVO_MOD_COUNT; m++) g_mod[m] = g_saved_mod[m];
    mods_from_environment();
}

bool revo_settings_fullscreen(void) { return g_fullscreen_saved && !getenv("REVO_WINDOW"); }

void revo_settings_toggle_fullscreen(void) { set_fullscreen(!revo_window_fullscreen()); }

extern "C" int revo_mod(int mod) { return mod >= 0 && mod < REVO_MOD_COUNT && g_mod[mod]; }

extern "C" int revo_set(int setting) { return setting >= 0 && setting < REVO_SET_COUNT ? g_set[setting].load() : 0; }

extern "C" float revo_rayman_speed(void) { return 1.0f + (float)g_set[REVO_SET_SPEED].load(); }

extern "C" int revo_extras_on(void) {
    return g_set[REVO_SET_SPEED] != 0 || g_set[REVO_SET_INF_HEALTH] != 0 || g_set[REVO_SET_NOCLIP] != 0;
}

extern "C" float revo_draw_distance(void) {
    const int v = g_set[REVO_SET_DRAW_DISTANCE].load();
    return v > 0 && v < 5 ? k_draw_distances[v] : 1.0f;
}

extern "C" unsigned long long revo_frame_count(void);
static int aspect_at(void) { return -1; }

extern "C" void revo_view_aspect(int *across, int *down) {
    bool wide = g_mod[REVO_MOD_WIDESCREEN], wide10 = g_mod[REVO_MOD_WIDESCREEN_1610];
    int wider = g_set[REVO_SET_WIDER];
    const int forced = aspect_at();
    if (forced >= 0) wide = forced == 1, wide10 = forced == 2, wider = 0;
    *across = wide || wide10 ? 16 : 4;
    *down = wide ? 9 : wide10 ? 10 : 3;
    if (wide && wider == 1) *across = 64, *down = 27;
    if (wide && wider == 2) screen_shape(across, down);
}

int revo_action_bit(int action) { return action >= 0 && action < REVO_ACT_COUNT ? k_actions[action].bit : 0; }
int revo_key_binding(int action, int slot) { return g_keys[action][slot & 1]; }
int revo_pad_binding(int action) { return g_pads[action]; }
extern "C" int revo_icon_binding(int action) { return action >= 0 && action < REVO_ACT_COUNT ? g_pads[action].load() : -1; }

extern "C" int revo_icon_style(void) {
    const int v = g_set[REVO_SET_BUTTON_ICONS].load();
    return v > 0 ? v - 1 : revo_keyboard_last() ? REVO_ICONS_KEYBOARD : revo_controller_style();
}

/* A key's name the way a keycap shows it: upper case, short, and the arrows as l, r, u, d, which the
   icons draw as arrows. */
static std::string keycap(int k) {
    switch (k) {
    case 0: return "";
    case SDLK_LEFT: return "l";
    case SDLK_RIGHT: return "r";
    case SDLK_UP: return "u";
    case SDLK_DOWN: return "d";
    case SDLK_RETURN: case SDLK_KP_ENTER: return "ENTER";
    case SDLK_ESCAPE: return "ESC";
    case SDLK_BACKSPACE: return "BKSP";
    case SDLK_SPACE: return "SPACE";
    case SDLK_LSHIFT: case SDLK_RSHIFT: return "SHIFT";
    case SDLK_LCTRL: case SDLK_RCTRL: return "CTRL";
    case SDLK_LALT: case SDLK_RALT: return "ALT";
    case SDLK_CAPSLOCK: return "CAPS";
    case SDLK_DELETE: return "DEL";
    case SDLK_INSERT: return "INS";
    case SDLK_PAGEUP: return "PGUP";
    case SDLK_PAGEDOWN: return "PGDN";
    default: break;
    }
    std::string n = key_name(k), out;
    if (n.rfind("Keypad ", 0) == 0) n = "KP" + n.substr(7);
    for (char c : n)
        if (c != ' ') out += (char)toupper((unsigned char)c);
    return out.size() > 6 ? out.substr(0, 6) : out;
}

/* The keycap of an action's key, for the button icons; REVO_ACT_COUNT gives the four moves, up,
   left, down and right, one character each, or nothing. */
extern "C" const char *revo_icon_key(int action) {
    static char names[REVO_ACT_COUNT + 1][16];
    if (action < 0 || action > REVO_ACT_COUNT) return "";
    std::string s;
    if (action == REVO_ACT_COUNT) {
        static const int moves[4] = {REVO_ACT_UP, REVO_ACT_LEFT, REVO_ACT_DOWN, REVO_ACT_RIGHT};
        for (int m : moves) {
            const std::string c = keycap(g_keys[m][0] ? g_keys[m][0].load() : g_keys[m][1].load());
            if (c.size() != 1) return "";
            s += c;
        }
    } else {
        s = keycap(g_keys[action][0] ? g_keys[action][0].load() : g_keys[action][1].load());
    }
    snprintf(names[action], sizeof names[action], "%s", s.c_str());
    return names[action];
}

bool revo_settings_event(const SDL_Event &e) {
    if (g_open && g_capture.action >= 0) {
        const int a = g_capture.action;
        if (g_capture.slot < 2 && e.type == SDL_KEYDOWN && !e.key.repeat) {
            const SDL_Keycode k = e.key.keysym.sym;
            if (k == SDLK_DELETE) bind_key(a, g_capture.slot, 0);
            else if (!reserved_key(k)) bind_key(a, g_capture.slot, (int)k);
            g_capture.action = -1;
            return true;
        }
        if (g_capture.slot == 2 && e.type == SDL_CONTROLLERBUTTONDOWN) {
            g_pad_held++;
            const int b = e.cbutton.button;
            if (g_pads[a] == b) bind_pad(a, -1);
            else if (b != SDL_CONTROLLER_BUTTON_GUIDE) bind_pad(a, b);
            g_capture.action = -1;
            g_pad_quiet = true;
            return true;
        }
        if (g_capture.slot == 2 && e.type == SDL_CONTROLLERAXISMOTION && e.caxis.value > 24000 &&
            (e.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT || e.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT)) {
            bind_pad(a, e.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT ? REVO_PAD_TRIGGER_LEFT : REVO_PAD_TRIGGER_RIGHT);
            g_capture.action = -1;
            g_pad_quiet = true;
            return true;
        }
    }
    switch (e.type) {
    case SDL_KEYDOWN: {
        const SDL_Keycode k = e.key.keysym.sym;
        if (k == SDLK_F1 || (k == SDLK_COMMA && (e.key.keysym.mod & KMOD_GUI))) {
            if (!e.key.repeat) open_menu(!g_open);
            return true;
        }
        if (k == SDLK_F2) {
            if (!e.key.repeat) toggle_fps_counter();
            return true;
        }
        if (k == SDLK_F3) {
            if (!e.key.repeat) set_value(O_NOCLIP, value(O_NOCLIP) ? 0 : 1);
            return true;
        }
        if (!g_open) return false;
        if (k == SDLK_ESCAPE && !popup_open()) open_menu(false);
        else if ((k == SDLK_q || k == SDLK_PAGEUP) && !e.key.repeat) switch_tab(-1);
        else if ((k == SDLK_e || k == SDLK_PAGEDOWN) && !e.key.repeat) switch_tab(1);
        else revo_ui_event(e);
        return true;
    }
    case SDL_KEYUP:
        if (g_open) revo_ui_event(e);
        return false;
    case SDL_TEXTINPUT:
    case SDL_MOUSEMOTION:
    case SDL_MOUSEWHEEL:
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
        if (g_open) revo_ui_event(e);
        return g_open;
    case SDL_CONTROLLERBUTTONDOWN: {
        g_pad_held++;
        SDL_GameController *c = SDL_GameControllerFromInstanceID(e.cbutton.which);
        const int b = e.cbutton.button;
        noclip_combo(c);
        const bool combo = c && ((b == SDL_CONTROLLER_BUTTON_LEFTSTICK && SDL_GameControllerGetButton(c, SDL_CONTROLLER_BUTTON_RIGHTSTICK)) ||
                                 (b == SDL_CONTROLLER_BUTTON_RIGHTSTICK && SDL_GameControllerGetButton(c, SDL_CONTROLLER_BUTTON_LEFTSTICK)));
        if (b == SDL_CONTROLLER_BUTTON_GUIDE || b == SDL_CONTROLLER_BUTTON_TOUCHPAD || combo) {
            open_menu(!g_open);
            return true;
        }
        if (!g_open) return false;
        if (b == SDL_CONTROLLER_BUTTON_LEFTSHOULDER) switch_tab(-1);
        else if (b == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) switch_tab(1);
        else if ((b == SDL_CONTROLLER_BUTTON_B && !popup_open()) || b == SDL_CONTROLLER_BUTTON_START) open_menu(false);
        return true;
    }
    case SDL_CONTROLLERBUTTONUP:
        g_pad_held = std::max(0, g_pad_held - 1);
        noclip_combo(SDL_GameControllerFromInstanceID(e.cbutton.which));
        return g_open;
    case SDL_CONTROLLERAXISMOTION:
        noclip_combo(SDL_GameControllerFromInstanceID(e.caxis.which));
        return g_open;
    default:
        return false;
    }
}

bool revo_settings_open(void) { return g_open; }

void revo_settings_draw(int width, int height) {
    const int rate = frame_rate();
    const int shown = shown_rate();
    const bool timer = g_set[REVO_SET_TIMER] != 0;
    const char *saving_words = "";
    const float saving = saving_opacity(&saving_words);
    const bool update = revo_update_badge_wanted() && !g_open;
    const bool extras = revo_extras_on() != 0;
    if ((!g_open && !g_fps_counter && !timer && saving <= 0.0f && !update && !extras) || width <= 0 || height <= 0) return;
    if (!revo_ui_begin()) return;
    ImGuiIO &io = ImGui::GetIO();
    if (g_pad_quiet && g_pad_held == 0) g_pad_quiet = false;
    if (g_pad_quiet) io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableGamepad;
    else io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    if (g_fps_counter) fps_counter(rate, shown);
    if (timer) speedrun_timer();
    if (extras) extras_badge(timer);
    if (saving > 0.0f) saving_badge(saving_words, saving, (timer ? 2 : 0) + (extras ? 1 : 0));
    if (update) revo_update_badge();
    if (g_open) menu(width, height);
    revo_ui_end();
}
