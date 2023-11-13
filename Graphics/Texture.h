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

namespace TextureType
{
enum Enum
{
    Texture1D, Texture2D, Texture3D, Texture1DArray, Texture2DArray, TextureCubeArray, Max,
};

static VkImageType ToVkImageType(TextureType::Enum type)
{
    static VkImageType s_vk_image_type[TextureType::Enum::Max] = {
        VK_IMAGE_TYPE_1D, VK_IMAGE_TYPE_2D, VK_IMAGE_TYPE_3D, VK_IMAGE_TYPE_1D, VK_IMAGE_TYPE_2D, VK_IMAGE_TYPE_3D,
    };
    return s_vk_image_type[type];
}

static VkImageViewType ToVkImageViewType(TextureType::Enum type)
{
    static VkImageViewType s_vk_view_image_type[TextureType::Enum::Max] = {
        VK_IMAGE_VIEW_TYPE_1D, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_VIEW_TYPE_3D, VK_IMAGE_VIEW_TYPE_1D_ARRAY, VK_IMAGE_VIEW_TYPE_2D_ARRAY, VK_IMAGE_VIEW_TYPE_CUBE_ARRAY,
    };
    return s_vk_view_image_type[type];
}


} // namespace TextureType   



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

    TextureType::Enum type = TextureType::Enum::Texture2D;

    Sampler* sampler = nullptr;

    const char* name = nullptr;
}; // struct Texture

struct CreateTextureParams
{
    VkFormat vk_format = VK_FORMAT_UNDEFINED;
    TextureType::Enum type = TextureType::Enum::Texture2D;

    uint16 width = 1;
    uint16 height = 1;
    uint16 depth = 1;
    uint8 mipmaps = 1;
    uint8 flags = 0;

    const char* name = nullptr;
    void* data = nullptr;

    CreateTextureParams& SetSize(uint16 width, uint16 height, uint16 depth);
    CreateTextureParams& SetFlags(uint8 mipmaps, uint8 flags);
    CreateTextureParams& SetFormatType(VkFormat format, TextureType::Enum type);
    CreateTextureParams& SetName(const char* name);
    CreateTextureParams& SetData(void* data);

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
    TextureType::Enum type = TextureType::Enum::Texture2D;
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