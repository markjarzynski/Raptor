#include "CommandBuffer.h"

namespace Raptor
{
namespace Graphics
{

CommandBuffer::CommandBuffer(QueueType type, uint32 bufferSize, uint32 submitSize, Flags flags)
    : type(type), bufferSize(bufferSize), uFlags(flags)
{
    reset();
}

CommandBuffer::~CommandBuffer()
{
    uFlags &= ~Flags::isRecording;
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