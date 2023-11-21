#pragma once
//#include <vk_mem_alloc.h>
#include "Types.h"
#include "Constants.h"

namespace Raptor
{
namespace Graphics
{

typedef uint32 ResourceHandle;

typedef ResourceHandle BufferHandle;
typedef ResourceHandle TextureHandle;
typedef ResourceHandle ShaderStateHandle;
typedef ResourceHandle SamplerHandle;
typedef ResourceHandle DescriptorSetLayoutHandle;
typedef ResourceHandle DescriptorSetHandle;
typedef ResourceHandle PipelineHandle;
typedef ResourceHandle RenderPassHandle;

static BufferHandle InvalidBuffer = INVALID_INDEX;
static TextureHandle InvalidTexture = INVALID_INDEX;
static ShaderStateHandle InvalidShaderState = INVALID_INDEX;
static SamplerHandle InvalidSampler = INVALID_INDEX;
static DescriptorSetLayoutHandle InvalidDescriptorSetLayout = INVALID_INDEX;
static DescriptorSetHandle InvalidDescriptorSet = INVALID_INDEX;
static PipelineHandle InvalidPipeline = INVALID_INDEX;
static RenderPassHandle InvalidRenderPass = INVALID_INDEX;

static const uint8 MAX_IMAGE_OUTPUTS = 8;
static const uint8 MAX_DESCRIPTOR_SET_LAYOUTS = 8;
static const uint8 MAX_SHADER_STAGES = 5;
static const uint8 MAX_DESCRIPTORS_PER_SET = 16;
static const uint8 MAX_VERTEX_STREAMS = 16;
static const uint8 MAX_VERTEX_ATTRIBUTES = 16;

static const uint32 SUBMIT_HEADER_SENTINEL = 0xfefeb7ba;
static const uint32 MAX_RESOURCE_DELETIONS = 64;

static const uint32 MAX_SWAPCHAIN_IMAGES = 3;

enum class QueueType
{
    Graphics, Compute, CopyTransfer, Max
};

struct Rect2D
{
    float x = 0.f;
    float y = 0.f;
    float width = 0.f;
    float height = 0.f;
}; // struct Rect2D

struct Rect2DInt
{
    uint16 x = 0;
    uint16 y = 0;
    uint16 width = 0;
    uint16 height = 0;
}; // struct Rect2DInt

struct Viewport 
{
    Rect2DInt rect;
    float min_depth = 0.f;
    float max_depth = 0.f;
}; // struct Viewport

enum class TopologyType
{
    Unknown, Point, Line, Triangle, Patch, Max
};

enum class ResourceUsageType
{
    Immutable, Dynamic, Stream, Max
};

enum class ResourceDeletionType
{
    Buffer, Texture, Pipeline, Sampler, DescriptorSetLayout, DescriptorSet, RenderPass, ShaderState, Max
};

struct ResourceUpdate
{
    ResourceDeletionType type;
    ResourceHandle handle;
    uint32 current_frame;
}; // struct ResourceUpdate

struct DescriptorSetUpdate
{
    DescriptorSetHandle handle;
    uint32 frame_issued = 0;
}; // struct DescriptorSetUpdate


enum PipelineStage 
{
    DrawIndirect, VertexInput, VertexShader, FragmentShader, RenderTarget, ComputeShader, Transfer, Max 
};

struct ImageBarrier
{
    TextureHandle texture;
}; // struct ImageBarrier

struct MemoryBarrier
{
    BufferHandle buffer;
}; // struct MemoryBarrier

struct ExecutionBarrier
{
    PipelineStage src_pipeline_stage = PipelineStage::DrawIndirect;
    PipelineStage dst_pipeline_stage = PipelineStage::DrawIndirect;

    uint32 new_barrier_experimental = UINT32_MAX;
    uint32 load_operation = 0;

    uint32 num_image_barriers = 0;
    uint32 num_memory_barriers = 0;

    ImageBarrier image_barriers[8];
    MemoryBarrier memory_barriers[8];

}; // struct ExecutionBarrier

struct ResourceData
{
    void* data = nullptr;
}; // struct ResourceData

struct ResourceBinding 
{
    uint16 type = 0;
    uint16 start = 0;
    uint16 count = 0;
    uint16 set = 0;

