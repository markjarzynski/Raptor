#define VMA_IMPLEMENTATION
#include "Vulkan.h"
#include "Config.h"
#include "Debug.h"
//#include "Type.h"
#include "Defines.h"
#include "CommandBuffer.h"
#include <EAStdC/EASprintf.h>
#include <EASTL/fixed_vector.h>
#include <EASTL/algorithm.h>

namespace Raptor
{
namespace Graphics
{

using eastl::clamp;

static const char* s_requested_layers[] = {
#ifdef VULKAN_DEBUG
    "VK_LAYER_KHRONOS_validation",
#else
    "",
#endif
};

static const char* s_requested_extensions[] = {
    VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef VK_USE_PLATFORM_WIN32_KHR
    VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
#ifdef VULKAN_DEBUG
    VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
};

PFN_vkSetDebugUtilsObjectNameEXT pfnSetDebugUtilsObjectNameEXT;
PFN_vkCmdBeginDebugUtilsLabelEXT pfnCmdBeginDebugUtilsLabelEXT;
PFN_vkCmdEndDebugUtilsLabelEXT pfnCmdEndDebugUtilsLabelEXT;

Vulkan::Vulkan(Window& window, Allocator& allocator)
    : window(&window), allocator(&allocator)
{
    CreateInstance();
    CreateDebugUtilsMessenger();
    CreateSurface();
    CreatePhysicalDevices();
    SetSurfaceFormat();
    SetPresentMode();
    CreateSwapChain();
    CreateVmaAllocator();
    CreatePools();
    CreateSemaphores();
    CreateGPUTimestampManager();
}

Vulkan::~Vulkan()
{
    DestroyGPUTimestampManager();
    DestroySemaphores();
    DestroyPools();
    DestroyVmaAllocator();
    DestroySwapChain();
    DestroyPhysicalDevices();
    DestroySurface();
    DestroyDebugUtilsMessenger();
    DestroyInstance();
}

//------------------------------------------------------------------------------
void Vulkan::CreateInstance()
{
    VkResult result;
    allocationCallbacks = nullptr;

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = window->GetName();
    appInfo.applicationVersion = 1; //VK_MAKE_VERSION(RAPTOR_VERSION_MAJOR, RAPTOR_VERSION_MINOR, RAPTOR_VERSION_PATCH);
    appInfo.pEngineName = RAPTOR_PROJECT_NAME;
    appInfo.engineVersion = 1; //VK_MAKE_VERSION(RAPTOR_VERSION_MAJOR, RAPTOR_VERSION_MINOR, RAPTOR_VERSION_PATCH);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledLayerCount = ARRAY_SIZE(s_requested_layers);
    createInfo.ppEnabledLayerNames = s_requested_layers;
    createInfo.enabledExtensionCount = ARRAY_SIZE(s_requested_extensions);
    createInfo.ppEnabledExtensionNames = s_requested_extensions;

    result = vkCreateInstance(&createInfo, allocationCallbacks, &instance);
    ASSERT_MESSAGE(result == VK_SUCCESS, "[Vulkan] Error: Failed to create instance. code(%u).", result);
}

//------------------------------------------------------------------------------
void Vulkan::DestroyInstance()
{
    vkDestroyInstance(instance, allocationCallbacks);
}

//------------------------------------------------------------------------------
void Vulkan::CreateDebugUtilsMessenger()
{
#ifdef VULKAN_DEBUG
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugCreateInfo.pfnUserCallback = DebugUtilsCallback;
    debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;

    uint32 numExtensions;
    vkEnumerateInstanceExtensionProperties( nullptr, &numExtensions, nullptr );

    eastl::vector<VkExtensionProperties> extensions(numExtensions);
    vkEnumerateInstanceExtensionProperties(nullptr, &numExtensions, extensions.data());

    for (VkExtensionProperties ext : extensions)
    {
        if (strcmp(ext.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0)
        {
            uFlags |= DebugUtilsExtensionExist;
            break;
        }
    }

    if (uFlags & Flags::DebugUtilsExtensionExist)
    {
        auto vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (vkCreateDebugUtilsMessengerEXT != nullptr)
            vkCreateDebugUtilsMessengerEXT(instance, &debugCreateInfo, allocationCallbacks, &debugUtilsMessenger);
        else
            Raptor::Debug::Log("[Vulkan] Warning: Failed to setup debug messenger.");
    }
    else
    {
        Raptor::Debug::Log("[Vulkan] Warning: Extension %s for debugging does not exist!", VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
#endif
}

//------------------------------------------------------------------------------
void Vulkan::DestroyDebugUtilsMessenger()
{
#ifdef VULKAN_DEBUG
    if (uFlags & Flags::DebugUtilsExtensionExist)
    {
        PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (vkDestroyDebugUtilsMessengerEXT != nullptr)
            vkDestroyDebugUtilsMessengerEXT(instance, debugUtilsMessenger, allocationCallbacks);
    }
#endif
}

#ifdef VULKAN_DEBUG
static VkBool32 DebugUtilsCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT types, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data)
{
    ASSERT_MESSAGE(!(severity & (VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)),
        "Message ID: %s %i\nMessage: %s\n", 
        callback_data->pMessageIdName, callback_data->messageIdNumber, callback_data->pMessage);

    return VK_FALSE;
}
#endif

//------------------------------------------------------------------------------
void Vulkan::CreateSurface()
{
    VkResult result = glfwCreateWindowSurface(instance, window->GetGLFWwindow(), allocationCallbacks, &surface);
    ASSERT_MESSAGE(result == VK_SUCCESS, "[Vulkan] Error: Failed to create window surface.");
}

void Vulkan::DestroySurface()
{
    vkDestroySurfaceKHR(instance, surface, allocationCallbacks);
}

//------------------------------------------------------------------------------
void Vulkan::CreatePhysicalDevices()
{
    VkResult result;
    uint32 numPhysicalDevices;
     
    result = vkEnumeratePhysicalDevices(instance, &numPhysicalDevices, NULL);
    ASSERT_MESSAGE(result == VK_SUCCESS, "[Vulkan] Error: Failed to enumerate physical devices. code(%u).", result);

    eastl::vector<VkPhysicalDevice> physicalDevices(numPhysicalDevices);   
    result = vkEnumeratePhysicalDevices(instance, &numPhysicalDevices, physicalDevices.data());
    ASSERT_MESSAGE(result == VK_SUCCESS, "[Vulkan] Error: Failed to enumerate physical devices. code(%u).", result);

    VkPhysicalDevice discrete = VK_NULL_HANDLE;
    VkPhysicalDevice integrated = VK_NULL_HANDLE;
    for (VkPhysicalDevice pDevice : physicalDevices)
    {
        vkGetPhysicalDeviceProperties(pDevice, &physicalDeviceProperties);

        if (physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            if (GetFamilyQueue(pDevice))
            {
                // Prefer the first discrete gpu with present capabilities.
                discrete = pDevice;
                break;
            }
        }
        else if (physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
        {
            if (GetFamilyQueue(pDevice))
            {
                integrated = pDevice;
            }
        }
    }

    if (discrete != VK_NULL_HANDLE)
    {
        physicalDevice = discrete;
    }
    else if (integrated != VK_NULL_HANDLE)
    {
        physicalDevice = integrated;
    }
    else
    {
        ASSERT_MESSAGE(false, "[Vulkan] Error: Failed to find a suitable GPU device.");
    }

    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
    Raptor::Debug::Log("[Vulkan] Info: GPU Used: %s\n", physicalDeviceProperties.deviceName);

    gpuTimestampFrequency = physicalDeviceProperties.limits.timestampPeriod / (1000 * 1000);
    //uboAlignment = physicalDeviceProperties.limits.minUniformBufferOffsetAlignment;
    //ssboAlignment = physicalDeviceProperties.limits.minStorageBufferOffsetAlignment;

    eastl::vector<const char*> deviceExtensions;
    deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    const float queuePriority[] = { 1.f };
    VkDeviceQueueCreateInfo queueInfo[1] = {};
    queueInfo[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo[0].queueFamilyIndex = mainQueueFamilyIndex;
    queueInfo[0].queueCount = 1;
    queueInfo[0].pQueuePriorities = queuePriority;

    VkPhysicalDeviceFeatures2 physicalFeatures2 {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
    vkGetPhysicalDeviceFeatures2(physicalDevice, &physicalFeatures2);
    
    VkDeviceCreateInfo deviceCreateInfo {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = sizeof(queueInfo) / sizeof(queueInfo[0]);
    deviceCreateInfo.pQueueCreateInfos = queueInfo;
    deviceCreateInfo.enabledExtensionCount = deviceExtensions.size();
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
    deviceCreateInfo.pNext = &physicalFeatures2;

    result = vkCreateDevice(physicalDevice, &deviceCreateInfo, allocationCallbacks, &device);
    ASSERT_MESSAGE(result == VK_SUCCESS, "[Vulkan] Error: Failed to create logical device.");

    if (uFlags |= Flags::DebugUtilsExtensionExist)
    {
        pfnSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT");
        pfnCmdBeginDebugUtilsLabelEXT = (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetDeviceProcAddr(device, "vkCmdBeginDebugUtilsLabelEXT");
        pfnCmdEndDebugUtilsLabelEXT = (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetDeviceProcAddr(device, "vkCmdEndDebugUtilsLabelExt");
    }

    vkGetDeviceQueue(device, mainQueueFamilyIndex, 0, &queue);
}

//------------------------------------------------------------------------------
void Vulkan::DestroyPhysicalDevices()
{
    vkDestroyDevice(device, allocationCallbacks);
}

void Vulkan::SetSurfaceFormat()
{
    const VkFormat surfaceImageFormats[] = {VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM};
    const VkColorSpaceKHR surfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;

    uint32 surfaceSupportedFormatsCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceSupportedFormatsCount, NULL);
    eastl::vector<VkSurfaceFormatKHR> surfaceSupportedFormats(surfaceSupportedFormatsCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceSupportedFormatsCount, surfaceSupportedFormats.data());

    // Check for supported formats.
    const uint32 surfaceImageFormatsCount = ARRAY_SIZE(surfaceImageFormats);
    for (uint32 iImageFormats = 0; iImageFormats < surfaceImageFormatsCount; iImageFormats++)
    {
        for (uint32 iSupportedFormats = 0; iSupportedFormats < surfaceSupportedFormatsCount; iSupportedFormats++)
        {
            if (surfaceSupportedFormats[iSupportedFormats].format == surfaceImageFormats[iImageFormats] &&
                surfaceSupportedFormats[iSupportedFormats].colorSpace == surfaceColorSpace)
            {
                surfaceFormat = surfaceSupportedFormats[iSupportedFormats];
                return;
            }
        }
    }

    // Default to the first surface format supported.
    surfaceFormat = surfaceSupportedFormats[0];
}

bool Vulkan::SetPresentMode(VkPresentModeKHR requestedPresentMode)
{
    uint32 surfacePresentModesCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &surfacePresentModesCount, NULL);
    eastl::vector<VkPresentModeKHR> surfacePresentModes(surfacePresentModesCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &surfacePresentModesCount, surfacePresentModes.data());

    for (uint32 iPresentMode = 0; iPresentMode < surfacePresentModesCount; iPresentMode++)
    {
        if (requestedPresentMode == surfacePresentModes[iPresentMode])
        {
            presentMode = requestedPresentMode;
            Raptor::Debug::Log("[Vulkan] Info: Set present mode to: %d.\n", presentMode);
            return true;
        }
    }

    // Default to VK_PRESENT_MODE_FIFO_KHR if the requested present mode is not found.
    presentMode = VK_PRESENT_MODE_FIFO_KHR;
    Raptor::Debug::Log("[Vulkan] Warning: Could not set present mode to requested present mode: %d, defaulting to present mode: %d.", requestedPresentMode, presentMode);
    return false;
}

//------------------------------------------------------------------------------
void Vulkan::CreateSwapChain()
{
    VkResult result;
    VkBool32 surfaceSupported;

    swapchainImageCount = 3;

    // Create swapchain
    result = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, mainQueueFamilyIndex, surface, &surfaceSupported);
    ASSERT_MESSAGE(result == VK_SUCCESS, "[Vulkan] Error: No WSI support on physical device 0.");

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);

    VkExtent2D swapchainExtent = surfaceCapabilities.currentExtent;
    if (swapchainExtent.width == UINT32_MAX)
    {
        swapchainExtent.width = clamp(swapchainExtent.width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
        swapchainExtent.height = clamp(swapchainExtent.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
    }

    Raptor::Debug::Log("[Vulkan] Info: Create swapchain %u %u, min image %u.\n", swapchainExtent.width, swapchainExtent.height, surfaceCapabilities.minImageCount);

    swapchainWidth = (uint16)swapchainExtent.width;
    swapchainHeight = (uint16)swapchainExtent.height;

    VkSwapchainCreateInfoKHR swapchainCreateInfo {};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface = surface;
    swapchainCreateInfo.minImageCount = swapchainImageCount;
    swapchainCreateInfo.imageFormat = surfaceFormat.format;
    swapchainCreateInfo.imageExtent = swapchainExtent;
    swapchainCreateInfo.clipped = VK_TRUE;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode = presentMode;

    result = vkCreateSwapchainKHR(device, &swapchainCreateInfo, 0, &swapchain);
    ASSERT_MESSAGE(result == VK_SUCCESS, "[Vulkan] Error: Failed to create swapchain.");

    vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, NULL);
    vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, swapchainImages);

    for (uint32 iImage = 0; iImage < swapchainImageCount; iImage++)
    {
        VkImageViewCreateInfo viewInfo {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = surfaceFormat.format;
        viewInfo.image = swapchainImages[iImage];
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.layerCount = 1;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
        viewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
        viewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
        viewInfo.components.a = VK_COMPONENT_SWIZZLE_A;

        result = vkCreateImageView(device, &viewInfo, allocationCallbacks, &swapchainImageViews[iImage]);
        ASSERT_MESSAGE(result == VK_SUCCESS, "[Vulkan] Error: Failed to create Image View %d out of %d.", iImage, swapchainImageCount);
    }
}

//------------------------------------------------------------------------------
void Vulkan::DestroySwapChain()
{
    for (uint32 iImage = 0; iImage < swapchainImageCount; iImage++)
    {
        vkDestroyImageView(device, swapchainImageViews[iImage], allocationCallbacks);
        //vkDestroyFramebuffer(device, swapchainFramebuffers[iImage], allocationCallbacks);
    }
    vkDestroySwapchainKHR(device, swapchain, allocationCallbacks);
}

//------------------------------------------------------------------------------
void Vulkan::CreateVmaAllocator()
{
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = physicalDevice;
    allocatorInfo.device = device;
    allocatorInfo.instance = instance;

    VkResult result = vmaCreateAllocator(&allocatorInfo, &vmaAllocator);
    ASSERT_MESSAGE(result == VK_SUCCESS, "[Vulkan] Error: Failed to create VMA Allocator.");
}

//------------------------------------------------------------------------------
void Vulkan::DestroyVmaAllocator()
{
    vmaDestroyAllocator(vmaAllocator);
}

//------------------------------------------------------------------------------
void Vulkan::CreatePools()
{
    VkResult result;

    const uint32 GLOBAL_POOL_ELEMENTS = 128;

    VkDescriptorPoolSize poolSizes[] = 
    {
        { VK_DESCRIPTOR_TYPE_SAMPLER,                GLOBAL_POOL_ELEMENTS },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, GLOBAL_POOL_ELEMENTS },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          GLOBAL_POOL_ELEMENTS },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          GLOBAL_POOL_ELEMENTS },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   GLOBAL_POOL_ELEMENTS },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   GLOBAL_POOL_ELEMENTS },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         GLOBAL_POOL_ELEMENTS },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         GLOBAL_POOL_ELEMENTS },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, GLOBAL_POOL_ELEMENTS },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, GLOBAL_POOL_ELEMENTS },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       GLOBAL_POOL_ELEMENTS },
    };

    VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
    descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    descriptorPoolInfo.maxSets = GLOBAL_POOL_ELEMENTS * ARRAY_SIZE(poolSizes);
    descriptorPoolInfo.poolSizeCount = (uint32)ARRAY_SIZE(poolSizes);
    descriptorPoolInfo.pPoolSizes = poolSizes;

    result = vkCreateDescriptorPool(device, &descriptorPoolInfo, allocationCallbacks, &descriptorPool);
    ASSERT_MESSAGE(result == VK_SUCCESS, "[Vulkan] Error: Failed to create descriptor pool.");

    const uint32 GPU_TIME_QUERIES_PER_FRAME = 32;

    VkQueryPoolCreateInfo queryPoolInfo = {};
    queryPoolInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    queryPoolInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
    queryPoolInfo.queryCount = GPU_TIME_QUERIES_PER_FRAME * 2u * MAX_FRAMES;

    result = vkCreateQueryPool(device, &queryPoolInfo, allocationCallbacks, &queryPool);
    ASSERT_MESSAGE(result == VK_SUCCESS, "[Vulkan] Error: Failed to create query pool.");

    // init pools???
}

