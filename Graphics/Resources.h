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

} // namespace Graphics
} // namespace Raptor