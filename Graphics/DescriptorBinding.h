#pragma once

#include <vulkan/vulkan.h>
#include "Types.h"

namespace Raptor
{
namespace Graphics
{
struct DescriptorBinding
{
    VkDescriptorType vk_descriptor_type;
    uint16 start = 0;
    uint16 end = 0;
    uint16 set = 0;
    
    const char* name = nullptr;

}; // struct DescriptorBinding
} // namespace Graphics
} // namespace Raptor