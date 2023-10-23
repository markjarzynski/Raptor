#pragma once

#if (_MSC_VER)
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#define VK_ENABLE_BETA_EXTENSIONS
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <EASTL/allocator.h>

#include "Service.h"
#include "Window.h"
#include "Debug.h"
#include "Log.h"
#include "Types.h"
#include "GPUTimestampManager.h"
#include "Buffer.h"
#include "Texture.h"
#include "Sampler.h"
#include "RenderPass.h"
#include "ResourcePool.h"

#define VULKAN_DEBUG

namespace Raptor
{
namespace Graphics
{
class GPUDevice
{
    using Allocator = eastl::allocator;

public:

    GPUDevice(Window& window, Allocator& allocator, uint32 flags = 0, uint32 gpu_time_queries_per_frame = 32);
    ~GPUDevice();

    GPUDevice(const GPUDevice &) = delete;
    GPUDevice &operator=(const GPUDevice &) = delete;

    Window* window;
    Allocator* allocator;

    const char* version();

private:

    // Vulkan Instance
    void CreateInstance();
    void DestroyInstance();

    void CreateDebugUtilsMessenger();
    void DestroyDebugUtilsMessenger();

    void CreateSurface();
    void DestroySurface();

    void CreatePhysicalDevices();
    void DestroyPhysicalDevices();

    void SetSurfaceFormat();
    bool SetPresentMode(VkPresentModeKHR requested_present_mode = VK_PRESENT_MODE_FIFO_KHR);

    void CreateSwapChain();
    void DestroySwapChain();

    void CreateVmaAllocator();
    void DestroyVmaAllocator();

    void CreatePools(uint32 gpu_time_queries_per_frame);
    void DestroyPools();

    void CreateSemaphores();
    void DestroySemaphores();

    void CreateGPUTimestampManager(uint32 gpu_time_queries_per_frame);
    void DestroyGPUTimestampManager();

    void CreateSampler();
    void CreateBuffer();
    void CreateTexture();
    void CreateRenderPass();

    VkBool32 GetFamilyQueue(VkPhysicalDevice pDevice);

private:
    
    VkInstance vk_instance;
    VkAllocationCallbacks* vk_allocation_callbacks;
    VkDebugUtilsMessengerEXT vk_debug_utils_messenger;
    VkPhysicalDevice vk_physical_device;
    VkPhysicalDeviceProperties vk_physical_device_properties;
    float gpu_timestamp_frequency;
    VkSurfaceKHR vk_surface;
    VkSurfaceFormatKHR vk_surface_format;
    VkPresentModeKHR vk_present_mode;
    uint32 main_queue_family_index;
    VkDevice vk_device;
    VkQueue vk_queue;
    VkSwapchainKHR vk_swapchain;
    uint16 swapchain_width;
    uint16 swapchain_height;
    uint32 swapchain_image_count;
    static const uint32 MAX_SWAPCHAIN_IMAGES = 3;
    VkImage vk_swapchain_images[MAX_SWAPCHAIN_IMAGES];
    VkImageView vk_swapchain_image_views[MAX_SWAPCHAIN_IMAGES];
    VkFramebuffer vk_swapchain_framebuffers[MAX_SWAPCHAIN_IMAGES];
    VmaAllocator vma_allocator;
    VkDescriptorPool vk_descriptor_pool;
    VkQueryPool vk_query_pool;
    static const uint32 MAX_FRAMES = 3;
    VkSemaphore vk_image_acquired_semaphore;
    VkSemaphore vk_render_complete_semaphore[MAX_SWAPCHAIN_IMAGES];
    VkFence vk_command_buffer_executed_fence[MAX_SWAPCHAIN_IMAGES];
    static const uint32 QUERIES_PER_FRAME = 32;
    GPUTimestampManager* gpu_timestamp_manager = nullptr;



    ResourcePool buffers;
    ResourcePool textures;
    ResourcePool pipelines;
    ResourcePool samplers;
    ResourcePool descriptor_set_layouts;
    ResourcePool descriptor_sets;
    ResourcePool render_passes;
    ResourcePool command_buffers;
    ResourcePool shaders;

    enum Flags : uint32
    {
        DebugUtilsExtensionExist        = 0x1 << 0,
        EnableGPUTimeQueries            = 0x1 << 1,
    };
    uint32 m_uFlags = 0u;

}; // class GPUDevice

static bool check_result(VkResult result);

#ifdef VULKAN_DEBUG
static VkBool32 DebugUtilsCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT types, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data);
#endif

} // namespace Graphics
} // namespace Raptor