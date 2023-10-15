#pragma once

#include "Type.h"

namespace Raptor
{
namespace Graphics
{
class CommandBuffer
{
public:

    enum QueueType
    {
        Graphics, Compute, CopyTransfer, Max
    };
    
    CommandBuffer(QueueType type, uint32 bufferSize, uint32 submitSize, Flags flags);
    ~CommandBuffer();

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

    enum Flags
    {
        None        = 0x0,
        isRecording = 0x1,
        isBaked     = 0x1 << 1,
    };
    Flags uFlags = Flags::None;

    uint32 handle;

    uint32 currentCommand;
    //ResourceHandle resourceHandle;
    QueueType type = QueueType::Graphics;
    uint32 bufferSize = 0;
    
}; // class CommandBuffer
} // namespace Graphics
} // namespace Raptor