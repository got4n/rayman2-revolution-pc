#include "render_vk.h"

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#include <SDL.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "render.h"
#include "render_vk.h"
#include "native_frag.h"
#include "native_vert.h"
#include "overlay_frag.h"
#include "overlay_vert.h"
#include "present_frag.h"
#include "present_vert.h"
#include "rich_frag.h"
#include "rich_vert.h"

#define VK_INSTANCE_FUNCTIONS(X)                                                                                   \
    X(vkEnumeratePhysicalDevices) X(vkGetPhysicalDeviceProperties)                            \
    X(vkGetPhysicalDeviceFeatures2) X(vkGetPhysicalDeviceMemoryProperties)                                         \
    X(vkGetPhysicalDeviceQueueFamilyProperties) X(vkEnumerateDeviceExtensionProperties) X(vkCreateDevice)          \
    X(vkGetDeviceProcAddr) X(vkGetPhysicalDeviceSurfaceSupportKHR) X(vkGetPhysicalDeviceSurfaceCapabilitiesKHR) X(vkGetPhysicalDeviceSurfacePresentModesKHR)    \
    X(vkGetPhysicalDeviceSurfaceFormatsKHR)
#define VK_DEVICE_FUNCTIONS(X)                                                                                     \
    X(vkGetDeviceQueue) X(vkCreateCommandPool) X(vkAllocateCommandBuffers) X(vkBeginCommandBuffer)                \
    X(vkEndCommandBuffer) X(vkResetCommandBuffer) X(vkQueueSubmit) X(vkDeviceWaitIdle) X(vkCreateFence)            \
    X(vkWaitForFences) X(vkResetFences) X(vkGetFenceStatus) X(vkCreateImage) X(vkDestroyImage) X(vkGetImageMemoryRequirements)         \
    X(vkAllocateMemory) X(vkFreeMemory) X(vkBindImageMemory) X(vkCreateImageView) X(vkDestroyImageView)            \
    X(vkCreateBuffer) X(vkDestroyBuffer) X(vkGetBufferMemoryRequirements) X(vkBindBufferMemory) X(vkMapMemory)     \
    X(vkCreateSampler) X(vkCreateDescriptorSetLayout) X(vkCreateDescriptorPool) X(vkAllocateDescriptorSets)        \
    X(vkFreeDescriptorSets) X(vkUpdateDescriptorSets) X(vkCreatePipelineLayout) X(vkCreateShaderModule)            \
    X(vkDestroyShaderModule) X(vkCreateGraphicsPipelines) X(vkCreateRenderPass) X(vkCreateFramebuffer)             \
    X(vkDestroyFramebuffer) X(vkCmdBeginRenderPass) X(vkCmdEndRenderPass) X(vkCmdBindPipeline)                     \
    X(vkCmdBindDescriptorSets) X(vkCmdBindVertexBuffers) X(vkCmdSetViewport) X(vkCmdSetScissor) X(vkCmdDraw)      \
    X(vkCmdPipelineBarrier) X(vkCmdCopyBufferToImage) X(vkCmdCopyImageToBuffer) X(vkCmdCopyImage)                 \
    X(vkCmdClearColorImage) X(vkCmdPushConstants) X(vkCreateSemaphore) X(vkCreateSwapchainKHR)                    \
    X(vkDestroySwapchainKHR) X(vkGetSwapchainImagesKHR) X(vkAcquireNextImageKHR) X(vkQueuePresentKHR)            \
    X(vkCmdClearDepthStencilImage)

#define DECLARE(f) static PFN_##f f;
DECLARE(vkGetInstanceProcAddr)
DECLARE(vkCreateInstance)
DECLARE(vkEnumerateInstanceExtensionProperties)
VK_INSTANCE_FUNCTIONS(DECLARE)
VK_DEVICE_FUNCTIONS(DECLARE)
#undef DECLARE

typedef struct Buffer {
    VkBuffer buffer;
    VkDeviceMemory memory;
    u8 *map;
    VkDeviceSize size;
} Buffer;

typedef struct Target {
    VkImage color, depth;
    VkDeviceMemory color_memory, depth_memory;
    VkImageView color_view, depth_view;
    bool owns_depth;
    VkFramebuffer framebuffer;
    VkDescriptorSet sampled;
    VkDescriptorSet shown;
    int width, height;
    int pw, ph;
    float sx, sy;
} Target;

typedef struct Texture {
    VkImage image;
    VkDeviceMemory memory;
    VkImageView view;
    VkDescriptorSet set;
    bool cut_out;
    VkDescriptorSet lightmap_set;
} Texture;

typedef struct DrawState {
    float view[4];
    float fog_color[4];
    int32_t textured, texture_alpha, flat_color, fog;
    int32_t alpha_mode;
    float alpha_ref;
    int32_t sampler_index, pad;
    float projection[4];
    float projection2[4];
} DrawState;
_Static_assert(offsetof(DrawState, projection) == 64, "DrawState's projection: state.glsl's offset");
_Static_assert(offsetof(DrawState, projection2) == 80, "DrawState's projection2: state.glsl's offset");

typedef struct Vertex {
    float x, y, z, s, t, q;
    u8 rgba[4], flat[4];
    float fog;
} Vertex;

typedef struct RVertex {
    Vertex gs;
    float ls, lt;
} RVertex;

typedef struct Slot {
    VkCommandBuffer cmd;
    VkFence fence;
    bool in_flight;
    Buffer vertices, states, staging;
    VkDescriptorSet state_set;
} Slot;

enum { SLOTS = 3 };

typedef struct Overlay {
    VkImage image;
    VkDeviceMemory memory;
    VkImageView view;
    VkDescriptorSet set;
    int w, h;
} Overlay;

enum { PRESENTS = 2, OVERLAYS = 4, MAX_IMAGES = 8 };

typedef struct PresentSlot {
    VkCommandBuffer cmd;
    VkFence fence;
    bool in_flight;
    VkSemaphore acquired;
    Buffer staging;
    Overlay overlays[OVERLAYS];
} PresentSlot;

typedef struct Queued {
    u8 *rgba;
    int w, h, x, y, zoom;
} Queued;

typedef struct Grave {
    Target *target;
    Texture *texture;
} Grave;

enum { BLEND_NONE, BLEND_ALPHA, BLEND_ADD, BLEND_MULTIPLY };
enum { SAMPLERS = 40, PIPELINES = 128 };
typedef struct PipelineEntry {
    uint32_t key;
    VkPipeline pipeline;
} PipelineEntry;

static struct {
    bool ready;
    void *library;
    VkInstance instance;
    VkPhysicalDevice gpu;
    VkPhysicalDeviceMemoryProperties memory;
    VkDevice device;
    VkQueue queue;
    VkDeviceSize ubo_align;
    bool anisotropy;
    float max_anisotropy;
    VkCommandPool pool;
    Slot slots[SLOTS];
    int slot;
    VkCommandBuffer read_cmd;
    VkFence read_fence;
    VkDescriptorSetLayout samplers_layout, image_layout, texture_layout, state_layout;
    VkSampler samplers[SAMPLERS];
    VkDescriptorSet samplers_set;
    PipelineEntry pipelines[PIPELINES];
    int npipelines;
    VkPipeline bound;
    VkSampler nearest;
    VkSampler bilinear;
    VkDescriptorPool sets;
    VkPipelineLayout layout;
    VkShaderModule native_vs, native_fs, rich_vs, rich_fs;
    VkDescriptorSetLayout lightmap_layout;
    VkRenderPass pass;
    VkSampler sampler;
    Target frames[2];
    Texture no_texture;
    Buffer readback;
    size_t vertices_used, states_used, staging_used;
    Target *cur, *main;
    bool in_pass;
    Grave *graves;
    size_t ngraves, graves_cap;
    const char *library_path;
    uint32_t family;
    bool pipeline_made;
    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;
    bool swapchain_stale;
    bool surface_caps2;
    bool no_exclusive;
    bool readable;
    VkFormat format;
    VkExtent2D extent;
    uint32_t nimages;
    VkImage images[MAX_IMAGES];
    VkImageView views[MAX_IMAGES];
    VkFramebuffer framebuffers[MAX_IMAGES];
    VkSemaphore rendered[MAX_IMAGES];
    VkRenderPass window_pass;
    VkPipelineLayout present_layout, overlay_layout;
    VkPipeline present_pipeline, overlay_pipeline;
    PresentSlot present[PRESENTS];
    int present_next;
    int picture;
    VkRect2D show_rect;
    int present_mode, present_post;
    Queued queued[OVERLAYS];
    int nqueued;
    char shot[256];
} vk = {.picture = -1};

#define S (vk.slots[vk.slot])

static bool load_library(void) {
    const char *names[] = {getenv("REVO_VULKAN_LIBRARY"), "libvulkan.1.dylib", "libvulkan.dylib", "libMoltenVK.dylib",
                           "/opt/homebrew/lib/libMoltenVK.dylib", "/usr/local/lib/libMoltenVK.dylib",
                           "libvulkan.so.1", "libvulkan.so", "vulkan-1.dll"};
    if (vk.library) return true;
    for (size_t i = 0; i < sizeof names / sizeof *names && !vk.library; i++)
        if (names[i] && *names[i] && (vk.library = SDL_LoadObject(names[i]))) vk.library_path = names[i];
    if (!vk.library) return false;
    vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)SDL_LoadFunction(vk.library, "vkGetInstanceProcAddr");
    if (!vkGetInstanceProcAddr) return false;
    vkCreateInstance = (PFN_vkCreateInstance)vkGetInstanceProcAddr(NULL, "vkCreateInstance");
    vkEnumerateInstanceExtensionProperties =
        (PFN_vkEnumerateInstanceExtensionProperties)vkGetInstanceProcAddr(NULL, "vkEnumerateInstanceExtensionProperties");
    return vkCreateInstance && vkEnumerateInstanceExtensionProperties;
}

static bool has_extension(const VkExtensionProperties *e, uint32_t n, const char *name) {
    for (uint32_t i = 0; i < n; i++)
        if (!strcmp(e[i].extensionName, name)) return true;
    return false;
}

static bool open_instance(const char *const *wanted, unsigned nwanted) {
    uint32_t n = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &n, NULL);
    VkExtensionProperties *e = calloc(n ? n : 1, sizeof *e);
    vkEnumerateInstanceExtensionProperties(NULL, &n, e);
    const char *ext[16];
    uint32_t next = 0;
    bool portability = has_extension(e, n, "VK_KHR_portability_enumeration");
    if (portability) ext[next++] = "VK_KHR_portability_enumeration";
#ifdef _WIN32
    vk.surface_caps2 = has_extension(e, n, "VK_KHR_get_surface_capabilities2");
    if (vk.surface_caps2) ext[next++] = "VK_KHR_get_surface_capabilities2";
