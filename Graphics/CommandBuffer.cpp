#include "CommandBuffer.h"

namespace Raptor
{
namespace Graphics
{

CommandBuffer::CommandBuffer(QueueType type, uint32 bufferSize, uint32 submitSize, Flags flags)
    : queue_type(type), buffer_size(bufferSize), m_uFlags(flags)
{
    reset();
}

CommandBuffer::~CommandBuffer()
{
    m_uFlags &= ~Flags::isRecording;
}

void CommandBuffer::bindPass(RenderPassHandle handle)
{
    m_uFlags |= Flags::isRecording;

    //RenderPass* renderPass = device->access

}

void CommandBuffer::reset()
{
    // isRecording = false;
    // currentRenderPass = nullptr;
    // currentPipeline = nullptr;
    // currentCommand = 0;
}

} // namespace Graphics
} // namespace Raptor