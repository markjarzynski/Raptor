#include "RenderPass.h"

namespace Raptor
{
namespace Graphics
{

RenderPassOutput& RenderPassOutput::reset()
{
    numColorFormats = 0;
    for (uint32 i = 0; i < MAX_IMAGE_OUTPUTS; ++i)
    {
        colorFormats[i] = VK_FORMAT_UNDEFINED;
    }
    depthSteniclFormat = VK_FORMAT_UNDEFINED;
    colorOperation = depthOperation = stencilOperation = RenderPassOperation::Any;
    return *this;
}

RenderPassOutput& RenderPassOutput::color(VkFormat format)
{
    colorFormats[numColorFormats++] = format;
    return *this;
}

RenderPassOutput& RenderPassOutput::depth(VkFormat format)
{
    depthSteniclFormat = format;
    return *this;
}

RenderPassOutput& RenderPassOutput::setOperations(RenderPassOperation color, RenderPassOperation depth, RenderPassOperation stencil)
{
    colorOperation = color;
    depthOperation = depth;
    stencilOperation = stencil;
    return *this;
}

} // namespace Graphics
} // namespace Raptor