#include "Renderer.h"

namespace Raptor
{
namespace Graphics
{

using Allocator = eastl::allocator;

Renderer::Renderer(GPUDevice* gpu_device, Allocator& allocator)
    : gpu_device(gpu_device), allocator(&allocator)
{

}

Renderer::~Renderer()
{

}

} // namespace Graphics
} // namespace Raptor