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

    ResourceHandle* resouces = nullptr;
    SamplerHandle* samplers = nullptr;
    uint16* bindings = nullptr;

    const DescriptorSetLayout* layout = nullptr;
    uint32 num_resources = 0;

}; // struct DescriptorSet
} // namespace Graphics
} // namespace Raptor