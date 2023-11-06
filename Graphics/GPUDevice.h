#pragma once

#if (_MSC_VER)
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#define VK_ENABLE_BETA_EXTENSIONS
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <EASTL/allocator.h>
#include <EASTL/vector.h>

#include "Buffer.h"
#include "Debug.h"
#include "DescriptorSet.h"
#include "DescriptorSetLayout.h"
#include "GPUTimestampManager.h"
#include "Log.h"
#include "Pipeline.h"
#include "RenderPass.h"
#include "ResourcePool.h"
#include "Resources.h"
#include "Sampler.h"
#include "ShaderState.h"
#include "Texture.h"
#include "Types.h"
#include "Window.h"




#define VULKAN_DEBUG

namespace Raptor
{
namespace Graphics
{
enum class QueueType
{
    Graphics, Compute, CopyTransfer, Max
};

class CommandBuffer;

class GPUDevice
{
    using Allocator = eastl::allocator;

    template <typename T, typename Allocator>
    using Vector = eastl::vector<T, Allocator>;

public:

    GPUDevice(Window& window, Allocator& allocator, uint32 flags = 0, uint32 gpu_time_queries_per_frame = 32);
    ~GPUDevice();

    GPUDevice(const GPUDevice &) = delete;
    GPUDevice &operator=(const GPUDevice &) = delete;

private:

    void Init(uint32 gpu_time_queries_per_frame);

public:

    void Init(Window& window, Allocator& allocator, uint32 flags = 0, uint32 gpu_time_queries_per_frame = 32);
    void Shutdown();

    // Create Resources
    BufferHandle CreateBuffer(const CreateBufferParams& params);
    TextureHandle CreateTexture(const CreateTextureParams& params);
    PipelineHandle CreatePipeline();
    SamplerHandle CreateSampler(const CreateSamplerParams& params);
    DescriptorSetLayoutHandle CreateDescriptorSetLayout();
    DescriptorSetHandle CreateDescriptorSet();
    RenderPassHandle CreateRenderPass(const CreateRenderPassParams& params);
    ShaderStateHandle CreateShaderState();

    // Destroy Resources
    void DestroyBuffer(BufferHandle handle);
    void DestroyTexture(TextureHandle handle);
    void DestroyPipeline(PipelineHandle handle);
    void DestroySampler(SamplerHandle handle);
    void DestroyDescriptorSetLayout(DescriptorSetLayoutHandle handle);
    void DestroyDescriptorSet(DescriptorSetHandle handle);
    void DestroyRenderPass(RenderPassHandle handle);
    void DestroyShaderState(ShaderStateHandle handle);

    // Query Description
    void QueryBuffer(BufferHandle handle, BufferDescription& out_description);
    void QueryTexture(TextureHandle handle, TextureDescription& out_description);
    void QueryPipeline(PipelineHandle handle, PipelineDescription& out_description);
    void QuerySampler(SamplerHandle handle, SamplerDescription& out_description);
    void QueryDescriptorSetLayout(DescriptorSetLayoutHandle handle, DescriptorSetLayoutDescription& out_description);
    void QueryDescriptorSet(DescriptorSetHandle handle, DesciptorSetDescription& out_description);
    void QueryShaderState(ShaderStateHandle handle, ShaderStateDescription& out_description);

    const RenderPassOutput& GetRenderPassOutput(RenderPassHandle handle) const;

    // Swapchain
    void ResizeSwapchain();

    // Command Buffers
    void QueueCommandBuffer(CommandBuffer* command_buffer);
    CommandBuffer* GetCommandBuffer(QueueType type, bool begin);
    CommandBuffer* GetInstantCommandBuffer();

    // Rendering
    void NewFrame();
    void Present();
    void Resize(uint16 width, uint16 height);

    void FillBarrier(RenderPassHandle render_pass, ExecutionBarrier& out_barrier);

    BufferHandle GetFullscreenVertexBuffer() const;
    RenderPassHandle GetSwapchainPass() const;
    TextureHandle GetDummyTexture() const;
    BufferHandle GetDummyConstantBuffer() const;
    const RenderPassOutput& GetSwapchainOutput() const { return swapchain_output; }
    VkRenderPass GetVkRenderPass(const RenderPassOutput& output, const char* name);

    // Markers
    void SetResourceName(VkObjectType type, uint64 handle, const char* name);
    void PushMarker(VkCommandBuffer command_buffer, const char* name);
    void PopMarker(VkCommandBuffer command_buffer);

    // GPU Timings
    uint32 GetGPUTimestamps(GPUTimestamp* out_timestamps);

