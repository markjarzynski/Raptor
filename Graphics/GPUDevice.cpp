#if (_MSC_VER)
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#define VK_ENABLE_BETA_EXTENSIONS
#include <vulkan/vulkan.h>
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <EAStdC/EASprintf.h>
#include <EASTL/fixed_vector.h>
#include <EASTL/algorithm.h>

#include "GPUDevice.h"
#include "Raptor.h"
#include "Defines.h"
#include "DescriptorSet.h"
#include "DescriptorSetLayout.h"
#include "ShaderState.h"
#include "Pipeline.h"
#include "CommandBuffer.h"
#include "CommandBufferRing.h"

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

static CommandBufferRing* command_buffer_ring;

//------------------------------------------------------------------------------
GPUDevice::GPUDevice(Window& window, Allocator& allocator, uint32 flags, uint32 gpu_time_queries_per_frame)
    : window(&window), allocator(&allocator), m_uFlags(flags)
{
    CreateInstance();
    CreateDebugUtilsMessenger();
    CreateSurface();
    CreatePhysicalDevices();
    SetSurfaceFormat();
    SetPresentMode();
    CreateSwapChain();
    CreateVmaAllocator();
    CreatePools(gpu_time_queries_per_frame);
    CreateSemaphores();
    CreateGPUTimestampManager(gpu_time_queries_per_frame);
    CreateCommandBuffers();

    image_index = 0;
    current_frame = 1;
    previous_frame = 0;
    absolute_frame = 0;
    //m_uFlags &= ~Flags::TimestampsEnabled;

    resource_deleting_queue.set_allocator(allocator);
    descriptor_set_updates.set_allocator(allocator);

    default_sampler = CreateSampler("Sampler Default", VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
    fullscreen_vertex_buffer = CreateBuffer("Fullscreen Vertex Buffer", ResourceUsageType::Immutable, 0, nullptr, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    depth_texture = CreateTexture("Depth Texture", nullptr, VK_FORMAT_D32_SFLOAT, Texture::Type::Texture2D, window.width, window.height, 1, 1, 0);

}

//------------------------------------------------------------------------------
GPUDevice::~GPUDevice()
{
    DestroyCommandBuffers();
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
void GPUDevice::CreateInstance()
{
    VkResult result;
    vk_allocation_callbacks = nullptr;

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = window->GetName();
    appInfo.applicationVersion = 1; //VK_MAKE_VERSION(RAPTOR_VERSION_MAJOR, RAPTOR_VERSION_MINOR, RAPTOR_VERSION_PATCH);
    appInfo.pEngineName = "";//RAPTOR_PROJECT_NAME;
    appInfo.engineVersion = 1; //VK_MAKE_VERSION(RAPTOR_VERSION_MAJOR, RAPTOR_VERSION_MINOR, RAPTOR_VERSION_PATCH);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledLayerCount = ARRAY_SIZE(s_requested_layers);
    createInfo.ppEnabledLayerNames = s_requested_layers;
    createInfo.enabledExtensionCount = ARRAY_SIZE(s_requested_extensions);
    createInfo.ppEnabledExtensionNames = s_requested_extensions;

    result = vkCreateInstance(&createInfo, vk_allocation_callbacks, &vk_instance);
    ASSERT_MESSAGE(result == VK_SUCCESS, "[Vulkan] Error: Failed to create vk_instance. code(%u).", result);
}

//------------------------------------------------------------------------------
void GPUDevice::DestroyInstance()
{
    vkDestroyInstance(vk_instance, vk_allocation_callbacks);
}

//------------------------------------------------------------------------------
void GPUDevice::CreateDebugUtilsMessenger()
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
            m_uFlags |= Flags::DebugUtilsExtensionExist;
            break;
        }
    }

    if (m_uFlags & Flags::DebugUtilsExtensionExist)
    {
        auto vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vk_instance, "vkCreateDebugUtilsMessengerEXT");
        if (vkCreateDebugUtilsMessengerEXT != nullptr)
            vkCreateDebugUtilsMessengerEXT(vk_instance, &debugCreateInfo, vk_allocation_callbacks, &vk_debug_utils_messenger);
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
void GPUDevice::DestroyDebugUtilsMessenger()
{
#ifdef VULKAN_DEBUG
    if (m_uFlags & Flags::DebugUtilsExtensionExist)
    {
        PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vk_instance, "vkDestroyDebugUtilsMessengerEXT");
        if (vkDestroyDebugUtilsMessengerEXT != nullptr)
            vkDestroyDebugUtilsMessengerEXT(vk_instance, vk_debug_utils_messenger, vk_allocation_callbacks);
    }
#endif
}

