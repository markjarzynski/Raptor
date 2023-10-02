#pragma once

#if (_MSC_VER)
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#include <vulkan/vulkan.h>
#include "Window.h"
#include "Debug.h"
#include "Log.h"
#include "Type.h"

#define VULKAN_DEBUG

namespace Raptor
{
namespace Graphics
{
class Vulkan
{
public:

    Vulkan(Window& window);
    ~Vulkan();

    Vulkan(const Vulkan &) = delete;
    Vulkan &operator=(const Vulkan &) = delete;

    Window* window;

    const char* version();

private:

    // Vulkan Instance
    void CreateInstance();
    void DestroyInstance();

    void CreatePhysicalDevices();
    void DestroyPhysicalDevices();

    void CreateSurface();
    void DestroySurface();

    VkBool32 GetFamilyQueue(VkPhysicalDevice pDevice);

private:
    
    VkInstance instance;
    VkAllocationCallbacks* allocationCallbacks;
    VkDebugUtilsMessengerEXT debugUtilsMessenger;
    VkPhysicalDevice physicalDevice;
    VkPhysicalDeviceProperties physicalDeviceProperties;
    VkSurfaceKHR surface;
    uint32 mainQueueFamilyIndex;

    enum Flags
    {
        DebugUtilsExtensionExist        = 0x1 << 0,
        DynamicRenderingExtensionExists = 0x1 << 1,
    };
    uint32 flags = 0u;

}; // class Vulkan

static bool check_result(VkResult result);

#ifdef VULKAN_DEBUG
static VkBool32 DebugUtilsCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT types, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data);
#endif

} // namespace Graphics
} // namespace Raptor