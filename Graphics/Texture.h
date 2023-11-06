#pragma once
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include "Types.h"
#include "Resources.h"
#include "Sampler.h"
#include "ResourceManager.h"

namespace Raptor
{
namespace Graphics
{
using Raptor::Core::Resource;

struct Texture
{
    VkImage vk_image;
    VkImageView vk_image_view;
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

struct TextureDescription
{
    void* native_handle = nullptr;
    const char* name = nullptr;

    uint16 width = 1;
    uint16 height = 1;
    uint16 depth = 1;
    uint8 mipmaps = 1;
    uint8 render_target = 0;
    uint8 compute_access = 0;

    VkFormat vk_format = VK_FORMAT_UNDEFINED;
    VkImageType vk_image_type = VK_IMAGE_TYPE_2D;
    VkImageViewType vk_image_view_type = VK_IMAGE_VIEW_TYPE_2D;
}; // struct TextureDescription

struct TextureResource : public Resource
{
    TextureHandle handle;
    uint32 pool_index;
    TextureDescription desc;

    static constexpr char* type = "TextureType";
    static uint64 type_hash;
}; // struct TextureResource

namespace TextureFormat
{

inline bool HasDepth(VkFormat vk_format)
{
    return (vk_format >= VK_FORMAT_D16_UNORM && vk_format < VK_FORMAT_S8_UINT ) || 
        (vk_format >= VK_FORMAT_D16_UNORM_S8_UINT && vk_format <= VK_FORMAT_D32_SFLOAT_S8_UINT);
}

inline bool HasStencil(VkFormat vk_format)
{
    return vk_format >= VK_FORMAT_S8_UINT && vk_format <= VK_FORMAT_D32_SFLOAT_S8_UINT;

}

inline bool HasDepthOrStencil(VkFormat vk_format)
{
    return vk_format >= VK_FORMAT_D16_UNORM && vk_format <= VK_FORMAT_D32_SFLOAT_S8_UINT;
}

} // namespace TextureFormat;


} // namespace Graphics
} // namespace Raptor