//------------------------------------------------------------------------------
void Vulkan::DestroyPools()
{
    vkDestroyQueryPool(device, queryPool, allocationCallbacks);
    vkDestroyDescriptorPool(device, descriptorPool, allocationCallbacks);
}

//------------------------------------------------------------------------------
void Vulkan::CreateSemaphores()
{
    VkResult result;

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    result = vkCreateSemaphore(device, &semaphoreInfo, allocationCallbacks, &imageAcquiredSemaphore);
    ASSERT_MESSAGE(result == VK_SUCCESS, "[Vulkan] Error: Failed to create semaphore.");

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (uint32 i = 0; i < MAX_SWAPCHAIN_IMAGES; i++)
    {
        result = vkCreateSemaphore(device, &semaphoreInfo, allocationCallbacks, &renderCompleteSemaphore[i]);
        ASSERT_MESSAGE(result == VK_SUCCESS, "[Vulkan] Error: Failed to create semaphore %d.", i);

        result = vkCreateFence(device, &fenceInfo, allocationCallbacks, &commandBufferExecutedFence[i]);
        ASSERT_MESSAGE(result == VK_SUCCESS, "[Vulkan] Error: Failed to create fence %d.", i);
    }


}

//------------------------------------------------------------------------------
void Vulkan::DestroySemaphores()
{
    for (uint32 i = 0; i < MAX_SWAPCHAIN_IMAGES; i++)
    {
        vkDestroyFence(device, commandBufferExecutedFence[i], allocationCallbacks);
        vkDestroySemaphore(device, renderCompleteSemaphore[i], allocationCallbacks);
    }

    vkDestroySemaphore(device, imageAcquiredSemaphore, allocationCallbacks);
}

