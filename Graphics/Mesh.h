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
    MeshDraw()
    {
        index_buffer = position_buffer = tangent_buffer = normal_buffer = texcoord_buffer = InvalidBuffer;
        material_buffer = InvalidBuffer;
        material_data = MaterialData();
        index_offset = position_offset = tangent_offset = normal_offset = texcoord_offset = 0;
        count = 0;
        vk_index_type = VK_INDEX_TYPE_MAX_ENUM;
        descriptor_set = InvalidDescriptorSet;
    }

    ~MeshDraw(){}
    
    MeshDraw(MeshDraw& other)
    {
        index_buffer = other.index_buffer;
        position_buffer = other.position_buffer;
        tangent_buffer = other.tangent_buffer;
        normal_buffer = other.normal_buffer;
        texcoord_buffer = other.texcoord_buffer;

        material_buffer = other.material_buffer;
        material_data = other.material_data;

        index_offset = other.index_offset;
        position_offset = other.position_offset;
        tangent_offset = other.tangent_offset;
        normal_offset = other.normal_offset;
        texcoord_offset = other.texcoord_offset;

        count = other.count;

        vk_index_type = other.vk_index_type;

        descriptor_set = other.descriptor_set;
    }

    MeshDraw(const MeshDraw& other)
    {
        index_buffer = other.index_buffer;
        position_buffer = other.position_buffer;
        tangent_buffer = other.tangent_buffer;
        normal_buffer = other.normal_buffer;
        texcoord_buffer = other.texcoord_buffer;

        material_buffer = other.material_buffer;
        material_data = other.material_data;

        index_offset = other.index_offset;
        position_offset = other.position_offset;
        tangent_offset = other.tangent_offset;
        normal_offset = other.normal_offset;
        texcoord_offset = other.texcoord_offset;

        count = other.count;

        vk_index_type = other.vk_index_type;

        descriptor_set = other.descriptor_set;
    }

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