#pragma once

#include <vulkan/vulkan.h>

#include "CommandBuffer.h"
#include "GPUDevice.h"
#include "Resources.h"
#include "Types.h"

namespace Raptor
{
namespace Graphics
{
class CommandBufferRing
{

public:
    CommandBufferRing(GPUDevice* gpu_device);
    ~CommandBufferRing();

private:
    void Init();

public:

    void Init(GPUDevice* gpu_device);
    void Shutdown();

    void ResetPools(uint32 frame_index);
    
    CommandBuffer* GetCommandBuffer(uint32 frame, bool begin);
    CommandBuffer* GetCommandBufferInstant(uint32 frame, bool begin);

    static uint16 PoolFromIndex(uint32 index) { return (uint16)index / BUFFER_PER_POOL; }

    static const uint16 MAX_THREADS = 1;
    static const uint16 MAX_POOLS = MAX_SWAPCHAIN_IMAGES * MAX_THREADS;
    static const uint16 BUFFER_PER_POOL = 4;
    static const uint16 MAX_BUFFERS = BUFFER_PER_POOL * MAX_POOLS;

    GPUDevice* gpu_device;
    VkCommandPool vk_command_pools[MAX_POOLS];
    CommandBuffer command_buffers[MAX_BUFFERS];
    uint8 next_free_per_thread_frame[MAX_POOLS];

}; // class CommandBufferRing
} // namespace Graphics
} // namespace Raptor