#endif
    bool all = true;
    for (unsigned i = 0; i < nwanted && next < 16; i++) {
        if (!has_extension(e, n, wanted[i])) all = false;
        ext[next++] = wanted[i];
    }
    free(e);
    if (!all) {
        revo_log("[gli] Vulkan: the window's surface extensions are missing");
        return false;
    }
    VkApplicationInfo app = {VK_STRUCTURE_TYPE_APPLICATION_INFO, .pApplicationName = "Rayman Re-Revolution",
                             .apiVersion = VK_API_VERSION_1_1};
    VkInstanceCreateInfo ci = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, .pApplicationInfo = &app,
                               .flags = portability ? VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR : 0,
                               .enabledExtensionCount = next, .ppEnabledExtensionNames = ext};
    if (vkCreateInstance(&ci, NULL, &vk.instance) != VK_SUCCESS) return false;
#define LOAD(f) f = (PFN_##f)vkGetInstanceProcAddr(vk.instance, #f);
    VK_INSTANCE_FUNCTIONS(LOAD)
#undef LOAD
    return vkCreateDevice && vkGetDeviceProcAddr && vkGetPhysicalDeviceFeatures2;
}

static bool open_device(void) {
    uint32_t n = 0;
    vkEnumeratePhysicalDevices(vk.instance, &n, NULL);
    VkPhysicalDevice *gpus = calloc(n ? n : 1, sizeof *gpus);
    vkEnumeratePhysicalDevices(vk.instance, &n, gpus);
    uint32_t family = 0;
    bool portability = false, exclusive = false;
    for (uint32_t i = 0; i < n && !vk.gpu; i++) {
        uint32_t ne = 0;
        vkEnumerateDeviceExtensionProperties(gpus[i], NULL, &ne, NULL);
        VkExtensionProperties *e = calloc(ne ? ne : 1, sizeof *e);
        vkEnumerateDeviceExtensionProperties(gpus[i], NULL, &ne, e);
        bool swapchain = has_extension(e, ne, "VK_KHR_swapchain");
        portability = has_extension(e, ne, "VK_KHR_portability_subset");
        exclusive = has_extension(e, ne, "VK_EXT_full_screen_exclusive");
        free(e);
        if (vk.surface && !swapchain) continue;
        VkPhysicalDeviceFeatures2 f2 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
        vkGetPhysicalDeviceFeatures2(gpus[i], &f2);
        if (!f2.features.shaderSampledImageArrayDynamicIndexing) continue;
        vk.anisotropy = f2.features.samplerAnisotropy;
        uint32_t nq = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(gpus[i], &nq, NULL);
        VkQueueFamilyProperties *q = calloc(nq ? nq : 1, sizeof *q);
        vkGetPhysicalDeviceQueueFamilyProperties(gpus[i], &nq, q);
        for (uint32_t k = 0; k < nq; k++) {
            VkBool32 shows = VK_TRUE;
            if (vk.surface) vkGetPhysicalDeviceSurfaceSupportKHR(gpus[i], k, vk.surface, &shows);
            if ((q[k].queueFlags & VK_QUEUE_GRAPHICS_BIT) && shows) {
                vk.gpu = gpus[i];
                family = k;
                break;
            }
        }
        free(q);
    }
    free(gpus);
    if (!vk.gpu) return false;
    VkPhysicalDeviceProperties p;
    vkGetPhysicalDeviceProperties(vk.gpu, &p);
    vkGetPhysicalDeviceMemoryProperties(vk.gpu, &vk.memory);
    vk.ubo_align = p.limits.minUniformBufferOffsetAlignment;
    float priority = 1;
    VkDeviceQueueCreateInfo qci = {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, .queueFamilyIndex = family, .queueCount = 1,
                                   .pQueuePriorities = &priority};
    VkPhysicalDeviceFeatures2 f2 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
                                    .features = {.samplerAnisotropy = vk.anisotropy,
                                                 .depthClamp = VK_TRUE, .shaderClipDistance = VK_TRUE,
                                                 .shaderSampledImageArrayDynamicIndexing = VK_TRUE}};
    vk.max_anisotropy = p.limits.maxSamplerAnisotropy;
    const char *ext[4];
    uint32_t next = 0;
    if (portability) ext[next++] = "VK_KHR_portability_subset";
    if (vk.surface) ext[next++] = "VK_KHR_swapchain";
    vk.no_exclusive = vk.surface && vk.surface_caps2 && exclusive;
    if (vk.no_exclusive) ext[next++] = "VK_EXT_full_screen_exclusive";
    vk.family = family;
    VkDeviceCreateInfo dci = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, .pNext = &f2, .queueCreateInfoCount = 1,
                              .pQueueCreateInfos = &qci, .enabledExtensionCount = next,
                              .ppEnabledExtensionNames = ext};
    if (vkCreateDevice(vk.gpu, &dci, NULL, &vk.device) != VK_SUCCESS) return false;
#define LOAD(f) f = (PFN_##f)vkGetDeviceProcAddr(vk.device, #f);
    VK_DEVICE_FUNCTIONS(LOAD)
#undef LOAD
    vkGetDeviceQueue(vk.device, family, 0, &vk.queue);
    VkCommandPoolCreateInfo cpci = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                                    .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, .queueFamilyIndex = family};
    vkCreateCommandPool(vk.device, &cpci, NULL, &vk.pool);
    VkCommandBuffer cmds[SLOTS + 1 + PRESENTS];
    VkCommandBufferAllocateInfo cbai = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, .commandPool = vk.pool,
                                        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                                        .commandBufferCount = SLOTS + 1 + PRESENTS};
    vkAllocateCommandBuffers(vk.device, &cbai, cmds);
    VkFenceCreateInfo fci = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    for (int k = 0; k < SLOTS; k++) {
        vk.slots[k].cmd = cmds[k];
        vkCreateFence(vk.device, &fci, NULL, &vk.slots[k].fence);
    }
    VkSemaphoreCreateInfo sci = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    for (int k = 0; k < PRESENTS && vk.surface; k++) {
        vk.present[k].cmd = cmds[SLOTS + 1 + k];
        vkCreateFence(vk.device, &fci, NULL, &vk.present[k].fence);
        vkCreateSemaphore(vk.device, &sci, NULL, &vk.present[k].acquired);
    }
    for (int k = 0; k < MAX_IMAGES && vk.surface; k++) vkCreateSemaphore(vk.device, &sci, NULL, &vk.rendered[k]);
    vk.read_cmd = cmds[SLOTS];
    vkCreateFence(vk.device, &fci, NULL, &vk.read_fence);
    revo_log("[gli] Vulkan: %s", p.deviceName);
    return true;
}

static uint32_t memory_type(uint32_t bits, VkMemoryPropertyFlags want) {
    for (uint32_t i = 0; i < vk.memory.memoryTypeCount; i++)
        if ((bits >> i & 1) && (vk.memory.memoryTypes[i].propertyFlags & want) == want) return i;
    for (uint32_t i = 0; i < vk.memory.memoryTypeCount; i++)
        if ((bits >> i & 1) && !(want & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) return i;
    return 0;
}

static bool make_image_levels(int w, int h, uint32_t levels, VkFormat format, VkImageUsageFlags usage, VkImage *image,
                              VkDeviceMemory *memory, VkImageView *view) {
    VkImageCreateInfo ii = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, .imageType = VK_IMAGE_TYPE_2D, .format = format,
                            .extent = {(uint32_t)w, (uint32_t)h, 1}, .mipLevels = levels, .arrayLayers = 1,
                            .samples = VK_SAMPLE_COUNT_1_BIT, .tiling = VK_IMAGE_TILING_OPTIMAL, .usage = usage};
    if (vkCreateImage(vk.device, &ii, NULL, image) != VK_SUCCESS) return false;
    VkMemoryRequirements mr;
    vkGetImageMemoryRequirements(vk.device, *image, &mr);
    VkMemoryAllocateInfo mai = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, .allocationSize = mr.size,
                                .memoryTypeIndex = memory_type(mr.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)};
    if (vkAllocateMemory(vk.device, &mai, NULL, memory) != VK_SUCCESS) {
        vkDestroyImage(vk.device, *image, NULL);
        return false;
    }
    vkBindImageMemory(vk.device, *image, *memory, 0);
    VkImageViewCreateInfo vci = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, .image = *image,
                                 .viewType = VK_IMAGE_VIEW_TYPE_2D, .format = format,
                                 .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, levels, 0, 1}};
    vkCreateImageView(vk.device, &vci, NULL, view);
    return true;
}

static bool make_image(int w, int h, VkFormat format, VkImageUsageFlags usage, VkImage *image, VkDeviceMemory *memory,
                       VkImageView *view) {
    return make_image_levels(w, h, 1, format, usage, image, memory, view);
}

static void free_image(VkImage image, VkDeviceMemory memory, VkImageView view) {
    if (view) vkDestroyImageView(vk.device, view, NULL);
    if (image) vkDestroyImage(vk.device, image, NULL);
    if (memory) vkFreeMemory(vk.device, memory, NULL);
}

static void free_buffer(Buffer *b) {
    if (b->buffer) vkDestroyBuffer(vk.device, b->buffer, NULL);
    if (b->memory) vkFreeMemory(vk.device, b->memory, NULL);
    memset(b, 0, sizeof *b);
}

static bool make_buffer(Buffer *b, VkDeviceSize size, VkBufferUsageFlags usage) {
    if (b->size >= size) return true;
    free_buffer(b);
    size = size < 65536 ? 65536 : size * 3 / 2;
    VkBufferCreateInfo bci = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, .size = size, .usage = usage};
    if (vkCreateBuffer(vk.device, &bci, NULL, &b->buffer) != VK_SUCCESS) return false;
    VkMemoryRequirements mr;
    vkGetBufferMemoryRequirements(vk.device, b->buffer, &mr);
    VkMemoryAllocateInfo mai = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, .allocationSize = mr.size,
        .memoryTypeIndex = memory_type(mr.memoryTypeBits,
                                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)};
    if (vkAllocateMemory(vk.device, &mai, NULL, &b->memory) != VK_SUCCESS) return false;
    vkBindBufferMemory(vk.device, b->buffer, b->memory, 0);
    vkMapMemory(vk.device, b->memory, 0, VK_WHOLE_SIZE, 0, (void **)&b->map);
    b->size = size;
    return true;
}

static VkDescriptorSet allocate_set(VkDescriptorSetLayout layout) {
    VkDescriptorSet set = VK_NULL_HANDLE;
    VkDescriptorSetAllocateInfo ai = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, .descriptorPool = vk.sets,
                                      .descriptorSetCount = 1, .pSetLayouts = &layout};
    vkAllocateDescriptorSets(vk.device, &ai, &set);
    return set;
}

static void write_image(VkDescriptorSet set, uint32_t binding, VkDescriptorType type, VkImageView view) {
    VkDescriptorImageInfo info = {.sampler = vk.sampler, .imageView = view, .imageLayout = VK_IMAGE_LAYOUT_GENERAL};
    VkWriteDescriptorSet w = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, .dstSet = set, .dstBinding = binding,
                              .descriptorCount = 1, .descriptorType = type, .pImageInfo = &info};
    vkUpdateDescriptorSets(vk.device, 1, &w, 0, NULL);
}