//------------------------------------------------------------------------------
void Vulkan::CreateGPUTimestampManager()
{
    uint8* memory = (uint8*)allocator->allocate(sizeof(GPUTimestampManager) + sizeof(CommandBuffer*) * 128);
    //gpuTimestampManager = (GPUTimestampManager*)memory;
    gpuTimestampManager = new (memory)GPUTimestampManager(allocator, QUERIES_PER_FRAME, MAX_FRAMES);
}

//------------------------------------------------------------------------------
void Vulkan::DestroyGPUTimestampManager()
{

}

//------------------------------------------------------------------------------
void Vulkan::CreateSampler(){}
//------------------------------------------------------------------------------
void Vulkan::CreateBuffer(){}
//------------------------------------------------------------------------------
void Vulkan::CreateTexture(){}
//------------------------------------------------------------------------------
void Vulkan::CreateRenderPass(){}


//------------------------------------------------------------------------------
VkBool32 Vulkan::GetFamilyQueue(VkPhysicalDevice pDevice)
{
    uint32 familyQueueCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &familyQueueCount, nullptr);

    eastl::vector<VkQueueFamilyProperties> queueFamilies(familyQueueCount);
    vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &familyQueueCount, queueFamilies.data());

    VkBool32 surfaceSupported = VK_FALSE;

    for (uint32 iQueueFamily = 0; iQueueFamily < queueFamilies.size(); ++iQueueFamily)
    {
        if (queueFamilies[iQueueFamily].queueCount > 0 && queueFamilies[iQueueFamily].queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT))
        {
            vkGetPhysicalDeviceSurfaceSupportKHR(pDevice, iQueueFamily, surface, &surfaceSupported);

            if (surfaceSupported)
            {
                mainQueueFamilyIndex = iQueueFamily;
                break;
            }
        }
    }

    return surfaceSupported;
}

//------------------------------------------------------------------------------

} // namespace Graphics
} // namespace Raptor