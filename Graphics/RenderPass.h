#pragma once

#include <vulkan/vulkan.h>
#include "Types.h"
#include "Resources.h"

namespace Raptor
{
namespace Graphics
{

enum RenderPassOperation : uint8
{
    Load = 0x1,
    Clear = 0x2,
    Any = Load | Clear,
};

class RenderPassOutput
{
public:
    RenderPassOutput(){}
    ~RenderPassOutput(){}

    RenderPassOutput& reset();
    RenderPassOutput& color(VkFormat format);
    RenderPassOutput& depth(VkFormat format);
    RenderPassOutput& setOperations(RenderPassOperation color, RenderPassOperation depth, RenderPassOperation stencil);

private:

    VkFormat colorFormats[MAX_IMAGE_OUTPUTS];
    VkFormat depthSteniclFormat;
    uint32 numColorFormats;

    RenderPassOperation colorOperation = RenderPassOperation::Any;
    RenderPassOperation depthOperation = RenderPassOperation::Any;
    RenderPassOperation stencilOperation = RenderPassOperation::Any;

}; // class RenderPassOutput

struct RenderPass
{
    VkRenderPass vk_render_pass;
    VkFramebuffer vk_frame_buffer;

    RenderPassOutput output;

    TextureHandle outputTextures[MAX_IMAGE_OUTPUTS];
    TextureHandle outputDepth;

    enum Type
    {
        Geometry,
        Swapchain,
        Compute,
        Max
    };
    RenderPass::Type type;

    float scaleX = 1.f;
    float scaleY = 1.f;
    uint16 width = 0;
    uint16 height = 0;
    uint16 dispatchX = 0;
    uint16 dispatchY = 0;
    uint16 dispatchZ = 0;

    uint8 resize = 0;
    uint8 numRenderTargets = 0;

    const char* name = nullptr;
}; // struct RenderPass

struct CreateRenderPassParams
{
    uint16 num_render_targets = 0;
    RenderPass::Type type = RenderPass::Type::Geometry;

    TextureHandle output_textures[MAX_IMAGE_OUTPUTS] = {InvalidTexture};
    TextureHandle depth_stencil_texture = InvalidTexture;

    float scale_x = 1.f;
    float scale_y = 1.f;
    uint8 resize = 1;

    RenderPassOperation color_operation = RenderPassOperation::Any;
    RenderPassOperation depth_operation = RenderPassOperation::Any;
    RenderPassOperation stencil_operation = RenderPassOperation::Any;

    const char* name = nullptr;
}; // struct CreateRenderPassParams


} // namespace Graphics
} // namespace Raptor