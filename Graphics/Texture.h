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
    VkImage vk_image;
    VkImageView vl_image_view;
    VkFormat vk_format;
    VkImageLayout vk_image_layout;
    VmaAllocation vma_allocation;

    uint16 width = 1;
    uint16 height = 1;
    uint16 depth = 1;
    uint8 mipmaps = 1;

    enum Flags : uint8
    {
        Default      = 0x1 << 0,
        RenderTarget = 0x1 << 1,
        Compute      = 0x1 << 2,
    };
    uint8 flags = 0;

    TextureHandle handle;

    /*
    enum class Type
    {
        Texture1D, Texture2D, Texture3D, Texture1DArray, Texture2DArray, TextureCubeArray, Max,
    };
    Type type = Type::Texture2D;
    */
    VkImageType vk_image_type = VK_IMAGE_TYPE_2D;
    VkImageViewType vk_image_view_type = VK_IMAGE_VIEW_TYPE_2D;

    Sampler* sampler = nullptr;

    const char* name = nullptr;
}; // struct Texture

struct CreateTextureParams
{
    VkFormat vk_format = VK_FORMAT_UNDEFINED;
    //Texture::Type type = Texture::Type::Texture2D;
    VkImageType vk_image_type = VK_IMAGE_TYPE_2D;
    VkImageViewType vk_image_view_type = VK_IMAGE_VIEW_TYPE_2D;

    uint16 width = 1;
    uint16 height = 1;
    uint16 depth = 1;
    uint8 mipmaps = 1;
    uint8 flags = 0;

    const char* name = nullptr;
    void* data = nullptr;
}; // struct CreateTextureParams

} // namespace Graphics
} // namespace Raptor