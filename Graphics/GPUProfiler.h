#pragma once

#include <EASTL/allocator.h>
#include "Types.h"

namespace Raptor
{
namespace Graphics
{
class GPUProfiler
{

    using Allocator = eastl::allocator;

    // TODO

public:

    GPUProfiler(Allocator& allocator, uint32 max_frames);
    ~GPUProfiler();

private:

    Allocator* allocator;

    uint32 max_frames;

}; // class GPUProfiler
} // namespace Graphics
} // namespace Raptor