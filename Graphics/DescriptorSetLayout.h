#pragma once

#include <vulkan/vulkan.h>
#include "Types.h"
#include "Resources.h"
#include "DescriptorBinding.h"

namespace Raptor
{
namespace Graphics
{
struct DescriptorSetLayout
{
    VkDescriptorSetLayout vk_descriptor_set_layout;

    VkDescriptorSetLayoutBinding* vk_binding = nullptr;
    DescriptorBinding* bindings = nullptr;
    uint16 num_bindings = 0;
    uint16 set_index =0;

    DescriptorSetLayoutHandle handle;

}; // struct DescriptorSet
} // namespace Graphics
} // namespace Raptor