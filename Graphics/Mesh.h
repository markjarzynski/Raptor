#pragma once

#include <vulkan/vulkan.h>
#include "Resources.h"
#include "Types.h"
#include "VectorMath.h"
#include "Matrix.h"

namespace Raptor
{
namespace Graphics
{
using Raptor::Math::vec3s;
using Raptor::Math::vec4s;
using Raptor::Math::mat4s;

struct MaterialData
{
    vec4s base_color_factor;
    mat4s model;
    mat4s model_inv;

    vec3s emissive_factor;
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