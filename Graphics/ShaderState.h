#pragma once

#include <vulkan/vulkan.h>

#include "Types.h"
#include "Resources.h"

namespace Raptor
{
namespace Graphics
{
struct ShaderState
{
    VkPipelineShaderStageCreateInfo shader_stage_info[MAX_SHADER_STAGES];

    const char* name = nullptr;

    uint32 active_shaders = 0;
    bool graphics_pipeline = false;

}; // struct ShaderState

struct ShaderStateDescription
{
    void* native_handle = nullptr;
    const char* name = nullptr;
}; // struct ShaderStateDescription
} // namespace Graphics
} // namespace Raptor