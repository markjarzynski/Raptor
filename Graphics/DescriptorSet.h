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
    // TODO
}; // struct CreateDescriptorSetParams

struct DesciptorSetDescription
{
    ResourceData resources[MAX_DESCRIPTORS_PER_SET];
    uint32 num_active_resources = 0;
}; // struct DesciptorSetDescription
} // namespace Graphics
} // namespace Raptor