    const char* name = nullptr;
}; // struct ResourceBinding

enum ResourceState : uint32
{
    UNDEFINED                         = 0,
    VERTEX_AND_CONSTANT_BUFFER        = 0x1,
    INDEX_BUFFER                      = 0x2,
    RENDER_TARGET                     = 0x4,
    UNORDERED_ACCESS                  = 0x8,
    DEPTH_WRITE                       = 0x10,
    DEPTH_READ                        = 0x20,
    NON_PIXEL_SHADER_RESOURCE         = 0x40,
    PIXEL_SHADER_RESOURCE             = 0x80,
    SHADER_RESOURCE                   = 0x40 | 0x80,
    STREAM_OUT                        = 0x100,
    INDIRECT_ARGUMENT                 = 0x200,
    COPY_DEST                         = 0x400,
    COPY_SOURCE                       = 0x800,
    READ                              = 0x1 | 0x2 | 0x40 | 0x80 | 0x200 | 0x800,
    PRESENT                           = 0x1000,
    COMMON                            = 0x2000,
    RAYTRACING_ACCELERATION_STRUCTURE = 0x4000,
    SHADING_RATE_SOURCE               = 0x8000,
};

static VkAccessFlags ToVkAccessFlags(ResourceState state)
{
    VkAccessFlags flags = 0;

    if (state & ResourceState::COPY_SOURCE)
        flags |= VK_ACCESS_TRANSFER_READ_BIT;
    if (state & ResourceState::COPY_DEST)
        flags |= VK_ACCESS_TRANSFER_WRITE_BIT;
    if (state & ResourceState::VERTEX_AND_CONSTANT_BUFFER)
        flags |= VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
    if (state & ResourceState::INDEX_BUFFER)
        flags |= VK_ACCESS_INDEX_READ_BIT;
    if (state & ResourceState::UNORDERED_ACCESS)
        flags |= VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
    if (state & ResourceState::INDIRECT_ARGUMENT)
        flags |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
    if (state & ResourceState::RENDER_TARGET)
        flags |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    if (state & ResourceState::DEPTH_WRITE)
        flags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    if (state & ResourceState::SHADER_RESOURCE)
        flags |= VK_ACCESS_SHADER_READ_BIT;
    if (state & ResourceState::PRESENT)
        flags |= VK_ACCESS_MEMORY_READ_BIT;
#ifdef ENABLE_RAYTRACING
    if (state & ResourceState::RAYTRACING_ACCELERATION_STRUCTURE)
        flags |= VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV;
#endif

    return flags;
}

static VkImageLayout ToVkImageLayout(ResourceState state)
{
    if (state & ResourceState::COPY_SOURCE)
        return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    if (state & ResourceState::COPY_DEST)
        return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    if (state & ResourceState::RENDER_TARGET)
        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    if (state & ResourceState::DEPTH_WRITE)
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    if (state & ResourceState::DEPTH_READ)
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    if (state & ResourceState::UNORDERED_ACCESS)
        return VK_IMAGE_LAYOUT_GENERAL;
    if (state & ResourceState::SHADER_RESOURCE)
        return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    if (state & ResourceState::PRESENT)
        return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    if (state == ResourceState::COMMON)
        return VK_IMAGE_LAYOUT_GENERAL;

    return VK_IMAGE_LAYOUT_UNDEFINED;
}

static VkPipelineStageFlags ToVkPipelineStage(PipelineStage stage)
{
    static VkPipelineStageFlags vk_stage[] = {VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT};
    return vk_stage[stage];
}

static VkPipelineStageFlags DeterminePipelineStageFlags(VkAccessFlags accessFlags, QueueType queueType)
{
    VkPipelineStageFlags flags = 0;

    switch (queueType)
    {
        case QueueType::Graphics:
        {
            if ((accessFlags & (VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT)) != 0)
                flags |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;

            if ((accessFlags & (VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT)) != 0)
            {
                flags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
                flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
#ifdef ENABLE_RAYTRACING
                if (pRenderer->mVulkan.mRaytracingExtension)
                    flags |= VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV;
#endif
            }

            if ((accessFlags & VK_ACCESS_INPUT_ATTACHMENT_READ_BIT) != 0)
                flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

            if ((accessFlags & (VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)) != 0)
                flags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

            if ((accessFlags & (VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)) != 0)
                flags |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

        } break;

        case QueueType::Compute:
        {
            if ((accessFlags & (VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT)) != 0 ||
                (accessFlags & VK_ACCESS_INPUT_ATTACHMENT_READ_BIT) != 0 ||
                (accessFlags & (VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)) != 0 ||
                (accessFlags & (VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)) != 0)
                return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

            if ((accessFlags & (VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT)) != 0)
                flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

        } break;

        case QueueType::CopyTransfer : return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        default: break;
    }

    if ((accessFlags & VK_ACCESS_INDIRECT_COMMAND_READ_BIT) != 0)
        flags |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
    if ((accessFlags & (VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT)) != 0)
        flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;
    if ((accessFlags & (VK_ACCESS_HOST_READ_BIT | VK_ACCESS_HOST_WRITE_BIT)) != 0)
        flags |= VK_PIPELINE_STAGE_HOST_BIT;
    if (flags == 0)
        flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    return flags;
}


static const char* ToStageDefines(VkShaderStageFlagBits value)
{
    switch (value)
    {
        case VK_SHADER_STAGE_VERTEX_BIT:
            return "VERTEX";
        case VK_SHADER_STAGE_FRAGMENT_BIT:
            return "FRAGMENT";
        case VK_SHADER_STAGE_COMPUTE_BIT:
            return "Compute";
        default:
            return "";
    }
}

static const char* ToCompilerExtension(VkShaderStageFlagBits value)
{
    switch (value)
    {
        case VK_SHADER_STAGE_VERTEX_BIT:
            return "vert";
        case VK_SHADER_STAGE_FRAGMENT_BIT:
            return "frag";
        case VK_SHADER_STAGE_COMPUTE_BIT:
            return "comp";
        default:
            return "";
    }
}

} // namespace Graphics
} // namespace Raptor