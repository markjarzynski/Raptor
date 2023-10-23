#pragma once
#include <vk_mem_alloc.h>
#include "Resources.h"

namespace Raptor
{
namespace Graphics
{
struct Buffer
{
    VkBuffer buffer;
    VmaAllocation vmaAllocation;
    VkDeviceMemory deviceMemory;
    VkDeviceSize deviceSize;

    VkBufferUsageFlags typeFlags = 0;
    ResourceUsageType usage = ResourceUsageType::Immutable;
    uint32 size = 0;
    uint32 globalOffset = 0;

    BufferHandle handle;
    BufferHandle parentBuffer;

    const char* name = nullptr;
}; // struct Buffer
} // namespace Graphics
} // namespace Raptor