static void point_states(Slot *slot) {
    VkDescriptorBufferInfo info = {slot->states.buffer, 0, sizeof(DrawState)};
    VkWriteDescriptorSet w = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, .dstSet = slot->state_set, .dstBinding = 2,
                              .descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                              .pBufferInfo = &info};
    vkUpdateDescriptorSets(vk.device, 1, &w, 0, NULL);
}

static VkShaderModule shader(const uint32_t *code, size_t size) {
    VkShaderModuleCreateInfo ci = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, .codeSize = size, .pCode = code};
    VkShaderModule m = VK_NULL_HANDLE;
    vkCreateShaderModule(vk.device, &ci, NULL, &m);
    return m;
}

static int sampler_index(int filter_setting, bool linear, bool clamp_s, bool clamp_t) {
    int level = filter_setting <= 0 ? 0 : filter_setting <= 1 ? 1 : filter_setting <= 4 ? 2 : filter_setting <= 8 ? 3 : 4;
    return level * 8 + (linear ? 1 : 0) + (clamp_s ? 2 : 0) + (clamp_t ? 4 : 0);
}

static bool make_samplers(void) {
    static const float taps[5] = {0, 1, 4, 8, 16};
    for (int level = 0; level < 5; level++)
        for (int k = 0; k < 8; k++) {
            const VkFilter filter = (k & 1) || level ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
            VkSamplerCreateInfo sci = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO, .magFilter = filter, .minFilter = filter,
                                       .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
                                       .addressModeU = (k & 2) ? VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
                                                               : VK_SAMPLER_ADDRESS_MODE_REPEAT,
                                       .addressModeV = (k & 4) ? VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
                                                               : VK_SAMPLER_ADDRESS_MODE_REPEAT,
                                       .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                                       .maxLod = level ? VK_LOD_CLAMP_NONE : 0.0f};
            if (level && !(k & 1)) sci.magFilter = sci.minFilter = VK_FILTER_NEAREST, sci.maxLod = 0.0f;
            if (level && (k & 1) && vk.anisotropy && taps[level] > 1) {
                sci.anisotropyEnable = VK_TRUE;
                sci.maxAnisotropy = taps[level] < vk.max_anisotropy ? taps[level] : vk.max_anisotropy;
            }
            if (vkCreateSampler(vk.device, &sci, NULL, &vk.samplers[level * 8 + k]) != VK_SUCCESS) return false;
        }
    VkSamplerCreateInfo sci = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO, .magFilter = VK_FILTER_NEAREST,
                               .minFilter = VK_FILTER_NEAREST, .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                               .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                               .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE};
    if (vkCreateSampler(vk.device, &sci, NULL, &vk.nearest) != VK_SUCCESS) return false;
    sci.magFilter = sci.minFilter = VK_FILTER_LINEAR;
    return vkCreateSampler(vk.device, &sci, NULL, &vk.bilinear) == VK_SUCCESS;
}

static bool make_pipeline(void) {
    if (!make_samplers()) return false;
    VkDescriptorSetLayoutBinding samplers = {0, VK_DESCRIPTOR_TYPE_SAMPLER, SAMPLERS, VK_SHADER_STAGE_FRAGMENT_BIT,
                                             vk.samplers};
    VkDescriptorSetLayoutBinding image = {0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT};
    VkDescriptorSetLayoutBinding texture = {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT};
    VkDescriptorSetLayoutBinding state = {2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1,
                                          VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT};
    VkDescriptorSetLayoutCreateInfo lci = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, .bindingCount = 1,
                                           .pBindings = &samplers};
    vkCreateDescriptorSetLayout(vk.device, &lci, NULL, &vk.samplers_layout);
    lci.pBindings = &image;
    vkCreateDescriptorSetLayout(vk.device, &lci, NULL, &vk.image_layout);
    lci.pBindings = &texture;
    vkCreateDescriptorSetLayout(vk.device, &lci, NULL, &vk.texture_layout);
    lci.pBindings = &state;
    vkCreateDescriptorSetLayout(vk.device, &lci, NULL, &vk.state_layout);
    lci.pBindings = &texture;
    vkCreateDescriptorSetLayout(vk.device, &lci, NULL, &vk.lightmap_layout);
    VkDescriptorPoolSize sizes[] = {{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 8192},
                                    {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 8192},
                                    {VK_DESCRIPTOR_TYPE_SAMPLER, SAMPLERS},
                                    {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 4}};
    VkDescriptorPoolCreateInfo pci = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                                      .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT, .maxSets = 16384,
                                      .poolSizeCount = 4, .pPoolSizes = sizes};
    if (vkCreateDescriptorPool(vk.device, &pci, NULL, &vk.sets) != VK_SUCCESS) return false;
    vk.samplers_set = allocate_set(vk.samplers_layout);
    vk.sampler = vk.nearest;
    VkDescriptorSetLayout layouts[4] = {vk.samplers_layout, vk.image_layout, vk.state_layout, vk.lightmap_layout};
    VkPipelineLayoutCreateInfo plci = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, .setLayoutCount = 4,
                                       .pSetLayouts = layouts};
    vkCreatePipelineLayout(vk.device, &plci, NULL, &vk.layout);
    VkAttachmentDescription attachments[2] = {
        {.format = VK_FORMAT_R8G8B8A8_UNORM, .samples = VK_SAMPLE_COUNT_1_BIT, .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
         .storeOp = VK_ATTACHMENT_STORE_OP_STORE, .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
         .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE, .initialLayout = VK_IMAGE_LAYOUT_GENERAL,
         .finalLayout = VK_IMAGE_LAYOUT_GENERAL},
        {.format = VK_FORMAT_D32_SFLOAT, .samples = VK_SAMPLE_COUNT_1_BIT, .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
         .storeOp = VK_ATTACHMENT_STORE_OP_STORE, .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
         .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE, .initialLayout = VK_IMAGE_LAYOUT_GENERAL,
         .finalLayout = VK_IMAGE_LAYOUT_GENERAL}};
    VkAttachmentReference color_ref = {0, VK_IMAGE_LAYOUT_GENERAL}, depth_ref = {1, VK_IMAGE_LAYOUT_GENERAL};
    VkSubpassDescription subpass = {.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS, .colorAttachmentCount = 1,
                                    .pColorAttachments = &color_ref, .pDepthStencilAttachment = &depth_ref};
    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, .attachmentCount = 2,
                                   .pAttachments = attachments, .subpassCount = 1, .pSubpasses = &subpass};
    if (vkCreateRenderPass(vk.device, &rpci, NULL, &vk.pass) != VK_SUCCESS) return false;
    vk.native_vs = shader(native_vert, sizeof native_vert);
    vk.native_fs = shader(native_frag, sizeof native_frag);
    vk.rich_vs = shader(rich_vert, sizeof rich_vert);
    vk.rich_fs = shader(rich_frag, sizeof rich_frag);
    for (int k = 0; k < SLOTS; k++) {
        Slot *slot = &vk.slots[k];
        slot->state_set = allocate_set(vk.state_layout);
        if (!make_buffer(&slot->states, 4 << 20, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)) return false;
        point_states(slot);
    }
    return true;
}

static uint32_t pipeline_key(bool rich, int blend, int ztst, bool zwrite, uint32_t masked) {
    return (rich ? 1u : 0u) | (uint32_t)blend << 1 | (uint32_t)ztst << 3 | (zwrite ? 1u << 5 : 0u) | masked << 6;
}

static VkPipeline pipeline_for(uint32_t key) {
    for (int i = 0; i < vk.npipelines; i++)
        if (vk.pipelines[i].key == key) return vk.pipelines[i].pipeline;
    if (vk.npipelines == PIPELINES) return VK_NULL_HANDLE;
    const bool rich = key & 1;
    const int blend = (int)(key >> 1 & 3), ztst = (int)(key >> 3 & 3);
    const bool zwrite = key >> 5 & 1;
    const uint32_t masked = key >> 6 & 15;
    VkPipelineShaderStageCreateInfo stages[2] = {
        {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .stage = VK_SHADER_STAGE_VERTEX_BIT,
         .module = rich ? vk.rich_vs : vk.native_vs, .pName = "main"},
        {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
         .module = rich ? vk.rich_fs : vk.native_fs, .pName = "main"}};
    VkVertexInputBindingDescription binding = {0, rich ? sizeof(RVertex) : sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX};
    VkVertexInputAttributeDescription attributes[6] = {
        {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, x)},
        {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, s)},
        {2, 0, VK_FORMAT_R8G8B8A8_UINT, offsetof(Vertex, rgba)},
        {3, 0, VK_FORMAT_R8G8B8A8_UINT, offsetof(Vertex, flat)},
        {4, 0, VK_FORMAT_R32_SFLOAT, offsetof(Vertex, fog)},
        {5, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(RVertex, ls)}};
    VkPipelineVertexInputStateCreateInfo vi = {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
                                               .vertexBindingDescriptionCount = 1, .pVertexBindingDescriptions = &binding,
                                               .vertexAttributeDescriptionCount = rich ? 6 : 5,
                                               .pVertexAttributeDescriptions = attributes};
    VkPipelineInputAssemblyStateCreateInfo ia = {VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
                                                 .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST};
    VkPipelineViewportStateCreateInfo vps = {VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, .viewportCount = 1,
                                             .scissorCount = 1};
    VkPipelineRasterizationStateCreateInfo rs = {VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
                                                 .depthClampEnable = VK_TRUE,
                                                 .polygonMode = VK_POLYGON_MODE_FILL, .cullMode = VK_CULL_MODE_NONE,
                                                 .lineWidth = 1};
    VkPipelineMultisampleStateCreateInfo ms = {VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
                                               .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT};
    static const VkCompareOp compare[4] = {VK_COMPARE_OP_NEVER, VK_COMPARE_OP_ALWAYS, VK_COMPARE_OP_GREATER_OR_EQUAL,
                                           VK_COMPARE_OP_GREATER};
    VkPipelineDepthStencilStateCreateInfo dss = {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
                                                 .depthTestEnable = VK_TRUE, .depthWriteEnable = zwrite,
                                                 .depthCompareOp = compare[ztst]};
    VkPipelineColorBlendAttachmentState ba = {.colorWriteMask = (~masked) & 15};
    ba.alphaBlendOp = ba.colorBlendOp = VK_BLEND_OP_ADD;
    ba.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    ba.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    if (blend != BLEND_NONE) ba.blendEnable = VK_TRUE;
    if (blend == BLEND_ALPHA) ba.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA, ba.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    if (blend == BLEND_ADD) ba.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA, ba.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
    if (blend == BLEND_MULTIPLY) {
        ba.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        ba.dstColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        ba.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        ba.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    }
    VkPipelineColorBlendStateCreateInfo cb = {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                                              .attachmentCount = 1, .pAttachments = &ba};
    VkDynamicState dynamic[2] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo ds = {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
                                           .dynamicStateCount = 2, .pDynamicStates = dynamic};
    VkGraphicsPipelineCreateInfo gpci = {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO, .stageCount = 2,
                                         .pStages = stages, .pVertexInputState = &vi, .pInputAssemblyState = &ia,
                                         .pViewportState = &vps, .pRasterizationState = &rs, .pMultisampleState = &ms,
                                         .pDepthStencilState = &dss, .pColorBlendState = &cb, .pDynamicState = &ds,
                                         .layout = vk.layout, .renderPass = vk.pass};
    VkPipeline p;
    if (vkCreateGraphicsPipelines(vk.device, VK_NULL_HANDLE, 1, &gpci, NULL, &p) != VK_SUCCESS) return VK_NULL_HANDLE;
    vk.pipelines[vk.npipelines++] = (PipelineEntry){key, p};
    return p;
}

