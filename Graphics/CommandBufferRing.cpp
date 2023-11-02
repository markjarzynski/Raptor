#include "CommandBufferRing.h"
#include "Debug.h"

namespace Raptor
{
namespace Graphics
{

CommandBufferRing::CommandBufferRing(GPUDevice* gpuDevice)
    : gpu_device(gpuDevice)
{
    Init();
}

CommandBufferRing::~CommandBufferRing()
{

}

void CommandBufferRing::Init()
{
    VkResult result;

    for (uint32 i = 0; i < MAX_POOLS; i++)
    {
        VkCommandPoolCreateInfo pool_create_info {};
        pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        //pool_create_info.pNext = nullptr;
        pool_create_info.queueFamilyIndex = gpu_device->main_queue_family_index;
        pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        result = vkCreateCommandPool(gpu_device->vk_device, &pool_create_info, gpu_device->vk_allocation_callbacks, &vk_command_pools[i]);
        ASSERT_MESSAGE(result == VK_SUCCESS, "[Vulkan]: Error: Failed to create command pool %d.", i);
    }

    for (uint32 i = 0; i < MAX_BUFFERS; i++)
    {
        VkCommandBufferAllocateInfo alloc_info {};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        const uint32 pool_index = PoolFromIndex(i);
        alloc_info.commandPool = vk_command_pools[pool_index];
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandBufferCount = 1;
        
        result = vkAllocateCommandBuffers(gpu_device->vk_device, &alloc_info, &command_buffers[i].vk_command_buffer);
        ASSERT_MESSAGE(result == VK_SUCCESS, "[Vulkan]: Error: Failed to allocate command buffer %d.", i);

        command_buffers[i].gpu_device = gpu_device;
        command_buffers[i].handle = i;
        command_buffers[i].Reset();
    }
}

void CommandBufferRing::Init(GPUDevice* gpuDevice)
{
    this->gpu_device = gpuDevice;

    Init();
}

void CommandBufferRing::Shutdown()
{
    for (uint32 i = 0; i < MAX_SWAPCHAIN_IMAGES * MAX_THREADS; i++)
    {
        vkDestroyCommandPool(gpu_device->vk_device, vk_command_pools[i], gpu_device->vk_allocation_callbacks);
    }
}

void CommandBufferRing::ResetPools(uint32 frame_index)
{
    for (uint32 i = 0; i < MAX_THREADS; i++)
    {
        vkResetCommandPool(gpu_device->vk_device, vk_command_pools[frame_index * MAX_THREADS + i], 0);
    }
}

CommandBuffer* CommandBufferRing::GetCommandBuffer(uint32 frame, bool begin)
{
    CommandBuffer* cb = &command_buffers[frame * BUFFER_PER_POOL];

    if (begin)
    {
        cb->Reset();

        VkCommandBufferBeginInfo begin_info;
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        VkResult result = vkBeginCommandBuffer(cb->vk_command_buffer, &begin_info);
        ASSERT_MESSAGE(result == VK_SUCCESS, "[Vulkan]: Error: Failed to begin command buffer for frame %d.", frame);
    }

    return cb;
}

CommandBuffer* CommandBufferRing::GetCommandBufferInstant(uint32 frame, bool begin)
{
    CommandBuffer* cb = &command_buffers[frame * BUFFER_PER_POOL + 1];
    return cb;
}

} // namespace Graphics
} // namespace Raptor