    // Destroy Instant
    void DestroyBufferInstant(ResourceHandle buffer);
    void DestroyTextureInstant(ResourceHandle texture);
    void DestroyPipelineInstant(ResourceHandle pipeline);
    void DestroySamplerInstant(ResourceHandle sampler);
    void DestroyDescriptorSetLayoutInstant(ResourceHandle layout);
    void DestroyDescriptorSetInstant(ResourceHandle set);
    void DestroyRenderPassInstant(ResourceHandle render_pass);
    void DestroyShaderStateInstant(ResourceHandle shader);

    // Update Instant
    void UpdateDescriptorSetInstant(DescriptorSetUpdate* update);

    Window* window;
    Allocator* allocator;

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

    void CreateSwapchain();
    void DestroySwapchain();

    void CreateVmaAllocator();
    void DestroyVmaAllocator();

    void CreatePools(uint32 gpu_time_queries_per_frame);
    void DestroyPools();

    void CreateSemaphores();
    void DestroySemaphores();

    void CreateGPUTimestampManager(uint32 gpu_time_queries_per_frame);
    void DestroyGPUTimestampManager();

    void CreateCommandBuffers();
    void DestroyCommandBuffers();

    VkBool32 GetFamilyQueue(VkPhysicalDevice pDevice);
    void GetVulkanBinariesPath(char* path, sizet size = 512);

    void* MapBuffer(const MapBufferParams& params);
    void UnmapBuffer(const MapBufferParams& params);

public:

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
    
    CommandBuffer** queued_command_buffers = nullptr;
    uint32 num_allocated_command_buffers = 0;
    uint32 num_queued_command_buffers = 0;

    ResourcePool buffers;
    ResourcePool textures;
    ResourcePool pipelines;
    ResourcePool samplers;
    ResourcePool descriptor_set_layouts;
    ResourcePool descriptor_sets;
    ResourcePool render_passes;
    ResourcePool command_buffers;
    ResourcePool shaders;

    uint32 image_index;
    uint32 current_frame;
    uint32 previous_frame;
    uint32 absolute_frame;

    Vector<ResourceUpdate, Allocator> resource_deleting_queue;
    Vector<DescriptorSetUpdate, Allocator> descriptor_set_updates;
    
    BufferHandle fullscreen_vertex_buffer;
    RenderPassHandle swapchain_pass;
    SamplerHandle default_sampler;

    TextureHandle dummy_texture;
    BufferHandle dummy_constant_buffer;

    TextureHandle depth_texture;

    BufferHandle dynamic_buffer;
    uint8* dynamic_mapped_memory;
    uint32 dynamic_allocated_size;
    uint32 dynamic_per_frame_size;
    uint32 dynamic_max_per_frame_size;

    RenderPassOutput swapchain_output;

    char vulkan_binaries_path[512];
    
    enum Flags : uint32
    {
        DebugUtilsExtensionExist        = 0x1 << 0,
        EnableGPUTimeQueries            = 0x1 << 1,
        TimestampsEnabled               = 0x1 << 2,
        Resized                         = 0x1 << 3,
        GPUTimestampReset               = 0x1 << 4,
    };
    uint32 m_uFlags = 0u;

}; // class GPUDevice

static void CreateTexture(GPUDevice& gpu_device, const CreateTextureParams& params, TextureHandle handle, Texture* texture);
static void ResizeTexture(GPUDevice& gpu_device, Texture* texture, Texture* delete_texture, uint16 width, uint16 height, uint16 depth);
static void CreateSwapchainPass(GPUDevice& gpu_device, const CreateRenderPassParams params, RenderPass* render_pass);
static RenderPassOutput CreateRenderPassOutput(GPUDevice& gpu_device, const CreateRenderPassParams params);
static VkRenderPass GetRenderPass(GPUDevice& gpu_device, const RenderPassOutput& output, const char* name);
static VkRenderPass CreateRenderPass(GPUDevice& gpu_device, const RenderPassOutput& output, const char* name);
static void FillWriteDescriptorSets(GPUDevice& gpu_device, const DescriptorSetLayout* descriptor_set_layout, VkDescriptorSet vk_descriptor_set, VkWriteDescriptorSet* descriptor_write, VkDescriptorBufferInfo* buffer_info, VkDescriptorImageInfo* image_info, VkSampler vk_default_sampler, uint32& num_resources, const ResourceHandle* resources, const SamplerHandle* samplers, const uint16* bindings);

#ifdef VULKAN_DEBUG
static VkBool32 DebugUtilsCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT types, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data);
#endif

static void TransitionImageLayout(VkCommandBuffer command_buffer, VkImage vk_image, VkFormat vk_format, VkImageLayout vk_image_layout, VkImageLayout vk_image_layout_new, bool isDepth);

} // namespace Graphics
} // namespace Raptor