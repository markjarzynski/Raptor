#pragma once
#include <vk_mem_alloc.h>
#include "Resources.h"

namespace Raptor
{
namespace Graphics
{
struct Buffer
{
    VkBuffer vk_buffer;
    VmaAllocation vma_allocation;
    VkDeviceMemory vk_device_memory;
    VkDeviceSize vk_device_size;

    VkBufferUsageFlags flags = 0;
    ResourceUsageType usage = ResourceUsageType::Immutable;
    uint32 size = 0;
    uint32 global_offset = 0;

    BufferHandle handle;
    BufferHandle parent_buffer;

    const char* name = nullptr;
}; // struct Buffer

struct CreateBufferParams
{
    ResourceUsageType usage = ResourceUsageType::Immutable;
    uint32 size = 0;
    VkBufferUsageFlags flags = 0;
    void* data = nullptr;
    const char* name;
}; // struct CreateBufferParams

struct MapBufferParams
{
    BufferHandle buffer;
    uint32 offset = 0;
    uint32 size = 0;
}; // struct MapBufferParams

} // namespace Graphics
} // namespace Raptor