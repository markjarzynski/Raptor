#pragma once

#include <vulkan/vulkan.h>

#include "Types.h"
#include "Resources.h"
#include "DescriptorSetLayout.h"

namespace Raptor
{
namespace Graphics
{
struct Pipeline
{
    VkPipeline vk_pipeline;
    VkPipelineLayout vk_pipeline_layout;

    VkPipelineBindPoint vk_pipeline_bind_point;

    ShaderStateHandle shader_state;

    const DescriptorSetLayout* descriptor_set_layout[MAX_DESCRIPTOR_SET_LAYOUTS];
    DescriptorSetLayoutHandle descrptor_set_layout_handle[MAX_DESCRIPTOR_SET_LAYOUTS];
    uint32 num_active_layouts = 0;

    // DepthStencilCreation depth_stencil;
    // BlendStateCreation blend_state;
    // RasterizationCreation rasterization;

    PipelineHandle handle;
    bool graphics_pipeline = true;

}; // struct Pipeline
} // namespace Graphics
} // namespace Raptor