#pragma once

#include <vulkan/vulkan.h>

#include "Types.h"
#include "Resources.h"
#include "DescriptorSetLayout.h"

namespace Raptor
{
namespace Graphics
{
struct DescriptorSet
{
    VkDescriptorSet vk_descriptor_set;

    ResourceHandle* resources = nullptr;
    SamplerHandle* samplers = nullptr;
    uint16* bindings = nullptr;

    const DescriptorSetLayout* layout = nullptr;
    uint32 num_resources = 0;

}; // struct DescriptorSet

struct CreateDescriptorSetParams
{
    ResourceHandle resources[MAX_DESCRIPTORS_PER_SET];
    SamplerHandle samplers[MAX_DESCRIPTORS_PER_SET];
    uint16 bindings[MAX_DESCRIPTORS_PER_SET];

    DescriptorSetLayoutHandle layout;
    uint32 num_resources = 0;

    const char* name = nullptr;

    CreateDescriptorSetParams& Reset();
    CreateDescriptorSetParams& SetLayout(DescriptorSetLayoutHandle layout);
    CreateDescriptorSetParams& Texture(TextureHandle texture, uint16 binding);
    CreateDescriptorSetParams& Buffer(BufferHandle buffer, uint16 binding);
    CreateDescriptorSetParams& TextureSampler(TextureHandle texture, SamplerHandle sampler, uint16 binding);
    CreateDescriptorSetParams& SetName(const char* name);
}; // struct CreateDescriptorSetParams

struct DesciptorSetDescription
{
    ResourceData resources[MAX_DESCRIPTORS_PER_SET];
    uint32 num_active_resources = 0;
}; // struct DesciptorSetDescription
} // namespace Graphics
} // namespace Raptor