static void barrier(VkCommandBuffer cmd, VkPipelineStageFlags from, VkAccessFlags written, VkPipelineStageFlags to,
                    VkAccessFlags then) {
    VkMemoryBarrier mb = {VK_STRUCTURE_TYPE_MEMORY_BARRIER, .srcAccessMask = written, .dstAccessMask = then};
    vkCmdPipelineBarrier(cmd, from, to, 0, 1, &mb, 0, NULL, 0, NULL);
}

static void to_general(VkCommandBuffer cmd, VkImage image) {
    VkImageMemoryBarrier b = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, .srcAccessMask = 0,
                              .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT, .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                              .newLayout = VK_IMAGE_LAYOUT_GENERAL, .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                              .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .image = image,
                              .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0, 1}};
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1,
                         &b);
}

static void clear_image(VkCommandBuffer cmd, VkImage image) {
    static const VkClearColorValue zero = {{0}};
    VkImageSubresourceRange all = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    vkCmdClearColorImage(cmd, image, VK_IMAGE_LAYOUT_GENERAL, &zero, 1, &all);
}

static bool make_depth(VkCommandBuffer cmd, int w, int h, VkImage *image, VkDeviceMemory *memory, VkImageView *view) {
    VkImageCreateInfo ii = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, .imageType = VK_IMAGE_TYPE_2D,
                            .format = VK_FORMAT_D32_SFLOAT, .extent = {(uint32_t)w, (uint32_t)h, 1}, .mipLevels = 1,
                            .arrayLayers = 1, .samples = VK_SAMPLE_COUNT_1_BIT, .tiling = VK_IMAGE_TILING_OPTIMAL,
                            .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
                                     VK_IMAGE_USAGE_TRANSFER_DST_BIT};
    if (vkCreateImage(vk.device, &ii, NULL, image) != VK_SUCCESS) return false;
    VkMemoryRequirements mr;
    vkGetImageMemoryRequirements(vk.device, *image, &mr);
    VkMemoryAllocateInfo mai = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, .allocationSize = mr.size,
                                .memoryTypeIndex = memory_type(mr.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)};
    if (vkAllocateMemory(vk.device, &mai, NULL, memory) != VK_SUCCESS) {
        vkDestroyImage(vk.device, *image, NULL);
        return false;
    }
    vkBindImageMemory(vk.device, *image, *memory, 0);
    VkImageViewCreateInfo vci = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, .image = *image,
                                 .viewType = VK_IMAGE_VIEW_TYPE_2D, .format = VK_FORMAT_D32_SFLOAT,
                                 .subresourceRange = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1}};
    vkCreateImageView(vk.device, &vci, NULL, view);
    VkImageMemoryBarrier b = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                              .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED, .newLayout = VK_IMAGE_LAYOUT_GENERAL,
                              .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                              .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .image = *image,
                              .subresourceRange = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1}};
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1,
                         &b);
    VkClearDepthStencilValue far = {0.0f, 0};
    VkImageSubresourceRange all = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1};
    vkCmdClearDepthStencilImage(cmd, *image, VK_IMAGE_LAYOUT_GENERAL, &far, 1, &all);
    return true;
}

static void everything_barrier(VkCommandBuffer cmd) {
    VkMemoryBarrier mb = {VK_STRUCTURE_TYPE_MEMORY_BARRIER,
                          .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                                           VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_SHADER_WRITE_BIT |
                                           VK_ACCESS_TRANSFER_WRITE_BIT,
                          .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                                           VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                           VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT |
                                           VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_TRANSFER_READ_BIT |
                                           VK_ACCESS_TRANSFER_WRITE_BIT};
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 1, &mb, 0, NULL,
                         0, NULL);
}

static void wait_slot(Slot *slot) {
    if (!slot->in_flight) return;
    vkWaitForFences(vk.device, 1, &slot->fence, VK_TRUE, UINT64_MAX);
    vkResetFences(vk.device, 1, &slot->fence);
    slot->in_flight = false;
}

static bool slot_done(Slot *slot) {
    if (slot->in_flight && vkGetFenceStatus(vk.device, slot->fence) == VK_SUCCESS) wait_slot(slot);
    return !slot->in_flight;
}

static void wait_frame(void) {
    wait_slot(&S);
    for (int k = 0; k < SLOTS; k++)
        if (k != vk.slot && !slot_done(&vk.slots[k])) return;
    for (size_t i = 0; i < vk.ngraves; i++) {
        Target *t = vk.graves[i].target;
        Texture *x = vk.graves[i].texture;
        if (t) {
            if (t->framebuffer) vkDestroyFramebuffer(vk.device, t->framebuffer, NULL);
            if (t->shown) vkFreeDescriptorSets(vk.device, vk.sets, 1, &t->shown);
            if (t->sampled) vkFreeDescriptorSets(vk.device, vk.sets, 1, &t->sampled);
            free_image(t->color, t->color_memory, t->color_view);
            if (t->owns_depth) free_image(t->depth, t->depth_memory, t->depth_view);
            free(t);
        }
        if (x) {
            if (x->set) vkFreeDescriptorSets(vk.device, vk.sets, 1, &x->set);
            if (x->lightmap_set) vkFreeDescriptorSets(vk.device, vk.sets, 1, &x->lightmap_set);
            free_image(x->image, x->memory, x->view);
            free(x);
        }
    }
    vk.ngraves = 0;
}

static void wait_all(void) {
    for (int k = 0; k < SLOTS; k++) wait_slot(&vk.slots[k]);
    wait_frame();
}

static void begin(VkCommandBuffer cmd) {
    vkResetCommandBuffer(cmd, 0);
    VkCommandBufferBeginInfo bi = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                   .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};
    vkBeginCommandBuffer(cmd, &bi);
}

static void submit(VkCommandBuffer cmd, VkFence fence) {
    vkEndCommandBuffer(cmd);
    VkSubmitInfo si = {VK_STRUCTURE_TYPE_SUBMIT_INFO, .commandBufferCount = 1, .pCommandBuffers = &cmd};
    vkQueueSubmit(vk.queue, 1, &si, fence);
}

static bool make_target(VkCommandBuffer cmd, Target *t, int w, int h, int pw, int ph, const Target *depth) {
    memset(t, 0, sizeof *t);
    t->width = w;
    t->height = h;
    t->pw = pw;
    t->ph = ph;
    t->sx = (float)pw / (float)w;
    t->sy = (float)ph / (float)h;
    if (!make_image(pw, ph, VK_FORMAT_R8G8B8A8_UNORM,
                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
                        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                    &t->color, &t->color_memory, &t->color_view))
        return false;
    if (depth) {
        t->depth = depth->depth;
        t->depth_view = depth->depth_view;
    } else {
        if (!make_depth(cmd, pw, ph, &t->depth, &t->depth_memory, &t->depth_view)) return false;
        t->owns_depth = true;
    }
    to_general(cmd, t->color);
    clear_image(cmd, t->color);
    VkImageView views[2] = {t->color_view, t->depth_view};
    VkFramebufferCreateInfo fci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, .renderPass = vk.pass,
                                   .attachmentCount = 2, .pAttachments = views, .width = (uint32_t)pw,
                                   .height = (uint32_t)ph, .layers = 1};
    vkCreateFramebuffer(vk.device, &fci, NULL, &t->framebuffer);
    t->sampled = allocate_set(vk.image_layout);
    write_image(t->sampled, 0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, t->color_view);
    t->shown = allocate_set(vk.texture_layout);
    write_image(t->shown, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, t->color_view);
    return true;
}

static void bury(Target *t, Texture *x) {
    if (vk.ngraves == vk.graves_cap) {
        vk.graves_cap = vk.graves_cap * 2 + 16;
        vk.graves = realloc(vk.graves, sizeof *vk.graves * vk.graves_cap);
    }
    vk.graves[vk.ngraves++] = (Grave){t, x};
}

static void free_frames(void) {
    for (int b = 1; b >= 0; b--) {
        Target *t = malloc(sizeof *t);
        *t = vk.frames[b];
        bury(t, NULL);
        memset(&vk.frames[b], 0, sizeof vk.frames[b]);
    }
}

static bool make_frames(int pw, int ph) {
    begin(vk.read_cmd);
    bool ok = make_target(vk.read_cmd, &vk.frames[0], GS_WIDTH, GS_HEIGHT, pw, ph, NULL) &&
              make_target(vk.read_cmd, &vk.frames[1], GS_WIDTH, GS_HEIGHT, pw, ph, &vk.frames[0]);
    barrier(vk.read_cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_TRANSFER_READ_BIT);
    submit(vk.read_cmd, vk.read_fence);
    vkWaitForFences(vk.device, 1, &vk.read_fence, VK_TRUE, UINT64_MAX);
    vkResetFences(vk.device, 1, &vk.read_fence);
    return ok;
}

static Vertex vertex(const GsVertex *v, const GsVertex *last) {
    Vertex o = {v->x, v->y, v->z, v->s, v->t, v->q, {v->r, v->g, v->b, v->a}, {last->r, last->g, last->b, last->a},
                (float)v->fog};
    return o;
}

