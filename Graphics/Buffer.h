#pragma once
#include <vk_mem_alloc.h>
#include "Types.h"
#include "Resources.h"
#include "ResourceManager.h"

namespace Raptor
{
namespace Graphics
{
using Raptor::Core::Resource;

class Renderer;

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
    VkBufferUsageFlags flags = 0;
    uint32 size = 0;
    void* data = nullptr;
    const char* name;
}; // struct CreateBufferParams

struct MapBufferParams
{
    BufferHandle buffer;
    uint32 offset = 0;
    uint32 size = 0;
}; // struct MapBufferParams

struct BufferDescription
{
    void* native_handle = nullptr;
    const char* name = nullptr;

    VkBufferUsageFlags type_flags = 0;
    ResourceUsageType usage = ResourceUsageType::Immutable;
    uint32 size = 0;
    BufferHandle parent_handle;
}; // struct BufferDescription

struct BufferResource : public Resource
{
    BufferHandle handle;
    uint32 pool_index;
    BufferDescription desc;

    static constexpr char* type = "BufferType";
    static uint64 type_hash;
}; // struct BufferResource

} // namespace Graphics
} // namespace Raptor