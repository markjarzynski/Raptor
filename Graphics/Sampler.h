#pragma once
#include <vulkan/vulkan.h>
#include "Resources.h"

namespace Raptor
{
namespace Graphics
{
struct Sampler
{
    VkSampler sampler;

    VkFilter minFilter = VK_FILTER_NEAREST;
    VkFilter magFilter = VK_FILTER_NEAREST;
    VkSamplerMipmapMode mipFilter = VK_SAMPLER_MIPMAP_MODE_NEAREST;

    VkSamplerAddressMode addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    VkSamplerAddressMode addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    VkSamplerAddressMode addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    const char* name = nullptr;
}; // struct Sampler
} // namespace Graphics
} // namespace Raptor