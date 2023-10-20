#pragma once

#include <vulkan/vulkan.h>
#include "Types.h"
#include "Resources.h"

namespace Raptor
{
namespace Graphics
{

class RenderPassOutput
{
public:
    RenderPassOutput(){}
    ~RenderPassOutput(){}

    enum Operation : uint8
    {
        Load = 0x1,
        Clear = 0x2,
        Any = Load | Clear,
    };

    RenderPassOutput& reset();
    RenderPassOutput& color(VkFormat format);
    RenderPassOutput& depth(VkFormat format);
    RenderPassOutput& setOperations(Operation color, Operation depth, Operation stencil);

private:

    VkFormat colorFormats[MAX_IMAGE_OUTPUTS];
    VkFormat depthSteniclFormat;
    uint32 numColorFormats;

    Operation colorOperation = Operation::Any;
    Operation depthOperation = Operation::Any;
    Operation stencilOperation = Operation::Any;

}; // class RenderPassOutput

struct RenderPass
{
    VkRenderPass renderPass;
    VkFramebuffer frameBuffer;

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



} // namespace Graphics
} // namespace Raptor