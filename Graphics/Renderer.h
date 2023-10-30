#pragma once

#include <EASTL/allocator.h>
#include "GPUDevice.h"

namespace Raptor
{
namespace Graphics
{
class Renderer
{

    using Allocator = eastl::allocator;

    // TODO

public:

    Renderer(GPUDevice* gpu_device, Allocator& allocator);
    ~Renderer();

private:

    GPUDevice* gpu_device;
    Allocator* allocator;

}; // class Renderer
} // namespace Graphics
} // namespace Raptor