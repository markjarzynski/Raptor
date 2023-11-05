#pragma once

#include <EASTL/allocator.h>
#include "Types.h"
#include "GPUDevice.h"
#include "GPUTimestampManager.h"

namespace Raptor
{
namespace Graphics
{

enum GPUProfilerFlags : uint32
{
    isPaused = 0x1,
    isShutdown = 0x1 << 1,
};

class GPUProfiler
{

    using Allocator = eastl::allocator;

    // TODO

public:

    GPUProfiler(Allocator& allocator, uint32 max_frames);
    ~GPUProfiler();

    void Init(Allocator& _allocator, uint32 max_frames);
    void Shutdown();

    void Update(GPUDevice& gpu_device);
    void Draw();

private:

    Allocator* allocator;
    GPUTimestamp* timestamps;
    uint16* per_frame_active;

    uint32 max_frames;
    uint32 current_frame = 0;

    float max_time = 0.f;
    float min_time = 0.f;
    float average_time = 0.f;
    
    float max_duration = 16.666f;
    uint32 m_uFlags = GPUProfilerFlags::isPaused;

}; // class GPUProfiler
} // namespace Graphics
} // namespace Raptor