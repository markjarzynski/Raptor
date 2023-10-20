#pragma once

#include <vulkan/vulkan.h>
#include "Types.h"
#include "Resources.h"

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
    
    CommandBuffer(QueueType type, uint32 bufferSize, uint32 submitSize, Flags flags = Flags::None);
    ~CommandBuffer();

    void CommandBuffer::bindPass(RenderPassHandle handle);

    //void barrier(const ExecutionBarrier& barrier);

    //void FillBuffer(BufferHandle buffer, uint32 offset, uint32 size, uint32 data);

    void PushMarker(const char* name);
    void PopMarker();

    void reset();

private:

    VkCommandBuffer commandBuffer;

    //GpuDevice* device;

    VkDescriptorSet descriptorSets[16];

    //RenderPass* currentRenderPass;
    //Pipeline* currentPipeline;

    VkClearValue clears[2]; // 0 color, 1 depth

    uint32 uFlags = Flags::None;

    uint32 handle;

    uint32 currentCommand;
    //ResourceHandle resourceHandle;
    QueueType type = QueueType::Graphics;
    uint32 bufferSize = 0;
    
}; // class CommandBuffer
} // namespace Graphics
} // namespace Raptor