#pragma once
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include "Types.h"
#include "Resources.h"
#include "Sampler.h"

namespace Raptor
{
namespace Graphics
{

struct Texture
{
    VkImage image;
    VkImageView imageView;
    VkFormat format;
    VkImageLayout imageLayout;
    VmaAllocation vmaAllocation;

    uint16 width = 1;
    uint16 height = 1;
    uint16 depth = 1;
    uint8 mipmaps = 1;
    uint8 flags = 0;

    TextureHandle handle;

    enum Type
    {
        Texture1D, Texture2D, Texture3D, Texture1DArray, Texture2DArray, TextureCubeArray, Max,
    };
    Type type = Type::Texture2D;

    Sampler* sampler = nullptr;

    const char* name = nullptr;
}; // struct Texture
} // namespace Graphics
} // namespace Raptor