static size_t triangles(const GsDraw *d, const GsVertex *v, Vertex *out) {
    size_t n = 0;
    switch (d->prim) {
    case GS_TRIANGLES:
        for (u32 i = 0; i + 2 < d->count; i += 3)
            for (int k = 0; k < 3; k++) out[n++] = vertex(&v[i + (u32)k], &v[i + 2]);
        break;
    case GS_TRISTRIP:
        for (u32 i = 0; i + 2 < d->count; i++)
            for (int k = 0; k < 3; k++) out[n++] = vertex(&v[i + (u32)k], &v[i + 2]);
        break;
    case GS_TRIFAN:
        for (u32 i = 1; i + 1 < d->count; i++) {
            out[n++] = vertex(&v[0], &v[i + 1]);
            out[n++] = vertex(&v[i], &v[i + 1]);
            out[n++] = vertex(&v[i + 1], &v[i + 1]);
        }
        break;
    case GS_SPRITE:
        for (u32 i = 0; i + 1 < d->count; i += 2) {
            GsVertex c[4] = {v[i + 1], v[i + 1], v[i + 1], v[i + 1]};
            c[0].x = v[i].x; c[0].y = v[i].y;
            c[1].x = v[i + 1].x; c[1].y = v[i].y;
            c[2].x = v[i].x; c[2].y = v[i + 1].y;
            c[0].s = v[i].s; c[0].t = v[i].t;
            c[1].t = v[i].t;
            c[2].s = v[i].s;
            static const int order[6] = {0, 1, 2, 1, 3, 2};
            for (int k = 0; k < 6; k++) out[n++] = vertex(&c[order[k]], &v[i + 1]);
        }
        break;
    default:
        break;
    }
    return n;
}

static Target *texture_target(VkCommandBuffer cmd, GsTexture *t) {
    if (t->vk_target) return t->vk_target;
    Target *g = calloc(1, sizeof *g);
    if (!make_target(cmd, g, t->width, t->height, t->width, t->height, NULL)) {
        free(g);
        return NULL;
    }
    t->vk_target = g;
    return g;
}

#define MIP_LEVELS_MAX 16

static size_t level_bytes(int w, int h) { return ((size_t)w * (size_t)h * 4 + 15) & ~(size_t)15; }

static uint32_t mip_chain(int w, int h, size_t *bytes) {
    uint32_t n = 0;
    size_t b = 0;
    for (;;) {
        b += level_bytes(w, h);
        n++;
        if ((w == 1 && h == 1) || n == MIP_LEVELS_MAX) break;
        w = w > 1 ? w / 2 : 1;
        h = h > 1 ? h / 2 : 1;
    }
    if (bytes) *bytes = b;
    return n;
}

static void downsample(const u32 *src, int sw, int sh, u32 *dst, int dw, int dh) {
    for (int y = 0; y < dh; y++)
        for (int x = 0; x < dw; x++) {
            const int x0 = x * 2 < sw ? x * 2 : sw - 1, x1 = x * 2 + 1 < sw ? x * 2 + 1 : sw - 1;
            const int y0 = y * 2 < sh ? y * 2 : sh - 1, y1 = y * 2 + 1 < sh ? y * 2 + 1 : sh - 1;
            const u32 p[4] = {src[y0 * sw + x0], src[y0 * sw + x1], src[y1 * sw + x0], src[y1 * sw + x1]};
            unsigned sum[3] = {0, 0, 0}, plain[3] = {0, 0, 0}, alpha = 0;
            for (int k = 0; k < 4; k++) {
                const unsigned a = p[k] >> 24;
                for (int c = 0; c < 3; c++) {
                    const unsigned v = p[k] >> (8 * c) & 0xFF;
                    sum[c] += v * a;
                    plain[c] += v;
                }
                alpha += a;
            }
            u32 out = ((alpha + 2) / 4) << 24;
            for (int c = 0; c < 3; c++) {
                const unsigned v = alpha ? (sum[c] + alpha / 2) / alpha : (plain[c] + 2) / 4;
                out |= (u32)(v > 255 ? 255 : v) << (8 * c);
            }
            dst[y * dw + x] = out;
        }
}

static void upload(VkCommandBuffer cmd, GsTexture *t) {
    if (t->vk || !t->texels || t->width <= 0 || t->height <= 0) return;
    size_t bytes;
    const uint32_t levels = mip_chain(t->width, t->height, &bytes);
    Texture *x = calloc(1, sizeof *x);
    if (!make_image_levels(t->width, t->height, levels, VK_FORMAT_R8G8B8A8_UNORM,
                           VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, &x->image, &x->memory,
                           &x->view)) {
        free(x);
        return;
    }
    u8 *chain = malloc(bytes);
    if (!chain) {
        free_image(x->image, x->memory, x->view);
        free(x);
        return;
    }
    VkBufferImageCopy copies[MIP_LEVELS_MAX];
    const size_t count = (size_t)t->width * (size_t)t->height;
    u32 *first = (u32 *)chain;
    for (size_t i = 0; i < count; i++) {
        const u32 c = t->texels[i], a = c >> 24;
        if (a < 64) x->cut_out = true;
        first[i] = (c & 0xFFFFFFu) | (a >= 128 ? 255u : a * 2u) << 24;
    }
    size_t at = 0;
    int w = t->width, h = t->height;
    for (uint32_t level = 0; level < levels; level++) {
        copies[level] = (VkBufferImageCopy){.bufferOffset = vk.staging_used + at,
                                            .imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, level, 0, 1},
                                            .imageExtent = {(uint32_t)w, (uint32_t)h, 1}};
        if (level + 1 < levels) {
            const int nw = w > 1 ? w / 2 : 1, nh = h > 1 ? h / 2 : 1;
            downsample((const u32 *)(chain + at), w, h, (u32 *)(chain + at + level_bytes(w, h)), nw, nh);
            at += level_bytes(w, h);
            w = nw;
            h = nh;
        }
    }
    memcpy(S.staging.map + vk.staging_used, chain, bytes);
    free(chain);
    to_general(cmd, x->image);
    vkCmdCopyBufferToImage(cmd, S.staging.buffer, x->image, VK_IMAGE_LAYOUT_GENERAL, levels, copies);
    vk.staging_used += bytes;
    x->set = allocate_set(vk.image_layout);
    write_image(x->set, 0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, x->view);
    t->vk = x;
}

static bool prologue(VkCommandBuffer cmd, const GsFrame *f) {
    size_t vertices = 0, states = 0, staging = 0;
    VkDeviceSize stride = (sizeof(DrawState) + vk.ubo_align - 1) / vk.ubo_align * vk.ubo_align;
    for (u32 i = 0; i < f->ndraws; i++) {
        const GsDraw *d = &f->draws[i];
        vertices += (size_t)d->count * 3 + 8;
        states += 2 * stride;
        const GsTexture *t = d->state.tme ? render_texture(d->state.texture) : NULL;
        size_t chain;
        if (t && !t->vk && t->texels && mip_chain(t->width, t->height, &chain)) staging += chain;
        const GsTexture *lm = d->state.lightmap ? render_texture(d->state.lightmap) : NULL;
        if (lm && !lm->vk && lm->texels && mip_chain(lm->width, lm->height, &chain)) staging += chain;
    }
    if (!make_buffer(&S.vertices, vertices * sizeof(RVertex) + 64, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)) return false;
    if (S.states.size < states) {
        if (!make_buffer(&S.states, states, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)) return false;
        point_states(&S);
    }
    if (staging && !make_buffer(&S.staging, staging, VK_BUFFER_USAGE_TRANSFER_SRC_BIT)) return false;
    vk.vertices_used = vk.states_used = vk.staging_used = 0;
    for (u32 i = 0; i < f->ndraws; i++) {
        const GsState *s = &f->draws[i].state;
        GsTexture *t = s->tme ? render_texture(s->texture) : NULL;
        if (t && t->target) texture_target(cmd, t);
        else if (t) upload(cmd, t);
        GsTexture *lm = s->lightmap ? render_texture(s->lightmap) : NULL;
        if (lm && !lm->target) upload(cmd, lm);
        if (s->target && s->target != GS_TARGET_SHOWN) {
            GsTexture *into = render_texture(s->target);
            if (into && into->target) texture_target(cmd, into);
        }
    }
    // The targets made above were cleared by a copy: the passes that draw into them wait for it.
    barrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_SHADER_WRITE_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT |
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_TRANSFER_READ_BIT |
                VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);
    return true;
}

static VkDescriptorSet lightmap_set(Texture *x) {
    if (!x->lightmap_set) {
        x->lightmap_set = allocate_set(vk.lightmap_layout);
        VkDescriptorImageInfo lightmap = {.sampler = vk.bilinear, .imageView = x->view,
                                          .imageLayout = VK_IMAGE_LAYOUT_GENERAL};
        VkWriteDescriptorSet w = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, .dstSet = x->lightmap_set, .dstBinding = 0,
                                  .descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                  .pImageInfo = &lightmap};
        vkUpdateDescriptorSets(vk.device, 1, &w, 0, NULL);
    }
    return x->lightmap_set;
}

static void end_pass(void) {
    if (!vk.in_pass) return;
    vkCmdEndRenderPass(S.cmd);
    vk.in_pass = false;
    everything_barrier(S.cmd);
}

static void use_target(Target *t) {
    if (vk.in_pass && vk.cur == t) return;
    end_pass();
    vk.cur = t;
    uint32_t w = (uint32_t)t->pw, h = (uint32_t)t->ph;
    VkRenderPassBeginInfo rbi = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, .renderPass = vk.pass,
                                 .framebuffer = t->framebuffer, .renderArea = {{0, 0}, {w, h}}};
    vkCmdBeginRenderPass(S.cmd, &rbi, VK_SUBPASS_CONTENTS_INLINE);
    vk.in_pass = true;
    vk.bound = VK_NULL_HANDLE;
    VkViewport vp = {0, 0, (float)w, (float)h, 0, 1};
    vkCmdSetViewport(S.cmd, 0, 1, &vp);
    vkCmdBindDescriptorSets(S.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk.layout, 0, 1, &vk.samplers_set, 0, NULL);
}

static Target *shown_buffer(void) {
    return &vk.frames[vk.main == &vk.frames[0]];
}

static void copy_shown(void) {
    end_pass();
    const Target *from = shown_buffer();
    VkImageCopy copy = {.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
                        .dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
                        .extent = {(uint32_t)vk.frames[0].pw, (uint32_t)vk.frames[0].ph, 1}};
    vkCmdCopyImage(S.cmd, from->color, VK_IMAGE_LAYOUT_GENERAL, vk.main->color, VK_IMAGE_LAYOUT_GENERAL, 1, &copy);
    everything_barrier(S.cmd);
}

static VkDeviceSize lightmap_vertices(const GsFrame *f, const GsDraw *d, size_t *count) {
    const VkDeviceSize at = (vk.vertices_used + 15) & ~(VkDeviceSize)15;
    RVertex *out = (RVertex *)(S.vertices.map + at);
    const GsVertex *v = f->vertices + d->first;
    const GsVertexExt *x = f->ext + d->first;
    size_t n = 0;
    for (u32 i = 0; i + 2 < d->count; i += 3)
        for (int k = 0; k < 3; k++) {
            const GsVertex *g = &v[i + (u32)k];
            const GsVertexExt *e = &x[i + (u32)k];
            out[n].gs = vertex(g, &v[i + 2]);
            out[n].ls = e->lu * g->q;
            out[n].lt = e->lv * g->q;
            n++;
        }
    const VkDeviceSize end = at + n * sizeof(RVertex);
    vk.vertices_used = (size_t)((end + sizeof(Vertex) - 1) / sizeof(Vertex) * sizeof(Vertex));
    *count = n;
    return at;
}

