#pragma once

#if (_MSC_VER)
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#define VK_ENABLE_BETA_EXTENSIONS
#include <vulkan/vulkan.h>
#include <VMA/vk_mem_alloc.h>
#include <EASTL/allocator.h>

#include "Window.h"
#include "Debug.h"
#include "Log.h"
#include "Type.h"
#include "GPUTimestampManager.h"

#define VULKAN_DEBUG

namespace Raptor
{
namespace Graphics
{

class Vulkan
{
    using Allocator = eastl::allocator;

public:

    Vulkan(Window& window, Allocator& allocator);
    ~Vulkan();

    Vulkan(const Vulkan &) = delete;
    Vulkan &operator=(const Vulkan &) = delete;

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
    bool SetPresentMode(VkPresentModeKHR requestedPresentMode = VK_PRESENT_MODE_FIFO_KHR);

    void CreateSwapChain();
    void DestroySwapChain();

    void CreateVmaAllocator();
    void DestroyVmaAllocator();

    void CreatePools();
    void DestroyPools();

    void CreateSemaphores();
    void DestroySemaphores();

    void CreateGPUTimestampManager();
    void DestroyGPUTimestampManager();

    void CreateSampler();
    void CreateBuffer();
    void CreateTexture();
    void CreateRenderPass();

    VkBool32 GetFamilyQueue(VkPhysicalDevice pDevice);

private:
    
    VkInstance instance;
    VkAllocationCallbacks* allocationCallbacks;
    VkDebugUtilsMessengerEXT debugUtilsMessenger;
    VkPhysicalDevice physicalDevice;
    VkPhysicalDeviceProperties physicalDeviceProperties;
    VkSurfaceKHR surface;
    VkSurfaceFormatKHR surfaceFormat;
    VkPresentModeKHR presentMode;
    uint32 mainQueueFamilyIndex;
    VkDevice device;
    VkQueue queue;
    VkSwapchainKHR swapchain;
    uint16 swapchainWidth;
    uint16 swapchainHeight;
    uint32 swapchainImageCount;
    static const uint32 MAX_SWAPCHAIN_IMAGES = 3;
    VkImage swapchainImages[MAX_SWAPCHAIN_IMAGES];
    VkImageView swapchainImageViews[MAX_SWAPCHAIN_IMAGES];
    VkFramebuffer swapchainFramebuffers[MAX_SWAPCHAIN_IMAGES];
    VmaAllocator vmaAllocator;
    VkDescriptorPool descriptorPool;
    VkQueryPool queryPool;
    static const uint32 MAX_FRAMES = 3;
    VkSemaphore imageAcquiredSemaphore;
    VkSemaphore renderCompleteSemaphore[MAX_SWAPCHAIN_IMAGES];
    VkFence commandBufferExecutedFence[MAX_SWAPCHAIN_IMAGES];
    static const uint32 QUERIES_PER_FRAME = 32;
    GPUTimestampManager* gpuTimestampManager = nullptr;

    float gpuTimestampFrequency;

    enum Flags
    {
        DebugUtilsExtensionExist        = 0x1 << 0,
    };
    uint32 uFlags = 0u;

}; // class Vulkan

static bool check_result(VkResult result);

#ifdef VULKAN_DEBUG
static VkBool32 DebugUtilsCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT types, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data);
#endif

} // namespace Graphics
} // namespace Raptor