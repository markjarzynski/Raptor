#include "Buffer.h"

namespace Raptor
{
namespace Graphics
{

CreateBufferParams& CreateBufferParams::Reset()
{
    size = 0;
    data = nullptr;

    return *this;
}

CreateBufferParams& CreateBufferParams::Set(ResourceUsageType usage, VkBufferUsageFlags flags, uint32 size)
{
    this->usage = usage;
    this->flags = flags;
    this->size = size;

    return *this;
}

CreateBufferParams& CreateBufferParams::SetData(void* data)
{
    this->data = data;
    return *this;
}

CreateBufferParams& CreateBufferParams::SetName(const char* name)
{
    this->name = name;
    return *this;
}

} // namesace Graphics
} // namespace Raptor