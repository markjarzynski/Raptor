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
#include <EASTL/hash_map.h>
#include <EASTL/string.h>

#include "Defines.h"
#include "File.h"
#include "GPUDevice.h"
#include "Raptor.h"
#include "Defines.h"
#include "DescriptorSet.h"
#include "DescriptorSetLayout.h"
#include "ShaderState.h"
#include "Pipeline.h"
#include "CommandBuffer.h"
#include "CommandBufferRing.h"
#include "Hash.h"

namespace Raptor
{
namespace Graphics
{

using eastl::clamp;

template<typename Key, typename T>
using HashMap = eastl::hash_map<Key, T>;

template<typename Key, typename T>
using Pair = eastl::pair<Key, T>;

using EA::StdC::Snprintf;

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
static HashMap<uint64, VkRenderPass> render_pass_cache;

//------------------------------------------------------------------------------
GPUDevice::GPUDevice(Window& window, Allocator& allocator, uint32 flags, uint32 gpu_time_queries_per_frame)
    : window(&window), allocator(&allocator), m_uFlags(flags)
{
    resource_deletion_queue.set_allocator(allocator);
    descriptor_set_updates.set_allocator(allocator);
    render_pass_cache.set_allocator(allocator);

    Init(gpu_time_queries_per_frame);
}

//------------------------------------------------------------------------------
GPUDevice::~GPUDevice()
{

}

//------------------------------------------------------------------------------
void GPUDevice::Init(uint32 gpu_time_queries_per_frame)
{
    CreateInstance();
    CreateDebugUtilsMessenger();
    CreateSurface();
    CreatePhysicalDevices();
    SetSurfaceFormat();
    SetPresentMode();
    CreateSwapchain();
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

    CreateSamplerParams sampler_params {};
    sampler_params.name = "Sampler Default";
    sampler_params.min_filter = VK_FILTER_LINEAR;
    sampler_params.mag_filter = VK_FILTER_LINEAR;
    sampler_params.mip_filter = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_params.address_mode_u = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_params.address_mode_v = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_params.address_mode_w = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    default_sampler = CreateSampler(sampler_params);
    
    CreateBufferParams fullscreen_buffer_params {};
    fullscreen_buffer_params.name = "Fullscreen Vertex Buffer";
    fullscreen_buffer_params.flags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    fullscreen_vertex_buffer = CreateBuffer(fullscreen_buffer_params);

    // create depth texture
    CreateTextureParams depth_texture_params {};
    depth_texture_params.vk_format = VK_FORMAT_D32_SFLOAT;
    depth_texture_params.type = TextureType::Enum::Texture2D;
    depth_texture_params.width = window->width;
    depth_texture_params.height = window->height;
    depth_texture_params.name = "Depth Texture";
    depth_texture = CreateTexture(depth_texture_params);

    // cache depth texture format
    swapchain_output.depth(VK_FORMAT_D32_SFLOAT);

    CreateRenderPassParams create_render_pass_params {};
    create_render_pass_params.type = RenderPassType::Swapchain;
    create_render_pass_params.color_operation = RenderPassOperation::Clear;
    create_render_pass_params.depth_operation = RenderPassOperation::Clear;
    create_render_pass_params.stencil_operation = RenderPassOperation::Clear;
    create_render_pass_params.name = "Swapchain";
    swapchain_pass = CreateRenderPass(create_render_pass_params);

    CreateTextureParams dummy_texture_params {};
    dummy_texture_params.vk_format = VK_FORMAT_R8_UINT;
    dummy_texture_params.name = "Dummy Texture";
    dummy_texture = CreateTexture(dummy_texture_params);

    CreateBufferParams dummy_constant_buffer_params {};
    dummy_constant_buffer_params.size = 16;
    dummy_constant_buffer_params.flags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    dummy_constant_buffer_params.name = "Dummy Constant Buffer";
    dummy_constant_buffer = CreateBuffer(dummy_constant_buffer_params);

    GetVulkanBinariesPath(vulkan_binaries_path);

    dynamic_per_frame_size = 1024 * 1024 * 10;

    CreateBufferParams dynamic_buffer_params {};
    dynamic_buffer_params.size = dynamic_per_frame_size * MAX_FRAMES;
    dynamic_buffer_params.flags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    dynamic_buffer_params.name = "Dynamic Persistent Buffer";
    dynamic_buffer = CreateBuffer(dynamic_buffer_params);

    MapBufferParams map_buffer_params {};
    map_buffer_params.buffer = dynamic_buffer;
    dynamic_mapped_memory = (uint8*)MapBuffer(map_buffer_params);
}

//------------------------------------------------------------------------------
void GPUDevice::Init(Window& window, Allocator& allocator, uint32 flags, uint32 gpu_time_queries_per_frame)
{
    this->window = &window;
    this->allocator = &allocator;
    this->m_uFlags = flags;

    resource_deletion_queue.set_allocator(allocator);
    descriptor_set_updates.set_allocator(allocator);
    render_pass_cache.set_allocator(allocator);

    Init(gpu_time_queries_per_frame);
}

//------------------------------------------------------------------------------
void GPUDevice::Shutdown()
{
    vkDeviceWaitIdle(vk_device);
    command_buffer_ring->Shutdown();


    MapBufferParams map_buffer_params {};
    map_buffer_params.buffer = dynamic_buffer;
    UnmapBuffer(map_buffer_params);

    // TODO

    DestroyCommandBuffers();
    DestroyGPUTimestampManager();
    DestroySemaphores();
    DestroyPools();
    DestroyVmaAllocator();
    DestroySwapchain();
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

    swapchain_output.reset();

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
                swapchain_output.color(surfaceImageFormats[iSupportedFormats]);
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
void GPUDevice::CreateSwapchain()
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
void GPUDevice::DestroySwapchain()
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
    allocator->deallocate(command_buffer_ring, sizeof(CommandBufferRing) + sizeof(CommandBuffer*) * 128);
    //allocator->deallocate(queued_command_buffers, sizeof(CommandBuffer*) * 128);
}

//------------------------------------------------------------------------------
BufferHandle GPUDevice::CreateBuffer(const CreateBufferParams& params)
{
    BufferHandle handle = buffers.obtainResource();
    if (handle == InvalidBuffer)
        return handle;

    Buffer* buffer = (Buffer*)buffers.accessResource(handle);

    buffer->name = params.name;
    buffer->size = params.size;
    buffer->flags = params.flags;
    buffer->usage = params.usage;
    buffer->handle = handle;
    buffer->global_offset = 0;
    buffer->parent_buffer = InvalidBuffer;

    static const VkBufferUsageFlags DYNAMIC_BUFFER_MASK = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    const bool USE_GLOBAL_BUFFER = (params.flags & DYNAMIC_BUFFER_MASK) != 0;

    if (params.usage == ResourceUsageType::Dynamic && USE_GLOBAL_BUFFER)
    {
        buffer->parent_buffer = dynamic_buffer;
        return handle;
    }

    VkBufferCreateInfo buffer_info {};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | params.flags;
    buffer_info.size = (params.size > 0) ? params.size : 1;
    
    VmaAllocationCreateInfo alloc_create_info {};
    alloc_create_info.flags = VMA_ALLOCATION_CREATE_STRATEGY_BEST_FIT_BIT;
    alloc_create_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    VmaAllocationInfo alloc_info {};

    VkResult result = vmaCreateBuffer(vma_allocator, &buffer_info, &alloc_create_info, &buffer->vk_buffer, &buffer->vma_allocation, &alloc_info);
    ASSERT_MESSAGE(result == VK_SUCCESS, "[Vulkan] Error: Failed to allocate buffer.");

    SetResourceName(VK_OBJECT_TYPE_BUFFER, (uint64)buffer->vk_buffer, params.name);

    buffer->vk_device_memory = alloc_info.deviceMemory;

    if (params.data)
    {
        void* data;
        vmaMapMemory(vma_allocator, buffer->vma_allocation, &data);
        memcpy(data, params.data, (size_t)params.size);
        vmaUnmapMemory(vma_allocator, buffer->vma_allocation);
    }

    return handle;
}

//------------------------------------------------------------------------------
static void CreateTexture(GPUDevice& gpu_device, const CreateTextureParams& params, TextureHandle handle, Texture* texture)
{
    VkResult result;

    texture->vk_format = params.vk_format;
    texture->type = params.type;
    texture->width = params.width;
    texture->height = params.height;
    texture->depth = params.depth;
    texture->mipmaps = params.mipmaps;
    texture->flags = params.flags;
    texture->sampler = nullptr;
    texture->handle = handle;

    VkImageCreateInfo image_info {};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.format = params.vk_format;
    image_info.flags = 0;
    image_info.imageType = TextureType::ToVkImageType(params.type);
    image_info.extent.width = params.width;
    image_info.extent.height = params.height;
    image_info.extent.depth = params.depth;
    image_info.mipLevels = params.mipmaps;
    image_info.arrayLayers = 1;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;

    image_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image_info.usage |= (params.flags & Texture::Flags::Compute) ? VK_IMAGE_USAGE_STORAGE_BIT : 0;

    if (TextureFormat::HasDepthOrStencil(params.vk_format))
    {
        image_info.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }
    else
    {
        image_info.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        image_info.usage |= (params.flags & Texture::Flags::RenderTarget) ? VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT : 0;
    }

    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VmaAllocationCreateInfo alloc_info {};
    alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    result = vmaCreateImage(gpu_device.vma_allocator, &image_info, &alloc_info, &texture->vk_image, &texture->vma_allocation, nullptr);
    ASSERT_MESSAGE(result == VK_SUCCESS, "[Vulkan] Error: Failed to alloc memory for Texture.");

    gpu_device.SetResourceName(VK_OBJECT_TYPE_IMAGE, (uint64)texture->vk_image, params.name);

    VkImageViewCreateInfo view_info {};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = texture->vk_image;
    view_info.viewType = TextureType::ToVkImageViewType(params.type);
    view_info.format = params.vk_format;

    if (TextureFormat::HasDepthOrStencil(params.vk_format))
    {
        view_info.subresourceRange.aspectMask = (TextureFormat::HasDepth(params.vk_format)) ? VK_IMAGE_ASPECT_DEPTH_BIT : 0;
    }
    else
    {
        view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.layerCount = 1;

    result = vkCreateImageView(gpu_device.vk_device, &view_info, gpu_device.vk_allocation_callbacks, &texture->vk_image_view);
    ASSERT_MESSAGE(result == VK_SUCCESS, "[Vulkan] Error: Failed to create Image View for Texture.");

    gpu_device.SetResourceName(VK_OBJECT_TYPE_IMAGE_VIEW, (uint64)texture->vk_image_view, params.name);

    texture->vk_image_layout = VK_IMAGE_LAYOUT_UNDEFINED;
}

//------------------------------------------------------------------------------
TextureHandle GPUDevice::CreateTexture(const CreateTextureParams& params)
{
    TextureHandle handle = textures.obtainResource();
    if (handle == InvalidTexture)
        return handle;

    Texture* texture = (Texture*)textures.accessResource(handle);

    Raptor::Graphics::CreateTexture(*this, params, handle, texture);


    if (params.data)
    {
        VkBufferCreateInfo buffer_info {};
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        buffer_info.size = params.width * params.height * 4;
        
        VmaAllocationCreateInfo alloc_create_info {};
        alloc_create_info.flags = VMA_ALLOCATION_CREATE_STRATEGY_BEST_FIT_BIT;
        alloc_create_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

        VmaAllocationInfo alloc_info {};
        VkBuffer staging_buffer;
        VmaAllocation staging_alloc;
        VkResult result = vmaCreateBuffer(vma_allocator, &buffer_info, &alloc_create_info, &staging_buffer, &staging_alloc, &alloc_info);
        ASSERT_MESSAGE(result == VK_SUCCESS, "[Vulkan] Error: Failed to create buffer during CreateTexture.");

        // copy buffer data
        void* dst_data;
        vmaMapMemory(vma_allocator, staging_alloc, &dst_data);
        memcpy(dst_data, params.data, static_cast<size_t>(buffer_info.size));
        vmaUnmapMemory(vma_allocator, staging_alloc);

        // execute command buffer
        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        CommandBuffer* command_buffer = GetInstantCommandBuffer();
        vkBeginCommandBuffer(command_buffer->vk_command_buffer, &begin_info);

        VkBufferImageCopy region = {};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {params.width, params.height, params.depth};

        TransitionImageLayout(command_buffer->vk_command_buffer, texture->vk_image, texture->vk_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, false);
        vkCmdCopyBufferToImage(command_buffer->vk_command_buffer, staging_buffer, texture->vk_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        TransitionImageLayout(command_buffer->vk_command_buffer, texture->vk_image, texture->vk_format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, false);

        vkEndCommandBuffer(command_buffer->vk_command_buffer);

        VkSubmitInfo submit_info {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.pCommandBuffers = &command_buffer->vk_command_buffer;

        vkQueueSubmit(vk_queue, 1, &submit_info, VK_NULL_HANDLE);
        vkQueueWaitIdle(vk_queue);

        vmaDestroyBuffer(vma_allocator, staging_buffer, staging_alloc);

        vkResetCommandBuffer(command_buffer->vk_command_buffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

        texture->vk_image_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

    return handle;
}



//------------------------------------------------------------------------------
PipelineHandle GPUDevice::CreatePipeline(const CreatePipelineParams& params)
{
    PipelineHandle handle = pipelines.obtainResource();
    if (handle == InvalidPipeline)
        return handle;

    ShaderStateHandle shader_state_handle = CreateShaderState(params.shaders);
    if (shader_state_handle == InvalidShaderState)
    {
        pipelines.releaseResource(handle);
        handle = InvalidPipeline;
        return handle;
    }

    Pipeline* pipeline = (Pipeline*)pipelines.accessResource(handle);
    ShaderState* shader_state = (ShaderState*)shaders.accessResource(shader_state_handle);

    pipeline->shader_state = shader_state_handle;

    VkDescriptorSetLayout vk_layouts[MAX_DESCRIPTOR_SET_LAYOUTS];

    for (uint32 i = 0; i < params.num_active_layouts; i++)
    {
        pipeline->descriptor_set_layout[i] = (DescriptorSetLayout*)descriptor_set_layouts.accessResource(params.descriptor_set_layout[i]);
        pipeline->descrptor_set_layout_handle[i] = params.descriptor_set_layout[i];
        
        vk_layouts[i] = pipeline->descriptor_set_layout[i]->vk_descriptor_set_layout;
    }

    VkPipelineLayoutCreateInfo pipeline_layout_info {};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.pSetLayouts = vk_layouts;
    pipeline_layout_info.setLayoutCount = params.num_active_layouts;

    VkPipelineLayout pipeline_layout;
    VkResult result = vkCreatePipelineLayout(vk_device, &pipeline_layout_info, vk_allocation_callbacks, &pipeline_layout);
    ASSERT(result == VK_SUCCESS);

    pipeline->vk_pipeline_layout = pipeline_layout;
    pipeline->num_active_layouts = params.num_active_layouts;

    if (shader_state->graphics_pipeline)
    {
        VkGraphicsPipelineCreateInfo pipeline_info {};
        pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipeline_info.pStages = shader_state->shader_stage_info;
        pipeline_info.stageCount = shader_state->active_shaders;
        pipeline_info.layout = pipeline_layout;

        VkPipelineVertexInputStateCreateInfo vertex_input_info {};
        vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        VkVertexInputAttributeDescription vertex_attributes[8];
        if (params.vertex_input.num_vertex_attributes > 0)
        {
            for (uint32 i = 0; i < params.vertex_input.num_vertex_attributes; i++)
            {
                const VertexAttribute& vertex_attribute = params.vertex_input.vertex_attributes[i];
                vertex_attributes[i] = { vertex_attribute.location, vertex_attribute.binding, VertexComponentFormat::ToVkFormat(vertex_attribute.format), vertex_attribute.offset};
            }

            vertex_input_info.vertexAttributeDescriptionCount = params.vertex_input.num_vertex_attributes;
            vertex_input_info.pVertexAttributeDescriptions = vertex_attributes;
        }
        else
        {
            vertex_input_info.vertexAttributeDescriptionCount = 0;
            vertex_input_info.pVertexAttributeDescriptions = nullptr;
        }

        VkVertexInputBindingDescription vertex_bindings[8];
        if (params.vertex_input.num_vertex_streams)
        {
            for (uint32 i = 0; i < params.vertex_input.num_vertex_streams; i++)
            {
                const VertexStream& vertex_stream = params.vertex_input.vertex_streams[i];
                VkVertexInputRate vertex_rate = (vertex_stream.input_rate == VertexInputRate::PerVertex) ? VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX : VkVertexInputRate::VK_VERTEX_INPUT_RATE_INSTANCE;
                vertex_bindings[i] = {vertex_stream.binding, vertex_stream.stride, vertex_rate};
            }

            vertex_input_info.vertexBindingDescriptionCount = params.vertex_input.num_vertex_streams;
            vertex_input_info.pVertexBindingDescriptions = vertex_bindings;
        }
        else
        {
            vertex_input_info.vertexBindingDescriptionCount = 0;
            vertex_input_info.pVertexBindingDescriptions = nullptr;
        }

        pipeline_info.pVertexInputState = &vertex_input_info;

        VkPipelineInputAssemblyStateCreateInfo input_assembly {};
        input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        input_assembly.primitiveRestartEnable = VK_FALSE;

        pipeline_info.pInputAssemblyState = &input_assembly;

        VkPipelineColorBlendAttachmentState color_blend_attachment[8];

        if (params.blend_state.active_states > 0)
        {
            for (uint32 i = 0; i < params.blend_state.active_states; i++)
            {
                const BlendState& blend_state = params.blend_state.blend_states[i];

                color_blend_attachment[i].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
                color_blend_attachment[i].blendEnable = (blend_state.blend_enabled) ? VK_TRUE : VK_FALSE;
                color_blend_attachment[i].srcColorBlendFactor = blend_state.source_color;
                color_blend_attachment[i].dstColorBlendFactor = blend_state.destination_color;
                color_blend_attachment[i].colorBlendOp = blend_state.color_operation;

                if (blend_state.separte_blend)
                {
                    color_blend_attachment[i].srcAlphaBlendFactor = blend_state.source_alpha;
                    color_blend_attachment[i].dstAlphaBlendFactor = blend_state.destination_alpha;
                    color_blend_attachment[i].alphaBlendOp = blend_state.alpha_operation;
                }
                else
                {
                    color_blend_attachment[i].srcAlphaBlendFactor = blend_state.source_color;
                    color_blend_attachment[i].dstAlphaBlendFactor = blend_state.destination_color;
                    color_blend_attachment[i].alphaBlendOp = blend_state.color_operation;
                }
            }
        }
        else
        {
            color_blend_attachment[0] = {};
            color_blend_attachment[0].blendEnable = VK_FALSE;
            color_blend_attachment[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        }

        VkPipelineColorBlendStateCreateInfo color_blending {};
        color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blending.logicOpEnable = VK_FALSE;
        color_blending.logicOp = VK_LOGIC_OP_COPY;
        color_blending.attachmentCount = (params.blend_state.active_states > 0) ? params.blend_state.active_states : 1;
        color_blending.pAttachments = color_blend_attachment;
        color_blending.blendConstants[0] = 0.f;
        color_blending.blendConstants[1] = 0.f;
        color_blending.blendConstants[2] = 0.f;
        color_blending.blendConstants[3] = 0.f;

        pipeline_info.pColorBlendState = &color_blending;

        VkPipelineDepthStencilStateCreateInfo depth_stencil {};
        depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depth_stencil.depthWriteEnable = (params.depth_stencil.depth_write_enable) ? VK_TRUE : VK_FALSE;
        depth_stencil.stencilTestEnable = (params.depth_stencil.stencil_enable) ? VK_TRUE : VK_FALSE;
        depth_stencil.depthTestEnable = (params.depth_stencil.depth_enable) ? VK_TRUE : VK_FALSE;
        depth_stencil.depthCompareOp = params.depth_stencil.depth_comparison;
        if (params.depth_stencil.stencil_enable)
        {
            ASSERT(false);
        }

        pipeline_info.pDepthStencilState = &depth_stencil;

        VkPipelineMultisampleStateCreateInfo multisampling = {};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = 1.f;
        multisampling.pSampleMask = nullptr;
        multisampling.alphaToCoverageEnable = VK_FALSE;
        multisampling.alphaToOneEnable = VK_FALSE;

        pipeline_info.pMultisampleState = &multisampling;

        VkPipelineRasterizationStateCreateInfo rasterizer {};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.f;
        rasterizer.cullMode = params.rasterization.cull_mode;
        rasterizer.frontFace = params.rasterization.front;
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.f;
        rasterizer.depthBiasClamp = 0.f;
        rasterizer.depthBiasSlopeFactor = 0.f;
 
        pipeline_info.pRasterizationState = &rasterizer;

        pipeline_info.pTessellationState;

        VkViewport viewport {};
        viewport.x = 0.f;
        viewport.y = 0.f;
        viewport.width = (float)swapchain_width;
        viewport.height = (float)swapchain_height;
        viewport.minDepth = 0.f;
        viewport.maxDepth = 1.f;

        VkRect2D scissor {};
        scissor.offset = {0, 0};
        scissor.extent = {swapchain_width, swapchain_height};

        VkPipelineViewportStateCreateInfo viewport_state {};
        viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state.viewportCount = 1;
        viewport_state.pViewports = &viewport;
        viewport_state.scissorCount = 1;
        viewport_state.pScissors = &scissor;

        pipeline_info.pViewportState = &viewport_state;

        pipeline_info.renderPass = GetVkRenderPass(params.render_pass, params.name);

        VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        VkPipelineDynamicStateCreateInfo dynamic_state {};
        dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic_state.dynamicStateCount = ARRAY_SIZE(dynamic_states);
        dynamic_state.pDynamicStates = dynamic_states;

        pipeline_info.pDynamicState = &dynamic_state;

        vkCreateGraphicsPipelines(vk_device, VK_NULL_HANDLE, 1, &pipeline_info, vk_allocation_callbacks, &pipeline->vk_pipeline);

        pipeline->vk_pipeline_bind_point = VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS;
    }
    else
    {
        VkComputePipelineCreateInfo pipeline_info {};
        pipeline_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        pipeline_info.stage = shader_state->shader_stage_info[0];
        pipeline_info.layout = pipeline_layout;

        vkCreateComputePipelines(vk_device, VK_NULL_HANDLE, 1, &pipeline_info, vk_allocation_callbacks, &pipeline->vk_pipeline);

        pipeline->vk_pipeline_bind_point = VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_COMPUTE;
    }

    return handle;
}
//------------------------------------------------------------------------------
SamplerHandle GPUDevice::CreateSampler(const CreateSamplerParams& params)
{
    SamplerHandle handle = samplers.obtainResource();
    if (handle == InvalidSampler)
        return handle;

    Sampler* sampler = (Sampler*)samplers.accessResource(handle);

    sampler->name = params.name;

    sampler->min_filter = params.min_filter;
    sampler->mag_filter = params.mag_filter;
    sampler->mip_filter = params.mip_filter;

    sampler->address_mode_u = params.address_mode_u;
    sampler->address_mode_v = params.address_mode_v;
    sampler->address_mode_w = params.address_mode_w;

    VkSamplerCreateInfo create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    create_info.addressModeU = sampler->address_mode_u;
    create_info.addressModeV = sampler->address_mode_v;
    create_info.addressModeW = sampler->address_mode_w;
    create_info.minFilter = sampler->min_filter;
    create_info.magFilter = sampler->mag_filter;
    create_info.mipmapMode = sampler->mip_filter;
    create_info.anisotropyEnable = 0;
    create_info.compareEnable = 0;
    create_info.unnormalizedCoordinates = 0;
    create_info.borderColor = VkBorderColor::VK_BORDER_COLOR_INT_OPAQUE_WHITE;
    
    vkCreateSampler(vk_device, &create_info, vk_allocation_callbacks, &sampler->vk_sampler);
    SetResourceName(VK_OBJECT_TYPE_SAMPLER, (uint64)sampler->vk_sampler, sampler->name);

    return handle;
}

//------------------------------------------------------------------------------
DescriptorSetLayoutHandle GPUDevice::CreateDescriptorSetLayout(const CreateDescriptorSetLayoutParams& params)
{
    DescriptorSetLayoutHandle handle = descriptor_set_layouts.obtainResource();
    if (handle == InvalidDescriptorSetLayout)
        return handle;
    
    DescriptorSetLayout* descriptor_set_layout = (DescriptorSetLayout*)descriptor_set_layouts.accessResource(handle);

    descriptor_set_layout->num_bindings = (uint16) params.num_bindings;
    uint8* memory = (uint8*)allocator->allocate((sizeof(VkDescriptorSetLayoutBinding) + sizeof(DescriptorBinding)) * params.num_bindings);
    descriptor_set_layout->bindings = (DescriptorBinding*)memory;
    descriptor_set_layout->vk_binding = (VkDescriptorSetLayoutBinding*) (memory + sizeof(DescriptorBinding) * params.num_bindings);
    descriptor_set_layout->handle = handle;
    descriptor_set_layout->set_index = (uint16)params.set_index;
    
    uint32 used_bindings =0;
    for (uint32 i = 0; i < params.num_bindings; i++)
    {
        DescriptorBinding& binding = descriptor_set_layout->bindings[i];
        const CreateDescriptorSetLayoutParams::Binding& input_binding = params.bindings[i];
        binding.start = (input_binding.start == UINT16_MAX) ? (uint16)i : input_binding.start;
        binding.count = 1;
        binding.vk_descriptor_type = input_binding.vk_descriptor_type;
        binding.name = input_binding.name;

        VkDescriptorSetLayoutBinding& vk_binding = descriptor_set_layout->vk_binding[used_bindings++];

        vk_binding.binding = binding.start;
        vk_binding.descriptorType = input_binding.vk_descriptor_type;
        vk_binding.descriptorType = (vk_binding.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : vk_binding.descriptorType;
        vk_binding.descriptorCount = 1;

        vk_binding.stageFlags = VK_SHADER_STAGE_ALL;
        vk_binding.pImmutableSamplers = nullptr;
    }

    VkDescriptorSetLayoutCreateInfo layout_info {};
    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = used_bindings;
    layout_info.pBindings = descriptor_set_layout->vk_binding;

    vkCreateDescriptorSetLayout(vk_device, &layout_info, vk_allocation_callbacks, &descriptor_set_layout->vk_descriptor_set_layout);

    return handle;
}

static void FillWriteDescriptorSets(GPUDevice& gpu_device, const DescriptorSetLayout* descriptor_set_layout,
    VkDescriptorSet vk_descriptor_set, VkWriteDescriptorSet* descriptor_write, VkDescriptorBufferInfo* buffer_info,
    VkDescriptorImageInfo* image_info, VkSampler vk_default_sampler, uint32& num_resources,
    const ResourceHandle* resources, const SamplerHandle* samplers, const uint16* bindings)
{
    for (uint32 i = 0; i < num_resources; i++)
    {
        uint32 layout_binding_index = bindings[i];

        const DescriptorBinding& binding = descriptor_set_layout->bindings[layout_binding_index];

        descriptor_write[i] = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        descriptor_write[i].dstSet = vk_descriptor_set;
        const uint32 binding_point = binding.start;
        descriptor_write[i].dstBinding = binding_point;
        descriptor_write[i].dstArrayElement = 0;
        descriptor_write[i].descriptorCount = 1;

        switch (binding.vk_descriptor_type)
        {
            case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            {
                descriptor_write[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

                TextureHandle texture_handle = resources[i];
                Texture* texture_data = (Texture*)gpu_device.textures.accessResource(texture_handle);

                image_info[i].sampler = vk_default_sampler;
                if (texture_data->sampler)
                    image_info[i].sampler = texture_data->sampler->vk_sampler;
                
                if (samplers[i] != InvalidSampler)
                {
                    Sampler* sampler = (Sampler*)gpu_device.samplers.accessResource(samplers[i]);
                }

                image_info[i].imageLayout = TextureFormat::HasDepthOrStencil(texture_data->vk_format) ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                image_info[i].imageView = texture_data->vk_image_view;

                descriptor_write[i].pImageInfo = &image_info[i];

            } break;

            case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
            {
                descriptor_write[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

                TextureHandle texture_handle = resources[i];
                Texture* texture_data = (Texture*)gpu_device.textures.accessResource(texture_handle);

                image_info[i].sampler = nullptr;
                image_info[i].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                image_info[i].imageView = texture_data->vk_image_view;

                descriptor_write[i].pImageInfo = &image_info[i];

            } break;

            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            {
                BufferHandle buffer_handle = resources[i];
                Buffer* buffer = (Buffer*)gpu_device.buffers.accessResource(buffer_handle);

                descriptor_write[i].descriptorType = (buffer->usage == ResourceUsageType::Dynamic) ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

                if (buffer->parent_buffer != InvalidBuffer)
                {
                    Buffer* parent_buffer = (Buffer*)gpu_device.buffers.accessResource(buffer->parent_buffer);

                    buffer_info[i].buffer = parent_buffer->vk_buffer;
                }
                else
                {
                    buffer_info[i].buffer = buffer->vk_buffer;
                }

                buffer_info[i].offset = 0;
                buffer_info[i].range = buffer->size;

                descriptor_write[i].pBufferInfo = &buffer_info[i];

            } break;

            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            {
                BufferHandle buffer_handle = resources[i];
                Buffer* buffer = (Buffer*)gpu_device.buffers.accessResource(buffer_handle);

                descriptor_write[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

                if (buffer->parent_buffer != InvalidBuffer)
                {
                    Buffer* parent_buffer = (Buffer*)gpu_device.buffers.accessResource(buffer->parent_buffer);
                    
                    buffer_info[i].buffer = parent_buffer->vk_buffer;
                }
                else
                {
                    buffer_info[i].buffer = buffer->vk_buffer;
                }

                buffer_info[i].offset = 0;
                buffer_info[i].range = buffer->size;

                descriptor_write[i].pBufferInfo = &buffer_info[i];

            } break;

            default:
            {
                ASSERT_MESSAGE(false, "[Vulkan] Error: Resource type %d not supported in descriptor set creation.\n", binding.vk_descriptor_type);
            }
        }
    }
}

//------------------------------------------------------------------------------
DescriptorSetHandle GPUDevice::CreateDescriptorSet(const CreateDescriptorSetParams& params)
{
    // TODO
    return 0;
}

//------------------------------------------------------------------------------
static void CreateSwapchainPass(GPUDevice& gpu_device, const CreateRenderPassParams params, RenderPass* render_pass)
{
    VkAttachmentDescription color_attachment {};
    color_attachment.format = gpu_device.vk_surface_format.format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_ref {};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depth_attachment {};
    Texture* depth_texture = (Texture*)gpu_device.textures.accessResource(gpu_device.depth_texture);
    depth_attachment.format = depth_texture->vk_format;
    depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_attachment_ref {};
    depth_attachment_ref.attachment = 1;
    depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;
    subpass.pDepthStencilAttachment = &depth_attachment_ref;

    VkAttachmentDescription attachments[] = {color_attachment, depth_attachment};
    VkRenderPassCreateInfo render_pass_info {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = 2;
    render_pass_info.pAttachments = attachments;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;

    VkResult result = vkCreateRenderPass(gpu_device.vk_device, &render_pass_info, nullptr, &render_pass->vk_render_pass);
    ASSERT_MESSAGE(result == VK_SUCCESS, "[Vulkan] Error: CreateRenderPass.");

    gpu_device.SetResourceName(VK_OBJECT_TYPE_RENDER_PASS, (uint64)render_pass->vk_render_pass, params.name);

    VkFramebufferCreateInfo framebuffer_info {};
    framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_info.renderPass = render_pass->vk_render_pass;
    framebuffer_info.attachmentCount = 2;
    framebuffer_info.width = gpu_device.swapchain_width;
    framebuffer_info.height = gpu_device.swapchain_height;
    framebuffer_info.layers = 1;

    VkImageView framebuffer_attachments[2];
    framebuffer_attachments[1] = depth_texture->vk_image_view;

    for (size_t i = 0; i < gpu_device.swapchain_image_count; i++)
    {
        framebuffer_attachments[0] = gpu_device.vk_swapchain_image_views[i];
        framebuffer_info.pAttachments = framebuffer_attachments;

        vkCreateFramebuffer(gpu_device.vk_device, &framebuffer_info, nullptr, &gpu_device.vk_swapchain_framebuffers[i]);
        gpu_device.SetResourceName(VK_OBJECT_TYPE_FRAMEBUFFER, (uint64)gpu_device.vk_swapchain_framebuffers[i], params.name);
    }

    render_pass->width = gpu_device.swapchain_width;
    render_pass->height = gpu_device.swapchain_height;

    VkCommandBufferBeginInfo begin_info {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    CommandBuffer* command_buffer = gpu_device.GetInstantCommandBuffer();
    vkBeginCommandBuffer(command_buffer->vk_command_buffer, &begin_info);

    VkBufferImageCopy region {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {gpu_device.swapchain_width, gpu_device.swapchain_height, 1};

    for (size_t i = 0; i < gpu_device.swapchain_image_count; i++)
    {
        TransitionImageLayout(command_buffer->vk_command_buffer, gpu_device.vk_swapchain_images[i], gpu_device.vk_surface_format.format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, false);
    }

    vkEndCommandBuffer(command_buffer->vk_command_buffer);

    VkSubmitInfo submit_info {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer->vk_command_buffer;

    vkQueueSubmit(gpu_device.vk_queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(gpu_device.vk_queue);
}

static RenderPassOutput CreateRenderPassOutput(GPUDevice& gpu_device, const CreateRenderPassParams params)
{
    RenderPassOutput output;
    output.reset();

    for (uint32 i = 0; i < params.num_render_targets; i++)
    {
        Texture* texture = (Texture*)gpu_device.textures.accessResource(params.output_textures[i]);
        output.color(texture->vk_format);
    }

    if (params.depth_stencil_texture != InvalidTexture)
    {
        Texture* texture = (Texture*)gpu_device.textures.accessResource(params.depth_stencil_texture);
        output.depth(texture->vk_format);   
    }

    output.setOperations(params.color_operation, params.depth_operation, params.stencil_operation);

    return output;
}

VkRenderPass GPUDevice::GetVkRenderPass(const RenderPassOutput& output, const char* name)
{
    uint64 hash = Raptor::Core::HashBytes((void*)&output, sizeof(RenderPassOutput));

    if (render_pass_cache.count(hash) > 0)
        return render_pass_cache.at(hash);
        
    VkRenderPass vk_render_pass = CreateVkRenderPass(*this, output, name);
    Pair<uint64, VkRenderPass> pair(hash, vk_render_pass);
    render_pass_cache.insert(pair);

    return vk_render_pass;
}

//
static VkRenderPass CreateVkRenderPass(GPUDevice& gpu_device, const RenderPassOutput& output, const char* name)
{
    VkAttachmentDescription color_attachments[8] = {};
    VkAttachmentReference color_attachments_ref[8] = {};

    VkAttachmentLoadOp color_op, depth_op, stencil_op;
    VkImageLayout color_initial, depth_initial;

    switch(output.colorOperation)
    {
        case RenderPassOperation::Load:
        {
            color_op = VK_ATTACHMENT_LOAD_OP_LOAD;
            color_initial = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        } break;
        case RenderPassOperation::Clear:
        {
            color_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
            color_initial = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        } break;
        default:
        {
            color_op = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            color_initial = VK_IMAGE_LAYOUT_UNDEFINED;
        }break;
    }

    switch(output.depthOperation)
    {
        case RenderPassOperation::Load:
        {
            depth_op = VK_ATTACHMENT_LOAD_OP_LOAD;
            depth_initial = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        } break;
        case RenderPassOperation::Clear:
        {
            depth_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
            depth_initial = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        } break;
        default:
        {
            depth_op = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            depth_initial = VK_IMAGE_LAYOUT_UNDEFINED;
        } break;
    }

    switch(output.stencilOperation)
    {
        case RenderPassOperation::Load:
        {
            stencil_op = VK_ATTACHMENT_LOAD_OP_LOAD;
        } break;
        case RenderPassOperation::Clear:
        {
            stencil_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
        } break;
        default:
        {
            stencil_op = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        } break;
    }

    for (uint32 i = 0; i < output.numColorFormats; i++)
    {
        VkAttachmentDescription& color_attachment = color_attachments[i];
        color_attachment.format = output.colorFormats[i];
        color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        color_attachment.loadOp = color_op;
        color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color_attachment.stencilLoadOp = stencil_op;
        color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        color_attachment.initialLayout = color_initial;
        color_attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference& color_attachment_ref = color_attachments_ref[i];
        color_attachment_ref.attachment = i;
        color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    VkAttachmentDescription depth_attachment {};
    VkAttachmentReference depth_attachment_ref {};

    if (output.depthSteniclFormat != VK_FORMAT_UNDEFINED)
    {
        depth_attachment.format = output.depthSteniclFormat;
        depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depth_attachment.loadOp = depth_op;
        depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depth_attachment.stencilLoadOp = stencil_op;
        depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depth_attachment.initialLayout = depth_initial;
        depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        depth_attachment_ref.attachment = output.numColorFormats;
        depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    VkSubpassDescription subpass {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    VkAttachmentDescription attachments[MAX_IMAGE_OUTPUTS + 1] {};
    uint32 active_attachments = 0;
    for (; active_attachments < output.numColorFormats; active_attachments += 2)
    {
        attachments[active_attachments] = color_attachments[active_attachments];
    }
    subpass.colorAttachmentCount = (active_attachments) ? active_attachments - 1 : 0;
    subpass.pColorAttachments = color_attachments_ref;
    subpass.pDepthStencilAttachment = nullptr;

    uint32 depth_stencil_count = 0;
    if (output.depthSteniclFormat != VK_FORMAT_UNDEFINED)
    {
        attachments[subpass.colorAttachmentCount] = depth_attachment;
        subpass.pDepthStencilAttachment = &depth_attachment_ref;
        depth_stencil_count = 1;
    }

    VkRenderPassCreateInfo render_pass_info {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = ( active_attachments ? active_attachments - 1 : 0 ) + depth_stencil_count;
    render_pass_info.pAttachments = attachments;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;

    VkRenderPass vk_render_pass;
    VkResult result = vkCreateRenderPass(gpu_device.vk_device, &render_pass_info, nullptr, &vk_render_pass);
    gpu_device.SetResourceName(VK_OBJECT_TYPE_RENDER_PASS, (uint64)vk_render_pass, name);

    return vk_render_pass;
}

static void CreateFramebuffer(GPUDevice& gpu_device, RenderPass* render_pass, const TextureHandle* output_textures, uint32 num_render_targets, TextureHandle depth_stencil_texture)
{
    VkFramebufferCreateInfo framebuffer_info {};
    framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_info.renderPass = render_pass->vk_render_pass;
    framebuffer_info.width = render_pass->width;
    framebuffer_info.height = render_pass->height;
    framebuffer_info.layers = 1;

    VkImageView framebuffer_attachments[MAX_IMAGE_OUTPUTS + 1] {};
    uint32 active_attachments = 0;
    for (; active_attachments < num_render_targets; active_attachments++)
    {
        Texture* texture = (Texture*)gpu_device.textures.accessResource(output_textures[active_attachments]);
        framebuffer_attachments[active_attachments] = texture->vk_image_view;
    }

    if (depth_stencil_texture != InvalidTexture)
    {
        Texture* depth_texture = (Texture*)gpu_device.textures.accessResource(depth_stencil_texture);
        framebuffer_attachments[active_attachments++] = depth_texture->vk_image_view;
    }

    framebuffer_info.pAttachments = framebuffer_attachments;
    framebuffer_info.attachmentCount = active_attachments;

    vkCreateFramebuffer(gpu_device.vk_device, &framebuffer_info, nullptr, &render_pass->vk_framebuffer);
    gpu_device.SetResourceName(VK_OBJECT_TYPE_FRAMEBUFFER, (uint64)render_pass->vk_framebuffer, render_pass->name);
}

//------------------------------------------------------------------------------
RenderPassHandle GPUDevice::CreateRenderPass(const CreateRenderPassParams& params)
{
    RenderPassHandle handle = render_passes.obtainResource();
    if (handle == InvalidRenderPass)
        return handle;

    RenderPass* render_pass = (RenderPass*)render_passes.accessResource(handle);
    render_pass->type = params.type;
    render_pass->dispatchX = 0;
    render_pass->dispatchY = 0;
    render_pass->dispatchZ = 0;
    render_pass->name = params.name;
    render_pass->vk_framebuffer = nullptr;
    render_pass->vk_render_pass = nullptr;
    render_pass->scaleX = params.scale_x;
    render_pass->scaleY = params.scale_y;
    render_pass->resize = params.resize;

    for ( uint32 i = 0; i < params.num_render_targets; ++i )
    {
        Texture* texture = (Texture*)textures.accessResource(params.output_textures[i]);
        render_pass->width = texture->width;
        render_pass->height = texture->height;

        render_pass->outputTextures[i] = params.output_textures[i];
    }

    render_pass->outputDepth = params.depth_stencil_texture;

    switch (params.type)
    {
        case RenderPassType::Swapchain:
        {
            CreateSwapchainPass(*this, params, render_pass);
        } break;
        
        case RenderPassType::Geometry:
        {
            render_pass->output = CreateRenderPassOutput(*this, params);
            render_pass->vk_render_pass = GetVkRenderPass(render_pass->output, params.name);

            CreateFramebuffer(*this, render_pass, params.output_textures, params.num_render_targets, params.depth_stencil_texture);
        } break;

        case RenderPassType::Compute:
        default:
            break;
    }

    return handle;
}

//------------------------------------------------------------------------------
ShaderStateHandle GPUDevice::CreateShaderState( const CreateShaderStateParams& params)
{
    ShaderStateHandle handle = InvalidShaderState;

    if (params.stages_count == 0 || params.stages == nullptr)
    {
        Raptor::Debug::Log("[Vulkan] Warning: Shader %s does not contain shader stages.\n", params.name);
        return handle;
    }

    handle = shaders.obtainResource();
    if (handle == InvalidShaderState)
        return handle;

    uint32 compiled_shaders = 0;

    ShaderState* shader_state = (ShaderState*)shaders.accessResource(handle);
    shader_state->graphics_pipeline = true;
    shader_state->active_shaders = 0;

    for (compiled_shaders = 0; compiled_shaders < params.stages_count; compiled_shaders++)
    {
        const ShaderStage& stage = params.stages[compiled_shaders];

        if (stage.type == VK_SHADER_STAGE_COMPUTE_BIT)
        {
            shader_state->graphics_pipeline = false;
        }

        VkShaderModuleCreateInfo shader_create_info {};
        shader_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

        if (params.spv_input)
        {
            shader_create_info.codeSize = stage.code_size;
            shader_create_info.pCode = reinterpret_cast<const uint32*>(stage.code);
        }
        else
        {
            shader_create_info = CompileShader(stage.code, stage.code_size, stage.type, params.name);
        }

        VkPipelineShaderStageCreateInfo& shader_stage_info = shader_state->shader_stage_info[compiled_shaders];
        memset(&shader_stage_info, 0, sizeof(VkPipelineShaderStageCreateInfo));
        shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shader_stage_info.pName = "main";
        shader_stage_info.stage = stage.type;

        if (vkCreateShaderModule(vk_device, &shader_create_info, nullptr, &shader_state->shader_stage_info[compiled_shaders].module) != VK_SUCCESS)
            break;
        
        SetResourceName(VK_OBJECT_TYPE_SHADER_MODULE, (uint64)shader_state->shader_stage_info[compiled_shaders].module, params.name);
    }

    if (compiled_shaders != params.stages_count) // creation failed
    {
        DestroyShaderState(handle);
        handle = InvalidShaderState;

        Raptor::Debug::Log("Error creating shader %s.\n", params.name);
        for (compiled_shaders = 0; compiled_shaders < params.stages_count; compiled_shaders++)
        {
            const ShaderStage& stage = params.stages[compiled_shaders];
            Raptor::Debug::Log("%u:\n%s\n", stage.type, stage.code);
        }

        return handle;
    }

    shader_state->active_shaders = compiled_shaders;
    shader_state->name = params.name;

    return handle;
}

//------------------------------------------------------------------------------
void GPUDevice::DestroyBuffer(BufferHandle handle)
{
    if (handle < buffers.poolSize)
        resource_deletion_queue.push_back({ResourceDeletionType::Buffer, handle, current_frame});
    else
        Raptor::Debug::Log("[Vulkan] Error: Trying to free invalid Buffer %u\n", handle);
}

//------------------------------------------------------------------------------
void GPUDevice::DestroyTexture(TextureHandle handle)
{
    if (handle < textures.poolSize)
        resource_deletion_queue.push_back({ResourceDeletionType::Texture, handle, current_frame});
    else
        Raptor::Debug::Log("[Vulkan] Error: Trying to free invalid Texture %u\n", handle);
}

//------------------------------------------------------------------------------
void GPUDevice::DestroyPipeline(PipelineHandle handle)
{
    if (handle < pipelines.poolSize)
    {
        resource_deletion_queue.push_back({ResourceDeletionType::Pipeline, handle, current_frame});
        Pipeline* pipeline = (Pipeline*)pipelines.accessResource(handle);
        DestroyShaderState(pipeline->shader_state);
    }
    else
    {
        Raptor::Debug::Log("[Vulkan] Error: Trying to free invalid Pipeline %u\n", handle);
    }
}

//------------------------------------------------------------------------------
void GPUDevice::DestroySampler(SamplerHandle handle)
{
    if (handle < samplers.poolSize)
        resource_deletion_queue.push_back({ResourceDeletionType::Sampler, handle, current_frame});
    else
        Raptor::Debug::Log("[Vulkan] Error: Trying to free invalid Sampler %u\n", handle);
}

//------------------------------------------------------------------------------
void GPUDevice::DestroyDescriptorSetLayout(DescriptorSetLayoutHandle handle)
{
    if (handle < descriptor_set_layouts.poolSize)
        resource_deletion_queue.push_back({ResourceDeletionType::DescriptorSetLayout, handle, current_frame});
    else
        Raptor::Debug::Log("[Vulkan] Error: Trying to free invalid DescriptorSetLayout %u\n", handle);
}

//------------------------------------------------------------------------------
void GPUDevice::DestroyDescriptorSet(DescriptorSetHandle handle)
{
    if (handle < descriptor_sets.poolSize)
        resource_deletion_queue.push_back({ResourceDeletionType::DescriptorSet, handle, current_frame});
    else
        Raptor::Debug::Log("[Vulkan] Error: Trying to free invalid DescriptorSet %u\n", handle);
}

//------------------------------------------------------------------------------
void GPUDevice::DestroyRenderPass(RenderPassHandle handle)
{
    if (handle < render_passes.poolSize)
        resource_deletion_queue.push_back({ResourceDeletionType::RenderPass, handle, current_frame});
    else
        Raptor::Debug::Log("[Vulkan] Error: Trying to free invalid RenderPass %u\n", handle);
}

//------------------------------------------------------------------------------
void GPUDevice::DestroyShaderState(ShaderStateHandle handle)
{
    if (handle < shaders.poolSize)
        resource_deletion_queue.push_back({ResourceDeletionType::ShaderState, handle, current_frame});
    else
        Raptor::Debug::Log("[Vulkan] Error: Trying to free invalid ShaderState %u\n", handle);
}

//------------------------------------------------------------------------------
void GPUDevice::QueryBuffer(BufferHandle handle, BufferDescription& out_description){}
//------------------------------------------------------------------------------
void GPUDevice::QueryTexture(TextureHandle handle, TextureDescription& out_description)
{
    if (handle == InvalidTexture)
        return;

    const Texture* texture = (Texture*)textures.accessResource(handle);
    out_description.width = texture->width;
    out_description.height = texture->height;
    out_description.depth = texture->depth;
    out_description.mipmaps = texture->mipmaps;
    out_description.vk_format = texture->vk_format;
    out_description.type = texture->type;
    out_description.render_target = (texture->flags & Texture::Flags::RenderTarget) == Texture::Flags::RenderTarget;
    out_description.compute_access = (texture->flags & Texture::Flags::Compute) == Texture::Flags::Compute;
    out_description.native_handle = (void*)&texture->vk_image;
    out_description.name = texture->name;
}
//------------------------------------------------------------------------------
void GPUDevice::QueryPipeline(PipelineHandle handle, PipelineDescription& out_description){}

//------------------------------------------------------------------------------
void GPUDevice::QuerySampler(SamplerHandle handle, SamplerDescription& out_description)
{
    if (handle == InvalidSampler)
        return;

    const Sampler* sampler = (Sampler*)samplers.accessResource(handle);
    out_description.address_mode_u = sampler->address_mode_u;
    out_description.address_mode_v = sampler->address_mode_v;
    out_description.address_mode_w = sampler->address_mode_w;
    out_description.min_filter = sampler->min_filter;
    out_description.mag_filter = sampler->mag_filter;
    out_description.mip_filter = sampler->mip_filter;
    out_description.name = sampler->name;
}

//------------------------------------------------------------------------------
void GPUDevice::QueryDescriptorSetLayout(DescriptorSetLayoutHandle handle, DescriptorSetLayoutDescription& out_description){}
//------------------------------------------------------------------------------
void GPUDevice::QueryDescriptorSet(DescriptorSetHandle handle, DesciptorSetDescription& out_description){}
//------------------------------------------------------------------------------
void GPUDevice::QueryShaderState(ShaderStateHandle handle, ShaderStateDescription& out_description){}
//------------------------------------------------------------------------------
const RenderPassOutput& GPUDevice::GetRenderPassOutput(RenderPassHandle handle) const
{
    const RenderPass* render_pass = (RenderPass*)render_passes.accessResource(handle);
    return render_pass->output;
}

//------------------------------------------------------------------------------
static void ResizeTexture(GPUDevice& gpu_device, Texture* texture, Texture* delete_texture, uint16 width, uint16 height, uint16 depth)
{
    delete_texture->vk_image_view = texture->vk_image_view;
    delete_texture->vk_image = texture->vk_image;
    delete_texture->vma_allocation = texture->vma_allocation;

    CreateTextureParams params;
    params.mipmaps = texture->mipmaps;
    params.flags = texture->flags;
    params.vk_format = texture->vk_format;
    params.type = texture->type;
    params.name = texture->name;
    params.width = texture->width;
    params.height = texture->height;
    params.depth = texture->depth;

    CreateTexture(gpu_device, params, texture->handle, texture);
}

//------------------------------------------------------------------------------
void GPUDevice::ResizeSwapchain()
{
    vkDeviceWaitIdle(vk_device);

    VkSurfaceCapabilitiesKHR surface_capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk_physical_device, vk_surface, &surface_capabilities);
    VkExtent2D swapchain_extent = surface_capabilities.currentExtent;

    if (swapchain_extent.width == 0 || swapchain_extent.height == 0)
    {
        return;
    }

    RenderPass* vk_swapchain_pass = (RenderPass*)render_passes.accessResource(swapchain_pass);
    vkDestroyRenderPass(vk_device, vk_swapchain_pass->vk_render_pass, vk_allocation_callbacks);

    DestroySwapchain();
    DestroySurface();

    CreateSurface();
    CreateSwapchain();

    TextureHandle delete_texture = textures.obtainResource();
    Texture* vk_delete_texture = (Texture*)textures.accessResource(delete_texture);
    vk_delete_texture->handle = delete_texture;
    Texture* vk_depth_texture = (Texture*)textures.accessResource(depth_texture);
    ResizeTexture(*this, vk_depth_texture, vk_delete_texture, swapchain_width, swapchain_height, 1);

    DestroyTexture(delete_texture);

    CreateRenderPassParams swapchain_pass_params {};
    swapchain_pass_params.type = RenderPassType::Swapchain;
    swapchain_pass_params.name = "Swapchain";
    CreateSwapchainPass(*this, swapchain_pass_params, vk_swapchain_pass);

    vkDeviceWaitIdle(vk_device);
}

//------------------------------------------------------------------------------
void* GPUDevice::MapBuffer(const MapBufferParams& params)
{
    if (params.buffer == InvalidBuffer)
        return nullptr;

    Buffer* buffer = (Buffer*)buffers.accessResource(params.buffer);

    if (buffer->parent_buffer == dynamic_buffer)
    {
        buffer->global_offset = dynamic_allocated_size;
        return nullptr; // TODO
    }

    void* data;
    vmaMapMemory(vma_allocator, buffer->vma_allocation, &data);
    return data;
}

void GPUDevice::UnmapBuffer(const MapBufferParams& params)
{
    if (params.buffer == InvalidBuffer)
        return;
    
    Buffer* buffer = (Buffer*)buffers.accessResource(params.buffer);
    if (buffer->parent_buffer == dynamic_buffer)
        return;

    vmaUnmapMemory(vma_allocator, buffer->vma_allocation);
}

//------------------------------------------------------------------------------
void GPUDevice::FrameCountersAdvance()
{
    previous_frame = current_frame;
    current_frame = (current_frame + 1) % swapchain_image_count;

    ++absolute_frame;
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



//------------------------------------------------------------------------------
void GPUDevice::QueueCommandBuffer(CommandBuffer* command_buffer)
{
    queued_command_buffers[num_queued_command_buffers++] = command_buffer;
}

//------------------------------------------------------------------------------
CommandBuffer* GPUDevice::GetCommandBuffer(QueueType type, bool begin)
{
    CommandBuffer* command_buffer = command_buffer_ring->GetCommandBuffer(current_frame, begin);

    if ((m_uFlags & Flags::GPUTimestampReset) && begin)
    {
        vkCmdResetQueryPool(command_buffer->vk_command_buffer, vk_query_pool, current_frame * gpu_timestamp_manager->queriesPerFrame * 2, gpu_timestamp_manager->queriesPerFrame);
        m_uFlags &= ~Flags::GPUTimestampReset;
    }

    return command_buffer;
}

//------------------------------------------------------------------------------
CommandBuffer* GPUDevice::GetInstantCommandBuffer()
{
    CommandBuffer* command_buffer = command_buffer_ring->GetCommandBufferInstant(current_frame, false);
    return command_buffer;
}

//------------------------------------------------------------------------------
void GPUDevice::NewFrame()
{
    VkFence* render_complete_fence = &vk_command_buffer_executed_fence[current_frame];

    if (vkGetFenceStatus(vk_device, *render_complete_fence) != VK_SUCCESS)
    {
        vkWaitForFences(vk_device, 1, render_complete_fence, VK_TRUE, UINT64_MAX);
    }

    vkResetFences(vk_device, 1, render_complete_fence);

    VkResult result = vkAcquireNextImageKHR(vk_device, vk_swapchain, UINT64_MAX, vk_image_acquired_semaphore, VK_NULL_HANDLE, &image_index );
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
        ResizeSwapchain();
    
    command_buffer_ring->ResetPools(current_frame);
    const uint32 used_size = dynamic_allocated_size - (dynamic_per_frame_size * previous_frame);
    dynamic_max_per_frame_size = MAX(used_size, dynamic_max_per_frame_size);
    dynamic_allocated_size = dynamic_per_frame_size * current_frame;

    if (descriptor_set_updates.size())
    {
        for (auto it = descriptor_set_updates.begin(); it != descriptor_set_updates.end(); it++)
        {
            UpdateDescriptorSetInstant(it);
            it->frame_issued = UINT32_MAX;
            descriptor_set_updates.erase(it);
        }
    }

}

void GPUDevice::Present()
{
    VkFence* render_complete_fence = &vk_command_buffer_executed_fence[current_frame];
    VkSemaphore* render_complete_semaphore = &vk_render_complete_semaphore[current_frame];

    VkCommandBuffer enqueued_command_buffers[4];
    for (uint32 i = 0; i < num_queued_command_buffers; i++)
    {
        CommandBuffer* command_buffer = queued_command_buffers[i];

        enqueued_command_buffers[i] = command_buffer->vk_command_buffer;

        if ((command_buffer->m_uFlags & CommandBufferFlags::isRecording) && command_buffer->current_render_pass && (command_buffer->current_render_pass->type != RenderPassType::Compute))
            vkCmdEndRenderPass(command_buffer->vk_command_buffer);
        
        vkEndCommandBuffer(command_buffer->vk_command_buffer);
    }

    VkSemaphore wait_semaphores[] = {vk_image_acquired_semaphore};
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    VkSubmitInfo submit_info {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.commandBufferCount = num_queued_command_buffers;
    submit_info.pCommandBuffers = enqueued_command_buffers;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = render_complete_semaphore;

    vkQueueSubmit(vk_queue, 1, &submit_info, *render_complete_fence);

    VkPresentInfoKHR present_info {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = render_complete_semaphore;

    VkSwapchainKHR swapchains[] = {vk_swapchain};
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swapchains;
    present_info.pImageIndices = &image_index;
    present_info.pResults = nullptr;
    VkResult result = vkQueuePresentKHR(vk_queue, &present_info);

    num_queued_command_buffers = 0;

    // TODO Gputimestamp

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || (m_uFlags & Flags::Resized))
    {
        m_uFlags &= ~Flags::Resized;
        ResizeSwapchain();

        FrameCountersAdvance();

        return;
    }

    FrameCountersAdvance();

    if (resource_deletion_queue.size() > 0)
    {
        // TODO
    }
}

void GPUDevice::Resize(uint16 width, uint16 height)
{
    swapchain_width = width;
    swapchain_height = height;

    m_uFlags |= Flags::Resized;
}

//------------------------------------------------------------------------------
uint32 GPUDevice::GetGPUTimestamps(GPUTimestamp* out_timestamps)
{
    return gpu_timestamp_manager->resolve(previous_frame, out_timestamps);
}

//------------------------------------------------------------------------------
void GPUDevice::DestroyBufferInstant(ResourceHandle buffer){}
//------------------------------------------------------------------------------
void GPUDevice::DestroyTextureInstant(ResourceHandle texture){}
//------------------------------------------------------------------------------
void GPUDevice::DestroyPipelineInstant(ResourceHandle pipeline){}
//------------------------------------------------------------------------------
void GPUDevice::DestroySamplerInstant(ResourceHandle sampler){}
//------------------------------------------------------------------------------
void GPUDevice::DestroyDescriptorSetLayoutInstant(ResourceHandle layout){}
//------------------------------------------------------------------------------
void GPUDevice::DestroyDescriptorSetInstant(ResourceHandle set){}
//------------------------------------------------------------------------------
void GPUDevice::DestroyRenderPassInstant(ResourceHandle render_pass){}
//------------------------------------------------------------------------------
void GPUDevice::DestroyShaderStateInstant(ResourceHandle shader){}

//------------------------------------------------------------------------------
void GPUDevice::UpdateDescriptorSetInstant(DescriptorSetUpdate* update)
{
    DescriptorSetHandle dummy_delete_descriptor_set_handle = descriptor_sets.obtainResource();
    DescriptorSet* dummy_delete_descriptor_set = (DescriptorSet*)descriptor_sets.accessResource(dummy_delete_descriptor_set_handle);

    DescriptorSet* descriptor_set = (DescriptorSet*)descriptor_sets.accessResource(update->handle);
    const DescriptorSetLayout* descriptor_set_layout = descriptor_set->layout;

    dummy_delete_descriptor_set->vk_descriptor_set = descriptor_set->vk_descriptor_set;
    dummy_delete_descriptor_set->bindings = nullptr;
    dummy_delete_descriptor_set->resources = nullptr;
    dummy_delete_descriptor_set->samplers = nullptr;
    dummy_delete_descriptor_set->num_resources = 0;

    DestroyDescriptorSet(dummy_delete_descriptor_set_handle);

    VkWriteDescriptorSet descriptor_write[8];
    VkDescriptorBufferInfo buffer_info[8];
    VkDescriptorImageInfo image_info[8];

    Sampler* sampler = (Sampler*)samplers.accessResource(default_sampler);

    VkDescriptorSetAllocateInfo alloc_info {};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = vk_descriptor_pool;
    alloc_info.descriptorSetCount = 1;
    alloc_info.pSetLayouts = &descriptor_set->layout->vk_descriptor_set_layout;
    vkAllocateDescriptorSets(vk_device, &alloc_info, &descriptor_set->vk_descriptor_set);

    uint32 num_resources = descriptor_set_layout->num_bindings;
    FillWriteDescriptorSets(*this, descriptor_set_layout, descriptor_set->vk_descriptor_set, descriptor_write, buffer_info, image_info, sampler->vk_sampler, num_resources, descriptor_set->resources, descriptor_set->samplers, descriptor_set->bindings);
    
    vkUpdateDescriptorSets(vk_device, num_resources, descriptor_write, 0, nullptr);
}







//------------------------------------------------------------------------------
static void TransitionImageLayout(VkCommandBuffer command_buffer, VkImage vk_image, VkFormat vk_format, VkImageLayout old_layout, VkImageLayout new_layout, bool isDepth)
{
    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = vk_image;
    barrier.subresourceRange.aspectMask = (isDepth) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkPipelineStageFlags dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }

    vkCmdPipelineBarrier(command_buffer, src_stage, dst_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void GPUDevice::GetVulkanBinariesPath(char* path, sizet size)
{
#if defined(_MSC_VER)
    char vulkan_env[512];
    ExpandEnvironmentStringsA("%VULKAN_SDK%", vulkan_env, 512);
    Snprintf(path, size, "%s\\Bin\\", vulkan_env);
#else
    char* vulkan_env = getenv("VULKAN_SDK");
    Snprintf(path, size, "%s/bin/", vulkan_env);
#endif
}

VkShaderModuleCreateInfo GPUDevice::CompileShader(const char* code, uint32 code_size, VkShaderStageFlagBits stage, const char* name)
{
    VkShaderModuleCreateInfo shader_create_info {};
    shader_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

    const char* temp_filename = "temp.shader";

    FILE* temp_shader_file = fopen(temp_filename, "w");
    fwrite(code, code_size, 1, temp_shader_file);
    fclose(temp_shader_file);


    eastl::string stage_define;//(allocator);
    stage_define.append_sprintf("%s_%s", ToStageDefines(stage), name);
    stage_define.make_upper();

    eastl::string glsl_compiler_path;//(allocator);
    eastl::string final_spirv_filename = "shader_final.spv";

#if defined(_MSC_VER)
    glsl_compiler_path.append_sprintf("%sglslangValidator.exe", vulkan_binaries_path);

    eastl::string arguments;
    arguments.append_sprintf("glslangValidator.exe %s -V --target-env vulkan1.2 -o %s -S %s --D %s --D %s", temp_filename, final_spirv_filename, ToCompilerExtension(stage), stage_define, ToStageDefines(stage));
#else
    glsl_compiler_path.append_sprintf("%sglslangValidator", vulkan_binaries_path);

    eastl::string arguments;
    arguments.append_sprintf("%s -V --target-env vulkan1.2 -o %s -S %s --D %s --D %s", temp_filename, final_spirv_filename, ToCompilerExtension(stage), stage_define, ToStageDefines(stage));
#endif

    Raptor::Core::ProcessExecute(".", glsl_compiler_path.c_str(), arguments.c_str());

    bool optimize_shaders = false;
    if (optimize_shaders)
    {
        // TODO
    }
    else
    {
        shader_create_info.pCode = reinterpret_cast<const uint32*>(Raptor::Core::FileReadBinary(final_spirv_filename.c_str(), allocator, &shader_create_info.codeSize));
    }

    if (shader_create_info.pCode == nullptr)
    {
        // TODO
    }

    Raptor::Core::FileDelete(temp_filename);
    Raptor::Core::FileDelete(final_spirv_filename.c_str());

    return shader_create_info;
}

} // namespace Graphics
} // namespace Raptor