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

enum ResourceUsageType
{
    Immutable, Dynamic, Stream, Max
};



} // namespace Graphics
} // namespace Raptor