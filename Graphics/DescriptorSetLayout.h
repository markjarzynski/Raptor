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

struct DescriptorSetLayoutDescription
{
    ResourceBinding bindings[MAX_DESCRIPTORS_PER_SET];
    uint32 num_active_bindings = 0;
}; // struct DescriptorSetLayoutDescription

struct CreateDescriptorSetLayoutParams
{

    struct Binding
    {
        VkDescriptorType vk_descriptor_type = VK_DESCRIPTOR_TYPE_MAX_ENUM;
        uint16 start = 0;
        uint16 count = 0;
        const char* name = nullptr;
    }; // struct Binding

    Binding bindings[MAX_DESCRIPTORS_PER_SET];
    uint32 num_bindings = 0;
    uint32 set_index = 0;

    const char* name = nullptr;

    CreateDescriptorSetLayoutParams& Reset();
    CreateDescriptorSetLayoutParams& AddBinding(const Binding& binding);
    CreateDescriptorSetLayoutParams& SetName(const char* name);
    CreateDescriptorSetLayoutParams& SetIndex(uint32 index);

}; // CreateDescriptorSetLayoutParams

} // namespace Graphics
} // namespace Raptor