static int blend_of(const GsState *s) {
    if (!s->blend) return BLEND_NONE;
    if (s->c == GS_AS && s->a == GS_CS && s->b == GS_CD && s->d == GS_CD) return BLEND_ALPHA;
    if (s->c == GS_AS && s->a == GS_CS && s->b == GS_ZERO && s->d == GS_CD) return BLEND_ADD;
    if (s->c == GS_AS && s->a == GS_CD && s->b == GS_ZERO && s->d == GS_ZERO) return BLEND_MULTIPLY;
    if (s->a == GS_ZERO && s->b == GS_ZERO && s->d == GS_CS) return BLEND_NONE;
    static bool told;
    if (!told) {
        told = true;
        revo_log("[gli] a blend drawn as opaque: (%d - %d) * %d + %d", s->a, s->b, s->c, s->d);
    }
    return BLEND_NONE;
}

static uint32_t masked_of(uint32_t fbmsk) {
    uint32_t m = 0;
    for (int k = 0; k < 4; k++)
        if ((fbmsk >> (8 * k) & 0xFF) == 0xFF) m |= 1u << k;
    return m;
}

typedef struct Pass {
    int alpha_mode;
    bool zwrite;
    uint32_t masked;
} Pass;

static int alpha_passes(const GsState *s, bool can_fail, Pass out[2]) {
    const uint32_t masked = masked_of(s->fbmsk);
    out[0] = (Pass){0, s->zwrite, masked};
    if (!s->alpha_test || s->atst == GS_ALWAYS) return 1;
    int pass_mode = 1, fail_mode = 2;
    if (s->atst == GS_LESS || s->atst == GS_LEQUAL) pass_mode = 2, fail_mode = 1;
    Pass fail = {fail_mode, false, 15};
    if (s->afail == GS_FB_ONLY) fail.masked = masked;
    if (s->afail == GS_ZB_ONLY) fail.zwrite = s->zwrite;
    if (s->afail == GS_RGB_ONLY) fail.masked = masked | 8;
    if (s->atst == GS_NEVER) {
        out[0] = fail;
        out[0].alpha_mode = 0;
        return 1;
    }
    if (!can_fail) return 1;
    if (fail.zwrite == s->zwrite && fail.masked == masked) return 1;
    out[0].alpha_mode = pass_mode;
    if (fail.masked == 15 && !fail.zwrite) return 1;
    out[1] = fail;
    return 2;
}

