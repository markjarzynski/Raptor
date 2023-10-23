#pragma once

#include "GPUDevice.h"
#include "CommandBuffer.h"
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

    void ResetPools(uint32 frame_index);
    
    CommandBuffer* GetCommandBuffer(uint32 frame, bool begin);
    CommandBuffer* GetCommandBufferInstant(uint32 frame, bool begin);

    static uint32 PoolFromIndex(uint32 index) { return index / BUFFER_PER_POOL; }

private:

    static const uint32 MAX_THREADS = 1;
    static const uint32 MAX_POOLS = MAX_SWAPCHAIN_IMAGES * MAX_THREADS;
    static const uint32 BUFFER_PER_POOL = 4;
    static const uint32 MAX_BUFFERS = BUFFER_PER_POOL * MAX_POOLS;

    GPUDevice* gpu_device;
    VkCommandPool vk_command_pools[MAX_POOLS];
    CommandBuffer command_buffers[MAX_BUFFERS];
    uint8 next_free_per_thread_frame[MAX_POOLS];

}; // class CommandBufferRing
} // namespace Graphics
} // namespace Raptor