//------------------------------------------------------------------------------
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
void GPUDevice::CreateSurface()
{
    VkResult result = glfwCreateWindowSurface(vk_instance, window->GetGLFWwindow(), vk_allocation_callbacks, &vk_surface);
    ASSERT_MESSAGE(result == VK_SUCCESS, "[Vulkan] Error: Failed to create window vk_surface.");
}

//------------------------------------------------------------------------------
void GPUDevice::DestroySurface()
{
    vkDestroySurfaceKHR(vk_instance, vk_surface, vk_allocation_callbacks);
}

//------------------------------------------------------------------------------
void GPUDevice::CreatePhysicalDevices()
{
    VkResult result;
    uint32 numPhysicalDevices;
     
    result = vkEnumeratePhysicalDevices(vk_instance, &numPhysicalDevices, NULL);
    ASSERT_MESSAGE(result == VK_SUCCESS, "[Vulkan] Error: Failed to enumerate physical devices. code(%u).", result);

    eastl::vector<VkPhysicalDevice> physicalDevices(numPhysicalDevices);   
    result = vkEnumeratePhysicalDevices(vk_instance, &numPhysicalDevices, physicalDevices.data());
    ASSERT_MESSAGE(result == VK_SUCCESS, "[Vulkan] Error: Failed to enumerate physical devices. code(%u).", result);

    VkPhysicalDevice discrete = VK_NULL_HANDLE;
    VkPhysicalDevice integrated = VK_NULL_HANDLE;
    for (VkPhysicalDevice pDevice : physicalDevices)
    {
        vkGetPhysicalDeviceProperties(pDevice, &vk_physical_device_properties);

        if (vk_physical_device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            if (GetFamilyQueue(pDevice))
            {
                // Prefer the first discrete gpu with present capabilities.
                discrete = pDevice;
                break;
            }
        }
        else if (vk_physical_device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
        {
            if (GetFamilyQueue(pDevice))
            {
                integrated = pDevice;
            }
        }
    }

    if (discrete != VK_NULL_HANDLE)
    {
        vk_physical_device = discrete;
    }
    else if (integrated != VK_NULL_HANDLE)
    {
        vk_physical_device = integrated;
    }
    else
    {
        ASSERT_MESSAGE(false, "[Vulkan] Error: Failed to find a suitable GPU vk_device.");
    }

    vkGetPhysicalDeviceProperties(vk_physical_device, &vk_physical_device_properties);
    Raptor::Debug::Log("[Vulkan] Info: GPU Used: %s\n", vk_physical_device_properties.deviceName);

    gpu_timestamp_frequency = vk_physical_device_properties.limits.timestampPeriod / (1000 * 1000);
    //uboAlignment = physicalDeviceProperties.limits.minUniformBufferOffsetAlignment;
    //ssboAlignment = physicalDeviceProperties.limits.minStorageBufferOffsetAlignment;

    eastl::vector<const char*> deviceExtensions;
    deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    const float queuePriority[] = { 1.f };
    VkDeviceQueueCreateInfo queueInfo[1] = {};
    queueInfo[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo[0].queueFamilyIndex = main_queue_family_index;
    queueInfo[0].queueCount = 1;
    queueInfo[0].pQueuePriorities = queuePriority;

    VkPhysicalDeviceFeatures2 physicalFeatures2 {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
    vkGetPhysicalDeviceFeatures2(vk_physical_device, &physicalFeatures2);
    
    VkDeviceCreateInfo deviceCreateInfo {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = sizeof(queueInfo) / sizeof(queueInfo[0]);
    deviceCreateInfo.pQueueCreateInfos = queueInfo;
    deviceCreateInfo.enabledExtensionCount = deviceExtensions.size();
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
    deviceCreateInfo.pNext = &physicalFeatures2;

    result = vkCreateDevice(vk_physical_device, &deviceCreateInfo, vk_allocation_callbacks, &vk_device);
    ASSERT_MESSAGE(result == VK_SUCCESS, "[Vulkan] Error: Failed to create logical vk_device.");

    if (m_uFlags |= Flags::DebugUtilsExtensionExist)
    {
        pfnSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(vk_device, "vkSetDebugUtilsObjectNameEXT");
        pfnCmdBeginDebugUtilsLabelEXT = (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetDeviceProcAddr(vk_device, "vkCmdBeginDebugUtilsLabelEXT");
        pfnCmdEndDebugUtilsLabelEXT = (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetDeviceProcAddr(vk_device, "vkCmdEndDebugUtilsLabelExt");
    }

    vkGetDeviceQueue(vk_device, main_queue_family_index, 0, &vk_queue);
}

//------------------------------------------------------------------------------
void GPUDevice::DestroyPhysicalDevices()
{
    vkDestroyDevice(vk_device, vk_allocation_callbacks);
}

//------------------------------------------------------------------------------
void GPUDevice::SetSurfaceFormat()
{
    const VkFormat surfaceImageFormats[] = {VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM};
    const VkColorSpaceKHR surfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;

    uint32 surfaceSupportedFormatsCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(vk_physical_device, vk_surface, &surfaceSupportedFormatsCount, NULL);
    eastl::vector<VkSurfaceFormatKHR> surfaceSupportedFormats(surfaceSupportedFormatsCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(vk_physical_device, vk_surface, &surfaceSupportedFormatsCount, surfaceSupportedFormats.data());

    // Check for supported formats.
    const uint32 surfaceImageFormatsCount = ARRAY_SIZE(surfaceImageFormats);
    for (uint32 iImageFormats = 0; iImageFormats < surfaceImageFormatsCount; iImageFormats++)
    {
        for (uint32 iSupportedFormats = 0; iSupportedFormats < surfaceSupportedFormatsCount; iSupportedFormats++)
        {
            if (surfaceSupportedFormats[iSupportedFormats].format == surfaceImageFormats[iImageFormats] &&
                surfaceSupportedFormats[iSupportedFormats].colorSpace == surfaceColorSpace)
            {
                vk_surface_format = surfaceSupportedFormats[iSupportedFormats];
                return;
            }
        }
    }

    // Default to the first vk_surface format supported.
    vk_surface_format = surfaceSupportedFormats[0];
}

//------------------------------------------------------------------------------
bool GPUDevice::SetPresentMode(VkPresentModeKHR requestedPresentMode)
{
    uint32 surfacePresentModesCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(vk_physical_device, vk_surface, &surfacePresentModesCount, NULL);
    eastl::vector<VkPresentModeKHR> surfacePresentModes(surfacePresentModesCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(vk_physical_device, vk_surface, &surfacePresentModesCount, surfacePresentModes.data());

    for (uint32 iPresentMode = 0; iPresentMode < surfacePresentModesCount; iPresentMode++)
    {
        if (requestedPresentMode == surfacePresentModes[iPresentMode])
        {
            vk_present_mode = requestedPresentMode;
            Raptor::Debug::Log("[Vulkan] Info: Set present mode to: %d.\n", vk_present_mode);
            return true;
        }
    }

    // Default to VK_PRESENT_MODE_FIFO_KHR if the requested present mode is not found.
    vk_present_mode = VK_PRESENT_MODE_FIFO_KHR;
    Raptor::Debug::Log("[Vulkan] Warning: Could not set present mode to requested present mode: %d, defaulting to present mode: %d.", requestedPresentMode, vk_present_mode);
    return false;
}

//------------------------------------------------------------------------------
void GPUDevice::CreateSwapChain()
{
    VkResult result;
    VkBool32 surfaceSupported;

    swapchain_image_count = 3;

    // Create vk_swapchain
    result = vkGetPhysicalDeviceSurfaceSupportKHR(vk_physical_device, main_queue_family_index, vk_surface, &surfaceSupported);
    ASSERT_MESSAGE(result == VK_SUCCESS, "[Vulkan] Error: No WSI support on physical vk_device 0.");

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk_physical_device, vk_surface, &surfaceCapabilities);

    VkExtent2D swapchainExtent = surfaceCapabilities.currentExtent;
    if (swapchainExtent.width == UINT32_MAX)
    {
        swapchainExtent.width = clamp(swapchainExtent.width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
        swapchainExtent.height = clamp(swapchainExtent.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
    }

    Raptor::Debug::Log("[Vulkan] Info: Create vk_swapchain %u %u, min image %u.\n", swapchainExtent.width, swapchainExtent.height, surfaceCapabilities.minImageCount);

    swapchain_width = (uint16)swapchainExtent.width;
    swapchain_height = (uint16)swapchainExtent.height;

    VkSwapchainCreateInfoKHR swapchainCreateInfo {};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface = vk_surface;
    swapchainCreateInfo.minImageCount = swapchain_image_count;
    swapchainCreateInfo.imageFormat = vk_surface_format.format;
    swapchainCreateInfo.imageExtent = swapchainExtent;
    swapchainCreateInfo.clipped = VK_TRUE;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode = vk_present_mode;

    result = vkCreateSwapchainKHR(vk_device, &swapchainCreateInfo, 0, &vk_swapchain);
    ASSERT_MESSAGE(result == VK_SUCCESS, "[Vulkan] Error: Failed to create vk_swapchain.");

    vkGetSwapchainImagesKHR(vk_device, vk_swapchain, &swapchain_image_count, NULL);
    vkGetSwapchainImagesKHR(vk_device, vk_swapchain, &swapchain_image_count, vk_swapchain_images);

    for (uint32 iImage = 0; iImage < swapchain_image_count; iImage++)
    {
        VkImageViewCreateInfo viewInfo {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = vk_surface_format.format;
        viewInfo.image = vk_swapchain_images[iImage];
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.layerCount = 1;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
        viewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
        viewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
        viewInfo.components.a = VK_COMPONENT_SWIZZLE_A;

        result = vkCreateImageView(vk_device, &viewInfo, vk_allocation_callbacks, &vk_swapchain_image_views[iImage]);
        ASSERT_MESSAGE(result == VK_SUCCESS, "[Vulkan] Error: Failed to create Image View %d out of %d.", iImage, swapchain_image_count);
    }
}

//------------------------------------------------------------------------------
void GPUDevice::DestroySwapChain()
{
    for (uint32 iImage = 0; iImage < swapchain_image_count; iImage++)
    {
        vkDestroyImageView(vk_device, vk_swapchain_image_views[iImage], vk_allocation_callbacks);
        //vkDestroyFramebuffer(vk_device, swapchainFramebuffers[iImage], vk_allocation_callbacks);
    }
    vkDestroySwapchainKHR(vk_device, vk_swapchain, vk_allocation_callbacks);
}

//------------------------------------------------------------------------------
void GPUDevice::CreateVmaAllocator()
{
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = vk_physical_device;
    allocatorInfo.device = vk_device;
    allocatorInfo.instance = vk_instance;

    VkResult result = vmaCreateAllocator(&allocatorInfo, &vma_allocator);
    ASSERT_MESSAGE(result == VK_SUCCESS, "[Vulkan] Error: Failed to create VMA Allocator.");
}

//------------------------------------------------------------------------------
void GPUDevice::DestroyVmaAllocator()
{
    vmaDestroyAllocator(vma_allocator);
}

//------------------------------------------------------------------------------
void GPUDevice::CreatePools(uint32 gpu_time_queries_per_frame)
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

    result = vkCreateDescriptorPool(vk_device, &descriptorPoolInfo, vk_allocation_callbacks, &vk_descriptor_pool);
    ASSERT_MESSAGE(result == VK_SUCCESS, "[Vulkan] Error: Failed to create descriptor pool.");

    VkQueryPoolCreateInfo queryPoolInfo = {};
    queryPoolInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    queryPoolInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
    queryPoolInfo.queryCount = gpu_time_queries_per_frame * 2u * MAX_FRAMES;

    result = vkCreateQueryPool(vk_device, &queryPoolInfo, vk_allocation_callbacks, &vk_query_pool);
    ASSERT_MESSAGE(result == VK_SUCCESS, "[Vulkan] Error: Failed to create query pool.");

    // init pools
    buffers.init(allocator, 4096, sizeof(Buffer));
    textures.init(allocator, 512, sizeof(Texture));
    render_passes.init(allocator, 256, sizeof(RenderPass));
    descriptor_set_layouts.init(allocator, 128, sizeof(DescriptorSetLayout));
    pipelines.init(allocator, 128, sizeof(Pipeline));
    shaders.init(allocator, 128, sizeof(ShaderState));
    descriptor_sets.init(allocator, 256, sizeof(DescriptorSet));
    samplers.init(allocator, 32, sizeof(Sampler));
}

//------------------------------------------------------------------------------
void GPUDevice::DestroyPools()
{
    samplers.shutdown();
    descriptor_sets.shutdown();
    shaders.shutdown();
    pipelines.shutdown();
    descriptor_set_layouts.shutdown();
    render_passes.shutdown();
    textures.shutdown();
    buffers.shutdown();

    vkDestroyQueryPool(vk_device, vk_query_pool, vk_allocation_callbacks);
    vkDestroyDescriptorPool(vk_device, vk_descriptor_pool, vk_allocation_callbacks);
}

//------------------------------------------------------------------------------
void GPUDevice::CreateSemaphores()
{
    VkResult result;

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    result = vkCreateSemaphore(vk_device, &semaphoreInfo, vk_allocation_callbacks, &vk_image_acquired_semaphore);
    ASSERT_MESSAGE(result == VK_SUCCESS, "[Vulkan] Error: Failed to create semaphore.");

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (uint32 i = 0; i < MAX_SWAPCHAIN_IMAGES; i++)
    {
        result = vkCreateSemaphore(vk_device, &semaphoreInfo, vk_allocation_callbacks, &vk_render_complete_semaphore[i]);
        ASSERT_MESSAGE(result == VK_SUCCESS, "[Vulkan] Error: Failed to create semaphore %d.", i);

        result = vkCreateFence(vk_device, &fenceInfo, vk_allocation_callbacks, &vk_command_buffer_executed_fence[i]);
        ASSERT_MESSAGE(result == VK_SUCCESS, "[Vulkan] Error: Failed to create fence %d.", i);
    }
}

//------------------------------------------------------------------------------
void GPUDevice::DestroySemaphores()
{
    for (uint32 i = 0; i < MAX_SWAPCHAIN_IMAGES; i++)
    {
        vkDestroyFence(vk_device, vk_command_buffer_executed_fence[i], vk_allocation_callbacks);
        vkDestroySemaphore(vk_device, vk_render_complete_semaphore[i], vk_allocation_callbacks);
    }

    vkDestroySemaphore(vk_device, vk_image_acquired_semaphore, vk_allocation_callbacks);
}

//------------------------------------------------------------------------------
void GPUDevice::CreateGPUTimestampManager(uint32 gpu_time_queries_per_frame)
{
    uint8* memory = (uint8*)allocator->allocate(sizeof(GPUTimestampManager) );//+ sizeof(CommandBuffer*) * 128);
    gpu_timestamp_manager = new (memory)GPUTimestampManager(allocator, gpu_time_queries_per_frame, MAX_FRAMES);
}

//------------------------------------------------------------------------------
void GPUDevice::DestroyGPUTimestampManager()
{
    allocator->deallocate(gpu_timestamp_manager, sizeof(GPUTimestampManager));
}

//------------------------------------------------------------------------------
void GPUDevice::CreateCommandBuffers()
{
    uint8* memory = (uint8*)allocator->allocate(sizeof(CommandBufferRing) + sizeof(CommandBuffer*) * 128);
    command_buffer_ring = new (memory)CommandBufferRing(this);

    //uint8* memory = (uint8*)allocator->allocate(sizeof(CommandBuffer*) * 128);
    queued_command_buffers = (CommandBuffer**)(memory + sizeof(CommandBufferRing));
}

//------------------------------------------------------------------------------
void GPUDevice::DestroyCommandBuffers()
{
    allocator->deallocate(command_buffer_ring, sizeof(CommandBufferRing));
    allocator->deallocate(queued_command_buffers, sizeof(CommandBuffer*) * 128);
}

//------------------------------------------------------------------------------
BufferHandle GPUDevice::CreateBuffer(const char* name, ResourceUsageType usage, uint32 size, void* data, VkBufferUsageFlags flags)
{
    BufferHandle handle = buffers.obtainResource();
    if (handle == INVALID_INDEX)
        return handle;

    Buffer* buffer = (Buffer*)buffers.accessResouce(handle);

    buffer->name = name;
    buffer->size = size;
    buffer->flags = flags;
    buffer->usage = usage;
    buffer->handle = handle;
    buffer->global_offset = 0;
    buffer->parent_buffer = InvalidBuffer;

    static const VkBufferUsageFlags DYNAMIC_BUFFER_MASK = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    const bool USE_GLOBAL_BUFFER = (flags & DYNAMIC_BUFFER_MASK) != 0;

    if (usage == ResourceUsageType::Dynamic && USE_GLOBAL_BUFFER)
    {
        buffer->parent_buffer = dynamic_buffer;
        return handle;
    }

    VkBufferCreateInfo buffer_info {};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffer_info.size = (size > 0) ? size : 1;
    
    VmaAllocationCreateInfo alloc_create_info {};
    alloc_create_info.flags = VMA_ALLOCATION_CREATE_STRATEGY_BEST_FIT_BIT;
    alloc_create_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    VmaAllocationInfo alloc_info {};

    VkResult result = vmaCreateBuffer(vma_allocator, &buffer_info, &alloc_create_info, &buffer->vk_buffer, &buffer->vma_allocation, &alloc_info);
    ASSERT_MESSAGE(result == VK_SUCCESS, "[Vulkan] Error: Failed to allocate buffer.");

    SetResourceName(VK_OBJECT_TYPE_BUFFER, (uint64)buffer->vk_buffer, name);

    buffer->vk_device_memory = alloc_info.deviceMemory;

    if (data)
    {
        void* memory;
        vmaMapMemory(vma_allocator, buffer->vma_allocation, &memory);
        memcpy(memory, data, (size_t)size);
        vmaUnmapMemory(vma_allocator, buffer->vma_allocation);
    }

    return handle;
}
//------------------------------------------------------------------------------
TextureHandle GPUDevice::CreateTexture(const char* name, void* data, VkFormat format, Texture::Type type, uint16 width, uint16 height, uint16 depth, uint8 mipmaps, uint8 flags)
{
    return 0;
}
//------------------------------------------------------------------------------
PipelineHandle GPUDevice::CreatePipeline()
{
    return 0;
}
//------------------------------------------------------------------------------
SamplerHandle GPUDevice::CreateSampler(const char* name, VkFilter min_filter, VkFilter mag_filter, VkSamplerMipmapMode mip_filter, VkSamplerAddressMode address_mode_u, VkSamplerAddressMode address_mode_v, VkSamplerAddressMode address_mode_w)
{
    SamplerHandle handle = samplers.obtainResource();
    if (handle == INVALID_INDEX)
        return handle;

    Sampler* sampler = (Sampler*)samplers.accessResouce(handle);

    sampler->name = name;

    sampler->min_filter = min_filter;
    sampler->mag_filter = mag_filter;
    sampler->mip_filter = mip_filter;

    sampler->address_mode_u = address_mode_u;
    sampler->address_mode_v = address_mode_v;
    sampler->address_mode_w = address_mode_w;

    VkSamplerCreateInfo create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    create_info.addressModeU = address_mode_u;
    create_info.addressModeV = address_mode_v;
    create_info.addressModeW = address_mode_w;
    create_info.minFilter = min_filter;
    create_info.magFilter = mag_filter;
    create_info.mipmapMode = mip_filter;
    create_info.anisotropyEnable = 0;
    create_info.compareEnable = 0;
    create_info.unnormalizedCoordinates = 0;
    create_info.borderColor = VkBorderColor::VK_BORDER_COLOR_INT_OPAQUE_WHITE;
    
    vkCreateSampler(vk_device, &create_info, vk_allocation_callbacks, &sampler->vk_sampler);
    SetResourceName(VK_OBJECT_TYPE_SAMPLER, (uint64)sampler->vk_sampler, name);

    return handle;
}

//------------------------------------------------------------------------------
DescriptorSetLayoutHandle GPUDevice::CreateDescriptorSetLayout()
{
    return 0;
}
//------------------------------------------------------------------------------
DescriptorSetHandle GPUDevice::CreateDescriptorSet()
{
    return 0;
}
//------------------------------------------------------------------------------
RenderPassHandle GPUDevice::CreateRenderPass()
{
    return 0;
}
//------------------------------------------------------------------------------
ShaderStateHandle GPUDevice::CreateShaderState()
{
    return 0;
}

//------------------------------------------------------------------------------
VkBool32 GPUDevice::GetFamilyQueue(VkPhysicalDevice pDevice)
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
            vkGetPhysicalDeviceSurfaceSupportKHR(pDevice, iQueueFamily, vk_surface, &surfaceSupported);

            if (surfaceSupported)
            {
                main_queue_family_index = iQueueFamily;
                break;
            }
        }
    }

    return surfaceSupported;
}

//------------------------------------------------------------------------------
void GPUDevice::SetResourceName(VkObjectType type, uint64 handle, const char* name)
{
    if (!(m_uFlags & Flags::DebugUtilsExtensionExist))
        return;

    VkDebugUtilsObjectNameInfoEXT name_info {};
    name_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    name_info.objectType = type;
    name_info.objectHandle = handle;
    name_info.pObjectName = name;
    pfnSetDebugUtilsObjectNameEXT(vk_device, &name_info);
}

} // namespace Graphics
} // namespace Raptor