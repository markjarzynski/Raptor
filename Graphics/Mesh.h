#pragma once

#include <vulkan/vulkan.h>
#include "Resources.h"
#include "Types.h"
#include "Vector.h"
#include "Matrix.h"

namespace Raptor
{
namespace Graphics
{
using Raptor::Math::vec3f;
using Raptor::Math::vec4f;
using Raptor::Math::mat4f;

enum MaterialFeatures : uint32
{
    ColorTexture            = 1 << 0,
    NormalTexture           = 1 << 1,
    RoughnessTexture        = 1 << 2,
    OcclusionTexture        = 1 << 3,
    EmissiveTexture         = 1 << 4,
    TangentVertexAttribute  = 1 << 5,
    TexcoordVertexAttribute = 1 << 6,
};

struct MaterialData
{
    vec4f base_color_factor;
    mat4f model;
    mat4f model_inv;

    vec3f emissive_factor;
    float metallic_factor;

    float roughness_factor;
    float occlusion_factor;
    uint32 flags;
}; // struct MaterialData

struct MeshDraw
{
    BufferHandle index_buffer;
    BufferHandle position_buffer;
    BufferHandle tangent_buffer;
    BufferHandle normal_buffer;
    BufferHandle texcoord_buffer;

    BufferHandle material_buffer;
    MaterialData material_data;

    uint32 index_offset;
    uint32 position_offset;
    uint32 tangent_offset;
    uint32 normal_offset;
    uint32 texcoord_offset;

    uint32 count;

    VkIndexType vk_index_type;

    DescriptorSetHandle descriptor_set;
}; // struct MeshDraw
} // namespace Graphics
} // namespace Raptor