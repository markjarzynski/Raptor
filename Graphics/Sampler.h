#pragma once
#include <vulkan/vulkan.h>
#include "Resources.h"

namespace Raptor
{
namespace Graphics
{
struct Sampler
{
    VkSampler vk_sampler;

    VkFilter min_filter = VK_FILTER_NEAREST;
    VkFilter mag_filter = VK_FILTER_NEAREST;
    VkSamplerMipmapMode mip_filter = VK_SAMPLER_MIPMAP_MODE_NEAREST;

    VkSamplerAddressMode address_mode_u = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    VkSamplerAddressMode address_mode_v = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    VkSamplerAddressMode address_mode_w = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    const char* name = nullptr;
}; // struct Sampler
} // namespace Graphics
} // namespace Raptor