#pragma once

#include <vulkan/vulkan.h>

#include "Types.h"
#include "Resources.h"
#include "DescriptorSetLayout.h"
#include "RenderPass.h"

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

enum class FillMode
{
    Wireframe, Solid, Point, Max
}; 

struct CreateRasterizationParams
{
    VkCullModeFlagBits cull_mode = VK_CULL_MODE_NONE;
    VkFrontFace front = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    FillMode fill = FillMode::Solid;
}; // struct CreateRasterizationParams

struct StencilOperationState
{
    VkStencilOp fail = VK_STENCIL_OP_KEEP;
    VkStencilOp pass = VK_STENCIL_OP_KEEP;
    VkStencilOp depth_fail = VK_STENCIL_OP_KEEP;
    VkCompareOp compare = VK_COMPARE_OP_ALWAYS;
    uint32 compare_mask = 0xff;
    uint32 write_mask = 0xff;
    uint32 reference = 0xff;
}; // struct StencilOperationState

struct CreateDepthStencilParams
{
    StencilOperationState front;
    StencilOperationState back;
    VkCompareOp depth_comparison = VK_COMPARE_OP_ALWAYS;

    uint8 depth_enable = 0;
    uint8 depth_write_enable = 0;
    uint8 stencil_enable = 0;
    uint8 pad;

    CreateDepthStencilParams& SetDepth(bool write, VkCompareOp comparison_test);

}; // struct CreateDepthStencilParams

enum class ColorWriteEnabled : uint8
{
    Red   = 0x1 << 0,
    Green = 0x1 << 1,
    Blue  = 0x1 << 2,
    Alpha = 0x1 << 3,
    All  = Red | Green | Blue | Alpha,
};

struct BlendState
{
    VkBlendFactor source_color = VK_BLEND_FACTOR_ONE;
    VkBlendFactor destination_color = VK_BLEND_FACTOR_ONE;
    VkBlendOp color_operation = VK_BLEND_OP_ADD;

    VkBlendFactor source_alpha = VK_BLEND_FACTOR_ONE;
    VkBlendFactor destination_alpha = VK_BLEND_FACTOR_ONE;
    VkBlendOp alpha_operation = VK_BLEND_OP_ADD;

    ColorWriteEnabled color_write_mask = ColorWriteEnabled::All;
    uint8 blend_enabled = 1;
    uint8 separte_blend = 1;
    uint8 pad;

}; // struct BlendState

struct CreateBlendStateParams
{
    BlendState blend_states[MAX_IMAGE_OUTPUTS];
    uint32 active_states = 0;
}; // CreateBlendStateParams

enum class VertexInputRate
{
    PerVertex, PerInstance, Max
};

struct VertexStream
{
    uint32 binding = 0;
    uint32 stride = 0;
    VertexInputRate input_rate = VertexInputRate::Max;
}; // struct VertexStream


namespace VertexComponentFormat
{
enum Enum
{
    Float, Float2, Float3, Float4,
    Mat4, 
    Byte, Byte4N, UByte, UByte4N,
    Short2, Short2N, Short4, Short4N,
    Uint, Uint2, Uint4,
    Max,
};

static VkFormat ToVkFormat(Enum format)
{
    static VkFormat s_vk_formats[Enum::Max] = {
        VK_FORMAT_R32_SFLOAT, VK_FORMAT_R32G32_SFLOAT, VK_FORMAT_R32G32B32_SFLOAT, VK_FORMAT_R32G32B32A32_SFLOAT,
        VK_FORMAT_R32G32B32A32_SFLOAT,
        VK_FORMAT_R8_SINT, VK_FORMAT_R8G8B8A8_SNORM, VK_FORMAT_R8_UINT, VK_FORMAT_R8G8B8A8_UINT, 
        VK_FORMAT_R16G16_SINT, VK_FORMAT_R16G16_SNORM, VK_FORMAT_R16G16B16A16_SINT, VK_FORMAT_R16G16B16A16_SNORM,
        VK_FORMAT_R32_UINT, VK_FORMAT_R32G32_UINT, VK_FORMAT_R32G32B32A32_UINT
    };
    return s_vk_formats[format];
}
}; // namespace VertexComponentFormat

// enum class VertexComponentFormat
// {
//     Float, Float2, Float3, Float4,
//     Mat4, 
//     Byte, Byte4N, UByte, UByte4N,
//     Short2, Short2N, Short4, Short4N,
//     Uint, Uint2, Uint4,
//     Max,
// };

struct VertexAttribute
{
    uint32 location = 0;
    uint32 binding = 0;
    //VkFormat format = VK_FORMAT_UNDEFINED;
    VertexComponentFormat::Enum format = VertexComponentFormat::Max;
    uint32 offset = 0;
}; // struct VertexAttribute

struct CreateVertexInputParams
{
    uint32 num_vertex_streams = 0;
    uint32 num_vertex_attributes = 0;

    VertexStream vertex_streams[MAX_VERTEX_STREAMS];
    VertexAttribute vertex_attributes[MAX_VERTEX_ATTRIBUTES];

    CreateVertexInputParams& Reset();
    CreateVertexInputParams& AddVertexAttribute( const VertexAttribute& attribute);
    CreateVertexInputParams& AddVertexStream(const VertexStream& stream);

}; // struct CreateVertexInputParams

struct ShaderStage
{
    const char* code = nullptr;
    uint32 code_size = 0;
    VkShaderStageFlagBits type = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
}; // struct ShaderStage

struct CreateShaderStateParams
{
    ShaderStage stages[MAX_SHADER_STAGES];

    const char* name = nullptr;

    uint32 stages_count = 0;
    uint32 spv_input = 0;

    CreateShaderStateParams& Reset();
    CreateShaderStateParams& SetName(const char* name);
    CreateShaderStateParams& AddStage(const char* code, uint32 code_size, VkShaderStageFlagBits type);
    CreateShaderStateParams& SetSpvInput(bool value);
}; // struct CreateShaderStateParams

struct ViewportState
{
    uint32 num_viewports = 0;
    uint32 num_scissors = 0;

    Viewport* viewport = nullptr;
    Rect2DInt* scissors = nullptr;
}; // struct ViewportState

struct CreatePipelineParams
{
    CreateRasterizationParams rasterization;
    CreateDepthStencilParams depth_stencil;
    CreateBlendStateParams blend_state;
    CreateVertexInputParams vertex_input;
    CreateShaderStateParams shaders;

    RenderPassOutput render_pass;
    DescriptorSetLayoutHandle descriptor_set_layout[MAX_DESCRIPTOR_SET_LAYOUTS];
    const ViewportState* viewport = nullptr;

    uint32 num_active_layouts = 0;

    const char* name = nullptr;

    CreatePipelineParams& AddDescriptorSetLayout(DescriptorSetLayoutHandle handle);
    RenderPassOutput& RenderPassOutput();
}; // struct CreatePipelineParams

struct PipelineDescription
{
    ShaderStateHandle shader;
}; // struct PipelineDescription
} // namespace Graphics
} // namespace Raptor