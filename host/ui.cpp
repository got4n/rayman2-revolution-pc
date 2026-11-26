#include "ui.h"

#include <algorithm>
#include <cstdio>

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include <vulkan/vulkan.h>
#include "imgui_impl_vulkan.h"

extern "C" {
#include "runtime.h"
typedef struct VkUi {
    void *instance, *gpu, *device, *queue, *render_pass;
    unsigned family, images;
    void *(*proc)(const char *name);
} VkUi;
bool vk_ui(VkUi *ui);
void vk_ui_hook(void (*record)(void *cmd));
}

namespace {

SDL_Window *g_window;
bool g_ready;
bool g_pending;
float g_font_scale = 1;
float g_scale = 1;

void load_font(ImGuiIO &io, float size) {
    static const char *const k_fonts[] = {
#ifdef _WIN32
        "C:\\Windows\\Fonts\\segoeui.ttf", "C:\\Windows\\Fonts\\arial.ttf",
#elif defined(__APPLE__)
        "/System/Library/Fonts/Supplemental/Arial.ttf", "/Library/Fonts/Arial.ttf",
#else
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", "/usr/share/fonts/TTF/DejaVuSans.ttf",
#endif
    };
    for (const char *path : k_fonts)
        if (FILE *f = fopen(path, "rb")) {
            fclose(f);
            if (io.Fonts->AddFontFromFileTTF(path, size)) return;
        }
    ImFontConfig c;
    c.SizePixels = size;
    io.Fonts->AddFontDefault(&c);
}

void style(float scale) {
    ImGui::StyleColorsDark();
    ImGuiStyle &s = ImGui::GetStyle();
    s.WindowRounding = 8;
    s.FrameRounding = 4;
    s.GrabRounding = 4;
    s.PopupRounding = 4;
    s.TabRounding = 4;
    s.WindowPadding = ImVec2(14, 12);
    s.FramePadding = ImVec2(8, 4);
    s.ItemSpacing = ImVec2(10, 7);
    s.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    ImVec4 *c = s.Colors;
    const ImVec4 accent(0.95f, 0.70f, 0.15f, 1.00f);
    c[ImGuiCol_WindowBg] = ImVec4(0.07f, 0.08f, 0.12f, 0.94f);
    c[ImGuiCol_PopupBg] = ImVec4(0.09f, 0.10f, 0.15f, 0.98f);
    c[ImGuiCol_TitleBg] = c[ImGuiCol_TitleBgActive] = ImVec4(0.12f, 0.14f, 0.24f, 1.00f);
    c[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.18f, 0.28f, 1.00f);
    c[ImGuiCol_FrameBgHovered] = ImVec4(0.22f, 0.25f, 0.40f, 1.00f);
    c[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.30f, 0.48f, 1.00f);
    c[ImGuiCol_Tab] = ImVec4(0.14f, 0.16f, 0.26f, 1.00f);
    c[ImGuiCol_TabHovered] = ImVec4(0.30f, 0.34f, 0.55f, 1.00f);
    c[ImGuiCol_TabSelected] = ImVec4(0.24f, 0.28f, 0.48f, 1.00f);
    c[ImGuiCol_TabSelectedOverline] = accent;
    c[ImGuiCol_Header] = ImVec4(0.24f, 0.28f, 0.48f, 1.00f);
    c[ImGuiCol_HeaderHovered] = ImVec4(0.30f, 0.34f, 0.58f, 1.00f);
    c[ImGuiCol_HeaderActive] = ImVec4(0.34f, 0.38f, 0.64f, 1.00f);
    c[ImGuiCol_Button] = ImVec4(0.20f, 0.23f, 0.38f, 1.00f);
    c[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.32f, 0.52f, 1.00f);
    c[ImGuiCol_ButtonActive] = ImVec4(0.34f, 0.38f, 0.62f, 1.00f);
    c[ImGuiCol_CheckMark] = accent;
    c[ImGuiCol_SliderGrab] = accent;
    c[ImGuiCol_NavCursor] = accent;
    c[ImGuiCol_Separator] = ImVec4(0.25f, 0.27f, 0.40f, 1.00f);
    s.ScaleAllSizes(scale);
}

PFN_vkVoidFunction vk_function(const char *name, void *proc) {
    return (PFN_vkVoidFunction)((void *(*)(const char *))proc)(name);
}

void record(void *cmd) {
    if (!g_pending) return;
    g_pending = false;
    if (ImDrawData *d = ImGui::GetDrawData()) ImGui_ImplVulkan_RenderDrawData(d, (VkCommandBuffer)cmd);
}

bool vulkan_backend() {
    VkUi ui;
    if (!vk_ui(&ui)) return false;
    if (!ImGui_ImplVulkan_LoadFunctions(VK_API_VERSION_1_1, vk_function, (void *)ui.proc)) {
        revo_log("[ui] Vulkan: ImGui's functions not found");
        return false;
    }
    ImGui_ImplVulkan_InitInfo i = {};
    i.ApiVersion = VK_API_VERSION_1_1;
    i.Instance = (VkInstance)ui.instance;
    i.PhysicalDevice = (VkPhysicalDevice)ui.gpu;
    i.Device = (VkDevice)ui.device;
    i.QueueFamily = ui.family;
    i.Queue = (VkQueue)ui.queue;
    i.RenderPass = (VkRenderPass)ui.render_pass;
    i.MinImageCount = 2;
    i.ImageCount = std::max(3u, ui.images);
    i.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    i.DescriptorPoolSize = 16;
    if (!ImGui_ImplVulkan_Init(&i)) {
        revo_log("[ui] Vulkan: ImGui_ImplVulkan_Init failed");
        return false;
    }
    vk_ui_hook(record);
    return true;
}

}

void revo_ui_init(SDL_Window *window) {
    g_window = window;
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad;
    SDL_DisplayMode mode;
    const int display = std::max(0, SDL_GetWindowDisplayIndex(window));
    if (SDL_GetDesktopDisplayMode(display, &mode) == 0 && mode.h > 0)
        g_font_scale = std::max(1.0f, std::min(3.0f, (float)mode.h / 1080.0f));
    load_font(io, 20.0f * g_font_scale);
    g_scale = g_font_scale;
    style(g_scale);
    ImGui_ImplSDL2_InitForVulkan(window);
    revo_log("[ui] Dear ImGui %s, scale %.2f", IMGUI_VERSION, (double)g_scale);
}

void revo_ui_event(const SDL_Event &e) {
    if (ImGui::GetCurrentContext()) ImGui_ImplSDL2_ProcessEvent(&e);
}

bool revo_ui_begin(void) {
    if (!ImGui::GetCurrentContext()) return false;
    if (!g_ready) g_ready = vulkan_backend();
    if (!g_ready) return false;
    int w = 0, h = 0;
    SDL_GetWindowSizeInPixels(g_window, &w, &h);
    const float scale = std::max(0.75f, std::min(g_font_scale, (float)h / 1080.0f));
    if (h > 0 && (scale > g_scale * 1.02f || scale < g_scale * 0.98f)) {
        g_scale = scale;
        style(scale);
        ImGui::GetIO().FontGlobalScale = scale / g_font_scale;
    }
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    return true;
}

void revo_ui_end(void) {
    ImGui::Render();
    g_pending = true;
}

float revo_ui_scale(void) { return g_scale; }