static bool draw(const GsFrame *f, const GsDraw *d) {
    const GsVertex *v = f->vertices;
    if (d->prim == GS_COPY) {
        copy_shown();
        return false;
    }
    const GsState *s = &d->state;
    Target *t;
    if (s->target == GS_TARGET_SHOWN) {
        t = shown_buffer();
    } else if (s->target) {
        GsTexture *into = render_texture(s->target);
        if (!into || !into->target || !into->vk_target) return false;
        t = into->vk_target;
    } else {
        t = vk.main;
    }
    const GsTexture *lightmap = s->lightmap ? render_texture(s->lightmap) : NULL;
    const bool rich = lightmap && lightmap->vk && d->prim == GS_TRIANGLES && f->ext && d->first + d->count <= f->next;
    size_t n;
    VkDeviceSize at;
    if (rich) {
        at = lightmap_vertices(f, d, &n);
    } else {
        at = vk.vertices_used;
        n = triangles(d, v + d->first, (Vertex *)(S.vertices.map + at));
        vk.vertices_used += n * sizeof(Vertex);
    }
    if (!n) return false;
    use_target(t);
    const int pw = t->pw, ph = t->ph;
    VkRect2D sc = {{0, 0}, {(uint32_t)pw, (uint32_t)ph}};
    if (s->scissor) {
        int x0 = (int)lrintf((float)s->scissor_box[0] * t->sx), y0 = (int)lrintf((float)s->scissor_box[1] * t->sy);
        int x1 = (int)lrintf((float)(s->scissor_box[2] + 1) * t->sx);
        int y1 = (int)lrintf((float)(s->scissor_box[3] + 1) * t->sy);
        x1 = x1 > pw ? pw : x1;
        y1 = y1 > ph ? ph : y1;
        if (x0 >= x1 || y0 >= y1) return false;
        sc = (VkRect2D){{x0, y0}, {(uint32_t)(x1 - x0), (uint32_t)(y1 - y0)}};
    }
    DrawState base;
    memset(&base, 0, sizeof base);
    base.view[0] = (float)t->width;
    base.view[1] = (float)t->height;
    base.view[2] = 0.5f / t->sx - 1.0f / 32.0f;
    base.view[3] = 0.5f / t->sy - 1.0f / 32.0f;
    if (d->camera) {
        base.projection[0] = d->projection.window[0];
        base.projection[1] = d->projection.window[1];
        base.projection[2] = d->projection.depth_scale;
        base.projection[3] = d->projection.depth_offset;
        base.projection2[0] = 1.0f;
        base.projection2[1] = d->projection.near_plane;
    }
    GsTexture *tex = s->tme ? render_texture(s->texture) : NULL;
    VkDescriptorSet texture = vk.no_texture.set;
    bool cut_out = false;
    if (tex) {
        if (tex->target && tex->vk_target) texture = ((Target *)tex->vk_target)->sampled, cut_out = true;
        else if (tex->vk) texture = ((Texture *)tex->vk)->set, cut_out = ((Texture *)tex->vk)->cut_out;
        else tex = NULL;
    }
    if (tex) {
        base.textured = s->tfx == GS_DECAL ? 2 : 1;
        base.texture_alpha = s->tcc;
        base.sampler_index = sampler_index(render_texture_filter(), s->mmag || s->mmin == 1 || s->mmin >= 4,
                                           s->wrap_s != GS_REPEAT, s->wrap_t != GS_REPEAT);
    }
    base.flat_color = d->prim == GS_SPRITE || s->flat;
    base.fog = s->fog;
    base.fog_color[0] = (float)(s->fog_color & 0xFF) / 255.0f;
    base.fog_color[1] = (float)(s->fog_color >> 8 & 0xFF) / 255.0f;
    base.fog_color[2] = (float)(s->fog_color >> 16 & 0xFF) / 255.0f;
    base.alpha_ref = (float)s->aref / 128.0f;
    bool can_fail = cut_out || !tex;
    for (u32 i = 0; i < d->count && !can_fail; i++) can_fail = v[d->first + i].a < 0x80;
    Pass passes[2];
    const int npasses = alpha_passes(s, can_fail, passes);
    const int blend = blend_of(s);
    vkCmdSetScissor(S.cmd, 0, 1, &sc);
    vkCmdBindVertexBuffers(S.cmd, 0, 1, &S.vertices.buffer, &at);
    vkCmdBindDescriptorSets(S.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk.layout, 1, 1, &texture, 0, NULL);
    if (rich) {
        VkDescriptorSet set3 = lightmap_set((Texture *)lightmap->vk);
        vkCmdBindDescriptorSets(S.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk.layout, 3, 1, &set3, 0, NULL);
    }
    for (int k = 0; k < npasses; k++) {
        const Pass *p = &passes[k];
        if (p->masked == 15 && !p->zwrite) continue;
        VkPipeline pipeline = pipeline_for(pipeline_key(rich, blend, s->ztst, p->zwrite, p->masked));
        if (!pipeline) return false;
        if (pipeline != vk.bound) {
            vkCmdBindPipeline(S.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
            vk.bound = pipeline;
        }
        DrawState *u = (DrawState *)(S.states.map + vk.states_used);
        *u = base;
        u->alpha_mode = p->alpha_mode;
        uint32_t offset = (uint32_t)vk.states_used;
        vkCmdBindDescriptorSets(S.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk.layout, 2, 1, &S.state_set, 1, &offset);
        vkCmdDraw(S.cmd, (uint32_t)n, 1, 0, 0);
        vk.states_used += (sizeof(DrawState) + vk.ubo_align - 1) / vk.ubo_align * vk.ubo_align;
    }
    return true;
}

static bool open_gpu(const char *const *extensions, unsigned n, VkSurfaceMaker make, void *ctx) {
    if (!load_library()) {
        revo_log("[gli] Vulkan: no library (REVO_VULKAN_LIBRARY)");
        return false;
    }
    if (!open_instance(extensions, n)) {
        revo_log("[gli] Vulkan: no instance");
        return false;
    }
    if (make && !make(ctx, (void *)vk.instance, (void *)&vk.surface)) {
        revo_log("[gli] Vulkan: no surface for the window");
        return false;
    }
    if (!open_device()) {
        revo_log("[gli] Vulkan: no GPU%s", make ? " that shows on the window" : "");
        return false;
    }
    if (!make_pipeline()) {
        revo_log("[gli] Vulkan: no pipeline");
        return false;
    }
    vk.pipeline_made = true;
    return true;
}

bool vk_open(int pw, int ph) {
    if (!vk.pipeline_made && !open_gpu(NULL, 0, NULL, NULL)) return false;
    begin(vk.read_cmd);
    static const u32 black = 0;
    GsTexture none = {.width = 1, .height = 1, .texels = (u32 *)&black};
    make_buffer(&S.staging, 64, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vk.staging_used = 0;
    upload(vk.read_cmd, &none);
    if (!none.vk) return false;
    vk.no_texture = *(Texture *)none.vk;
    free(none.vk);
    submit(vk.read_cmd, vk.read_fence);
    vkWaitForFences(vk.device, 1, &vk.read_fence, VK_TRUE, UINT64_MAX);
    vkResetFences(vk.device, 1, &vk.read_fence);
    if (!make_frames(pw, ph)) {
        revo_log("[gli] Vulkan: no frame buffer of %dx%d", pw, ph);
        return false;
    }
    vk.main = &vk.frames[0];
    vk.ready = true;
    return true;
}

bool vk_set_size(int pw, int ph) {
    if (!vk.ready) return false;
    wait_all();
    vkDeviceWaitIdle(vk.device);
    Target old[2] = {vk.frames[0], vk.frames[1]};
    memset(vk.frames, 0, sizeof vk.frames);
    if (!make_frames(pw, ph)) {
        free_frames();
        memcpy(vk.frames, old, sizeof old);
        return false;
    }
    for (int b = 1; b >= 0; b--) {
        Target *t = malloc(sizeof *t);
        *t = old[b];
        bury(t, NULL);
    }
    wait_all();
    vk.main = &vk.frames[0];
    return true;
}

int vk_draw_frame(const GsFrame *f) {
    if (!vk.ready) return 0;
    vk.slot = (vk.slot + 1) % SLOTS;
    wait_frame();
    begin(S.cmd);
    int drawn = 0;
    if (prologue(S.cmd, f)) {
        vk.main = &vk.frames[f->buffer & 1];
        vk.cur = NULL;
        for (u32 i = 0; i < f->ndraws; i++) drawn += draw(f, &f->draws[i]);
        end_pass();
    }
    submit(S.cmd, S.fence);
    S.in_flight = true;
    return drawn;
}

void vk_release_texture(GsTexture *t) {
    if (t->vk) bury(NULL, t->vk);
    if (t->vk_target) bury(t->vk_target, NULL);
    t->vk = t->vk_target = NULL;
}

const char *vk_library(void) { return load_library() ? vk.library_path : NULL; }

static VkPipeline window_pipeline(const uint32_t *vs_code, size_t vs_size, const uint32_t *fs_code, size_t fs_size,
                                  VkPipelineLayout layout, VkPrimitiveTopology topology, bool blend) {
    VkShaderModule vs = shader(vs_code, vs_size), fs = shader(fs_code, fs_size);
    VkPipelineShaderStageCreateInfo stages[2] = {
        {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .stage = VK_SHADER_STAGE_VERTEX_BIT, .module = vs,
         .pName = "main"},
        {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = fs,
         .pName = "main"}};
    VkPipelineVertexInputStateCreateInfo vi = {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    VkPipelineInputAssemblyStateCreateInfo ia = {VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
                                                 .topology = topology};
    VkPipelineViewportStateCreateInfo vps = {VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, .viewportCount = 1,
                                             .scissorCount = 1};
    VkPipelineRasterizationStateCreateInfo rs = {VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
                                                 .polygonMode = VK_POLYGON_MODE_FILL, .cullMode = VK_CULL_MODE_NONE,
                                                 .lineWidth = 1};
    VkPipelineMultisampleStateCreateInfo ms = {VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
                                               .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT};
    VkPipelineColorBlendAttachmentState att = {
        .blendEnable = blend, .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO, .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .alphaBlendOp = VK_BLEND_OP_ADD,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                          VK_COLOR_COMPONENT_A_BIT};
    VkPipelineColorBlendStateCreateInfo cb = {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                                              .attachmentCount = 1, .pAttachments = &att};
    VkDynamicState dynamic[2] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo ds = {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
                                           .dynamicStateCount = 2, .pDynamicStates = dynamic};
    VkGraphicsPipelineCreateInfo gpci = {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO, .stageCount = 2,
                                         .pStages = stages, .pVertexInputState = &vi, .pInputAssemblyState = &ia,
                                         .pViewportState = &vps, .pRasterizationState = &rs, .pMultisampleState = &ms,
                                         .pColorBlendState = &cb, .pDynamicState = &ds, .layout = layout,
                                         .renderPass = vk.window_pass};
    VkPipeline p = VK_NULL_HANDLE;
    vkCreateGraphicsPipelines(vk.device, VK_NULL_HANDLE, 1, &gpci, NULL, &p);
    vkDestroyShaderModule(vk.device, vs, NULL);
    vkDestroyShaderModule(vk.device, fs, NULL);
    return p;
}

static bool make_window_pipelines(void) {
    VkAttachmentDescription a = {.format = vk.format, .samples = VK_SAMPLE_COUNT_1_BIT,
                                 .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                                 .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                 .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                 .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                                 .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR};
    VkAttachmentReference ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkSubpassDescription sub = {.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS, .colorAttachmentCount = 1,
                                .pColorAttachments = &ref};
    VkSubpassDependency dep = {VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                               VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT};
    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, .attachmentCount = 1, .pAttachments = &a,
                                   .subpassCount = 1, .pSubpasses = &sub, .dependencyCount = 1,
                                   .pDependencies = &dep};
    if (vkCreateRenderPass(vk.device, &rpci, NULL, &vk.window_pass) != VK_SUCCESS) return false;
    VkPushConstantRange push = {VK_SHADER_STAGE_FRAGMENT_BIT, 0, 4 * sizeof(int32_t)};
    VkPipelineLayoutCreateInfo plci = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, .setLayoutCount = 1,
                                       .pSetLayouts = &vk.texture_layout, .pushConstantRangeCount = 1,
                                       .pPushConstantRanges = &push};
    vkCreatePipelineLayout(vk.device, &plci, NULL, &vk.present_layout);
    plci.pushConstantRangeCount = 0;
    vkCreatePipelineLayout(vk.device, &plci, NULL, &vk.overlay_layout);
    vk.present_pipeline = window_pipeline(present_vert, sizeof present_vert, present_frag, sizeof present_frag,
                                          vk.present_layout, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, false);
    vk.overlay_pipeline = window_pipeline(overlay_vert, sizeof overlay_vert, overlay_frag, sizeof overlay_frag,
                                          vk.overlay_layout, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, true);
    return vk.present_pipeline && vk.overlay_pipeline;
}

static VkFormat window_format(void) {
    uint32_t n = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(vk.gpu, vk.surface, &n, NULL);
    VkSurfaceFormatKHR *f = calloc(n ? n : 1, sizeof *f);
    vkGetPhysicalDeviceSurfaceFormatsKHR(vk.gpu, vk.surface, &n, f);
    VkFormat chosen = n && f[0].format != VK_FORMAT_UNDEFINED ? f[0].format : VK_FORMAT_B8G8R8A8_UNORM;
    for (uint32_t i = 0; i < n; i++)
        if (f[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR &&
            (f[i].format == VK_FORMAT_B8G8R8A8_UNORM || f[i].format == VK_FORMAT_R8G8B8A8_UNORM)) {
            chosen = f[i].format;
            break;
        }
    free(f);
    return chosen;
}

bool vk_open_window(const char *const *extensions, unsigned n, VkSurfaceMaker make, void *ctx) {
    if (!open_gpu(extensions, n, make, ctx)) {
        vk.surface = VK_NULL_HANDLE;
        vk.gpu = VK_NULL_HANDLE;
        vk.device = VK_NULL_HANDLE;
        vk.pipeline_made = false;
        return false;
    }
    vk.format = window_format();
    if (!make_window_pipelines()) {
        revo_log("[gli] Vulkan: no pipeline for the window");
        return false;
    }
    revo_log("[gli] Vulkan shows the frames on the window (format %d)", (int)vk.format);
    return true;
}

static void free_swapchain_images(void) {
    for (uint32_t i = 0; i < vk.nimages; i++) {
        if (vk.framebuffers[i]) vkDestroyFramebuffer(vk.device, vk.framebuffers[i], NULL);
        if (vk.views[i]) vkDestroyImageView(vk.device, vk.views[i], NULL);
        vk.framebuffers[i] = VK_NULL_HANDLE;
        vk.views[i] = VK_NULL_HANDLE;
    }
    vk.nimages = 0;
}

static VkPresentModeKHR present_mode(void) {
    const char *e = getenv("REVO_VSYNC");
    if (e && !strcmp(e, "fifo")) return VK_PRESENT_MODE_FIFO_KHR;
    VkPresentModeKHR modes[16];
    uint32_t n = 16;
    if (vkGetPhysicalDeviceSurfacePresentModesKHR(vk.gpu, vk.surface, &n, modes) < 0) return VK_PRESENT_MODE_FIFO_KHR;
    for (uint32_t i = 0; i < n; i++)
        if (modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) return VK_PRESENT_MODE_MAILBOX_KHR;
    return VK_PRESENT_MODE_FIFO_KHR;
}

static bool make_swapchain(int width, int height) {
    vkDeviceWaitIdle(vk.device);
    VkSurfaceCapabilitiesKHR caps;
    if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk.gpu, vk.surface, &caps) != VK_SUCCESS) return false;
    VkExtent2D extent = caps.currentExtent;
    if (extent.width == UINT32_MAX) {
        extent.width = (uint32_t)width;
        extent.height = (uint32_t)height;
    }
    if (!extent.width || !extent.height) return false;
    uint32_t count = caps.minImageCount < 3 ? 3 : caps.minImageCount;
    if (caps.maxImageCount && count > caps.maxImageCount) count = caps.maxImageCount;
    if (count > MAX_IMAGES) count = MAX_IMAGES;
    VkCompositeAlphaFlagBitsKHR alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    if (!(caps.supportedCompositeAlpha & alpha))
        for (uint32_t b = 1; b; b <<= 1)
            if (caps.supportedCompositeAlpha & b) {
                alpha = (VkCompositeAlphaFlagBitsKHR)b;
                break;
            }
    VkSwapchainCreateInfoKHR ci = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR, .surface = vk.surface,
                                   .minImageCount = count, .imageFormat = vk.format,
                                   .imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR, .imageExtent = extent,
                                   .imageArrayLayers = 1,
                                   .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                                 (caps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT),
                                   .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE, .preTransform = caps.currentTransform,
                                   .compositeAlpha = alpha, .presentMode = present_mode(), .clipped = VK_TRUE,
                                   .oldSwapchain = vk.swapchain};
    struct {
        VkStructureType sType;
        const void *pNext;
        int32_t fullScreenExclusive;
    } no_exclusive = {(VkStructureType)1000255000 , NULL,
                      2 };
    if (vk.no_exclusive) ci.pNext = &no_exclusive;
    VkSwapchainKHR made;
    if (vkCreateSwapchainKHR(vk.device, &ci, NULL, &made) != VK_SUCCESS) return false;
    free_swapchain_images();
    if (vk.swapchain) vkDestroySwapchainKHR(vk.device, vk.swapchain, NULL);
    vk.swapchain = made;
    vk.extent = extent;
    vk.swapchain_stale = false;
    vk.readable = (caps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) != 0;
    vk.nimages = MAX_IMAGES;
    vkGetSwapchainImagesKHR(vk.device, vk.swapchain, &vk.nimages, vk.images);
    for (uint32_t i = 0; i < vk.nimages; i++) {
        VkImageViewCreateInfo vci = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, .image = vk.images[i],
                                     .viewType = VK_IMAGE_VIEW_TYPE_2D, .format = vk.format,
                                     .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};
        vkCreateImageView(vk.device, &vci, NULL, &vk.views[i]);
        VkFramebufferCreateInfo fci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, .renderPass = vk.window_pass,
                                       .attachmentCount = 1, .pAttachments = &vk.views[i], .width = extent.width,
                                       .height = extent.height, .layers = 1};
        vkCreateFramebuffer(vk.device, &fci, NULL, &vk.framebuffers[i]);
    }
    revo_log("[gli] window swapchain %ux%u, %u images, %s%s", extent.width, extent.height, vk.nimages,
             ci.presentMode == VK_PRESENT_MODE_MAILBOX_KHR ? "mailbox" : "fifo", vk.no_exclusive ? ", never exclusive" : "");
    return true;
}

void vk_show(int b, int x, int y, int w, int h, int mode, int post) {
    vk.picture = b;
    vk.show_rect = (VkRect2D){{x, y}, {(uint32_t)(w > 0 ? w : 0), (uint32_t)(h > 0 ? h : 0)}};
    vk.present_mode = mode;
    vk.present_post = post;
}

