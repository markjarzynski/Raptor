#pragma once
#include <vulkan/vulkan.h>
#include "Resources.h"
#include "ResourceManager.h"

namespace Raptor
{
namespace Graphics
{

using Raptor::Core::Resource;

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

struct CreateSamplerParams
{
    const char* name;

    VkFilter min_filter = VK_FILTER_NEAREST;
    VkFilter mag_filter = VK_FILTER_NEAREST;
    VkSamplerMipmapMode mip_filter = VK_SAMPLER_MIPMAP_MODE_NEAREST;

    VkSamplerAddressMode address_mode_u = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    VkSamplerAddressMode address_mode_v = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    VkSamplerAddressMode address_mode_w = VK_SAMPLER_ADDRESS_MODE_REPEAT;

}; // struct CreateSamplerParams

struct SamplerDescription
{
    const char* name = nullptr;

    VkFilter min_filter = VK_FILTER_NEAREST;
    VkFilter mag_filter = VK_FILTER_NEAREST;
    VkSamplerMipmapMode mip_filter = VK_SAMPLER_MIPMAP_MODE_NEAREST;

    VkSamplerAddressMode address_mode_u = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    VkSamplerAddressMode address_mode_v = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    VkSamplerAddressMode address_mode_w = VK_SAMPLER_ADDRESS_MODE_REPEAT;

}; // struct SamplerDescription

struct SamplerResource : public Resource
{
    SamplerHandle handle;
    uint32 pool_index;
    SamplerDescription desc;

    static constexpr char* type ="SamplerType";
    static uint64 type_hash;
}; // struct SamplerResource



} // namespace Graphics
} // namespace Raptor