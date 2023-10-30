#include "GPUProfiler.h"

namespace Raptor
{
namespace Graphics
{

using Allocator = eastl::allocator;

GPUProfiler::GPUProfiler(Allocator& allocator, uint32 max_frames)
    : allocator(&allocator), max_frames(max_frames)
{

}

GPUProfiler::~GPUProfiler()
{

}

} // namespace Graphics
} // namespace Raptor