void vk_overlay(const u8 *rgba, int w, int h, int x, int y, int zoom) {
    if (vk.nqueued >= OVERLAYS || w <= 0 || h <= 0) return;
    Queued *q = &vk.queued[vk.nqueued++];
    *q = (Queued){malloc((size_t)w * (size_t)h * 4), w, h, x, y, zoom};
    memcpy(q->rgba, rgba, (size_t)w * (size_t)h * 4);
}

void vk_screenshot(const char *name) { snprintf(vk.shot, sizeof vk.shot, "%s", name); }

static void (*g_ui_record)(void *cmd);

static void *ui_proc(const char *name) { return (void *)vkGetInstanceProcAddr(vk.instance, name); }

bool vk_ui(VkUi *ui) {
    if (!vk.ready || !vk.surface || !vk.window_pass) return false;
    *ui = (VkUi){(void *)vk.instance, (void *)vk.gpu,   (void *)vk.device, (void *)vk.queue, (void *)vk.window_pass,
                 vk.family,           vk.nimages ? vk.nimages : 3, ui_proc};
    return true;
}

void vk_ui_hook(void (*record)(void *cmd)) { g_ui_record = record; }

static void drop_queued(void) {
    for (int k = 0; k < vk.nqueued; k++) free(vk.queued[k].rgba);
    vk.nqueued = 0;
}

static int upload_overlays(PresentSlot *p, VkCommandBuffer cmd) {
    size_t bytes = 0;
    for (int k = 0; k < vk.nqueued; k++) bytes += ((size_t)vk.queued[k].w * (size_t)vk.queued[k].h * 4 + 15) & ~(size_t)15;
    if (!bytes || !make_buffer(&p->staging, bytes, VK_BUFFER_USAGE_TRANSFER_SRC_BIT)) return 0;
    size_t at = 0;
    int n = 0;
    for (int k = 0; k < vk.nqueued; k++) {
        const Queued *q = &vk.queued[k];
        Overlay *o = &p->overlays[k];
        if (o->w != q->w || o->h != q->h) {
            free_image(o->image, o->memory, o->view);
            o->image = VK_NULL_HANDLE;
            o->w = o->h = 0;
            if (!make_image(q->w, q->h, VK_FORMAT_R8G8B8A8_UNORM,
                            VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, &o->image, &o->memory, &o->view))
                break;
            o->w = q->w;
            o->h = q->h;
            if (!o->set) o->set = allocate_set(vk.texture_layout);
            write_image(o->set, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, o->view);
        }
        const size_t size = (size_t)q->w * (size_t)q->h * 4;
        memcpy(p->staging.map + at, q->rgba, size);
        to_general(cmd, o->image);
        VkBufferImageCopy copy = {.bufferOffset = at, .imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
                                  .imageExtent = {(uint32_t)q->w, (uint32_t)q->h, 1}};
        vkCmdCopyBufferToImage(cmd, p->staging.buffer, o->image, VK_IMAGE_LAYOUT_GENERAL, 1, &copy);
        at += (size + 15) & ~(size_t)15;
        n++;
    }
    barrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            VK_ACCESS_SHADER_READ_BIT);
    return n;
}

static void image_barrier(VkCommandBuffer cmd, VkImage image, VkImageLayout from, VkImageLayout to,
                          VkPipelineStageFlags src, VkAccessFlags written, VkPipelineStageFlags dst, VkAccessFlags then) {
    VkImageMemoryBarrier b = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, .srcAccessMask = written, .dstAccessMask = then,
                              .oldLayout = from, .newLayout = to, .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                              .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .image = image,
                              .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};
    vkCmdPipelineBarrier(cmd, src, dst, 0, 0, NULL, 0, NULL, 1, &b);
}

static void write_shot(PresentSlot *p) {
    vkWaitForFences(vk.device, 1, &p->fence, VK_TRUE, UINT64_MAX);
    const uint32_t w = vk.extent.width, h = vk.extent.height;
    const bool bgr = vk.format == VK_FORMAT_B8G8R8A8_UNORM || vk.format == VK_FORMAT_B8G8R8A8_SRGB;
    FILE *o = fopen(vk.shot, "wb");
    if (!o) return;
    const size_t n = strlen(vk.shot);
    const bool bmp = n > 4 && !strcmp(vk.shot + n - 4, ".bmp");
    const uint32_t stride = bmp ? (w * 3 + 3) & ~3u : w * 3;
    if (bmp) {
        const uint32_t data = stride * h, file = 54 + data;
        u8 head[54] = {'B', 'M'};
        memcpy(head + 2, &file, 4);
        head[10] = 54;
        head[14] = 40;
        memcpy(head + 18, &w, 4);
        memcpy(head + 22, &h, 4);
        head[26] = 1;
        head[28] = 24;
        memcpy(head + 34, &data, 4);
        fwrite(head, 1, sizeof head, o);
    } else {
        fprintf(o, "P6\n%u %u\n255\n", w, h);
    }
    u8 *row = calloc(stride, 1);
    for (uint32_t k = 0; k < h; k++) {
        const uint32_t y = bmp ? h - 1 - k : k;
        const u8 *px = vk.readback.map + (size_t)y * w * 4;
        for (uint32_t x = 0; x < w; x++) {
            const u8 r = px[4 * x + (bgr ? 2 : 0)], g = px[4 * x + 1], b = px[4 * x + (bgr ? 0 : 2)];
            row[3 * x] = bmp ? b : r;
            row[3 * x + 1] = g;
            row[3 * x + 2] = bmp ? r : b;
        }
        fwrite(row, 1, stride, o);
    }
    free(row);
    fclose(o);
    revo_log("[platform] wrote %s", vk.shot);
}

void vk_present(int width, int height) {
    if (!vk.ready || !vk.surface) return;
    PresentSlot *p = &vk.present[vk.present_next];
    if (p->in_flight) {
        vkWaitForFences(vk.device, 1, &p->fence, VK_TRUE, UINT64_MAX);
        vkResetFences(vk.device, 1, &p->fence);
        p->in_flight = false;
    }
    if ((!vk.swapchain || vk.swapchain_stale || vk.extent.width != (uint32_t)width ||
         vk.extent.height != (uint32_t)height) &&
        !make_swapchain(width, height)) {
        drop_queued();
        return;
    }
    uint32_t index = 0;
    VkResult r = vkAcquireNextImageKHR(vk.device, vk.swapchain, UINT64_MAX, p->acquired, VK_NULL_HANDLE, &index);
    if (r == VK_ERROR_OUT_OF_DATE_KHR || (r != VK_SUCCESS && r != VK_SUBOPTIMAL_KHR)) {
        vk.swapchain_stale = true;
        drop_queued();
        return;
    }
    if (r == VK_SUBOPTIMAL_KHR) vk.swapchain_stale = true;
    vk.present_next = (vk.present_next + 1) % PRESENTS;
    const bool shot = vk.shot[0] && vk.readable &&
                      make_buffer(&vk.readback, (VkDeviceSize)vk.extent.width * vk.extent.height * 4,
                                  VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    VkCommandBuffer cmd = p->cmd;
    begin(cmd);
    barrier(cmd, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            VK_ACCESS_SHADER_READ_BIT);
    const int overlays = shot ? 0 : upload_overlays(p, cmd);
    VkClearValue black = {.color = {{0.0f, 0.0f, 0.0f, 1.0f}}};
    VkRenderPassBeginInfo rbi = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, .renderPass = vk.window_pass,
                                 .framebuffer = vk.framebuffers[index], .renderArea = {{0, 0}, vk.extent},
                                 .clearValueCount = 1, .pClearValues = &black};
    vkCmdBeginRenderPass(cmd, &rbi, VK_SUBPASS_CONTENTS_INLINE);
    VkRect2D all = {{0, 0}, vk.extent};
    vkCmdSetScissor(cmd, 0, 1, &all);
    if (vk.picture >= 0 && vk.show_rect.extent.width && vk.show_rect.extent.height) {
        const Target *t = &vk.frames[vk.picture & 1];
        VkViewport vp = {(float)vk.show_rect.offset.x, (float)vk.show_rect.offset.y, (float)vk.show_rect.extent.width,
                         (float)vk.show_rect.extent.height, 0, 1};
        struct { int32_t mode; float w, h; int32_t post; } pc = {vk.present_mode, vp.width, vp.height, vk.present_post};
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk.present_pipeline);
        vkCmdSetViewport(cmd, 0, 1, &vp);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk.present_layout, 0, 1, &t->shown, 0, NULL);
        vkCmdPushConstants(cmd, vk.present_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof pc, &pc);
        vkCmdDraw(cmd, 3, 1, 0, 0);
    }
    if (overlays) vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk.overlay_pipeline);
    for (int k = 0; k < overlays; k++) {
        const Queued *q = &vk.queued[k];
        VkViewport vp = {(float)q->x, (float)q->y, (float)(q->w * q->zoom), (float)(q->h * q->zoom), 0, 1};
        vkCmdSetViewport(cmd, 0, 1, &vp);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk.overlay_layout, 0, 1, &p->overlays[k].set, 0,
                                NULL);
        vkCmdDraw(cmd, 4, 1, 0, 0);
    }
    if (g_ui_record && !shot) g_ui_record((void *)cmd);
    vkCmdEndRenderPass(cmd);
    if (shot) {
        image_barrier(cmd, vk.images[index], VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                      VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_READ_BIT);
        VkBufferImageCopy copy = {.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
                                  .imageExtent = {vk.extent.width, vk.extent.height, 1}};
        vkCmdCopyImageToBuffer(cmd, vk.images[index], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, vk.readback.buffer, 1, &copy);
        image_barrier(cmd, vk.images[index], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                      VK_PIPELINE_STAGE_TRANSFER_BIT, 0, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0);
        barrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_HOST_BIT,
                VK_ACCESS_HOST_READ_BIT);
    }
    vkEndCommandBuffer(cmd);
    VkPipelineStageFlags wait = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo si = {VK_STRUCTURE_TYPE_SUBMIT_INFO, .waitSemaphoreCount = 1, .pWaitSemaphores = &p->acquired,
                       .pWaitDstStageMask = &wait, .commandBufferCount = 1, .pCommandBuffers = &cmd,
                       .signalSemaphoreCount = 1, .pSignalSemaphores = &vk.rendered[index]};
    vkQueueSubmit(vk.queue, 1, &si, p->fence);
    p->in_flight = true;
    VkPresentInfoKHR pi = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, .waitSemaphoreCount = 1,
                           .pWaitSemaphores = &vk.rendered[index], .swapchainCount = 1, .pSwapchains = &vk.swapchain,
                           .pImageIndices = &index};
    r = vkQueuePresentKHR(vk.queue, &pi);
    if (r == VK_ERROR_OUT_OF_DATE_KHR || r == VK_SUBOPTIMAL_KHR) vk.swapchain_stale = true;
    if (shot) write_shot(p);
    vk.shot[0] = 0;
    drop_queued();
}
