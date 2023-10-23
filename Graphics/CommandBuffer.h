#pragma once

#include <vulkan/vulkan.h>
#include "Types.h"
#include "Resources.h"
#include "GPUDevice.h"

namespace Raptor
{
namespace Graphics
{
class CommandBuffer
{
public:
    enum Flags : uint32
    {
        None        = 0x0,
        isRecording = 0x1,
        isBaked     = 0x1 << 1,
    };

    enum QueueType
    {
        Graphics, Compute, CopyTransfer, Max
    };
    
    CommandBuffer() {}
    CommandBuffer(QueueType type, uint32 bufferSize, uint32 submitSize, Flags m_uFlags = Flags::None);
    ~CommandBuffer();

    void CommandBuffer::bindPass(RenderPassHandle handle);

    //void barrier(const ExecutionBarrier& barrier);

    //void FillBuffer(BufferHandle buffer, uint32 offset, uint32 size, uint32 data);

    void PushMarker(const char* name);
    void PopMarker();

    void reset();

    VkCommandBuffer vk_command_buffer;

    GPUDevice* gpu_device;

    VkDescriptorSet vk_descriptor_sets[16];

    //RenderPass* currentRenderPass;
    //Pipeline* currentPipeline;

    VkClearValue vk_clears[2]; // 0 color, 1 depth

    uint32 m_uFlags = Flags::None;

    uint32 handle;

    uint32 current_command;
    //ResourceHandle resourceHandle;
    QueueType queue_type = QueueType::Graphics;
    uint32 buffer_size = 0;
    
}; // class CommandBuffer
} // namespace Graphics
} // namespace Raptor