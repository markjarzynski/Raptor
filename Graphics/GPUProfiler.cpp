#include <eastl/hash_map.h>

#include "GPUProfiler.h"
#include "Hash.h"

namespace Raptor
{
namespace Graphics
{

//template<typename Key, typename T>
//using HashMap = eastl::hash_map<Key, T>;

using Allocator = eastl::allocator;
using Raptor::Core::HashString;

eastl::hash_map<uint64,uint32> name_color;

static uint32 initial_frames_paused = 3;


GPUProfiler::GPUProfiler(Allocator& allocator, uint32 max_frames)
{
    Init(allocator, max_frames);
}

GPUProfiler::~GPUProfiler()
{
    if (!(m_uFlags & GPUProfilerFlags::isShutdown))
        Shutdown();
}

void GPUProfiler::Init(Allocator& _allocator, uint32 max_frames)
{
    this->allocator = &_allocator;
    this->max_frames = max_frames;

    timestamps = (GPUTimestamp*)allocator->allocate(sizeof(GPUTimestamp) * max_frames * 32);
    per_frame_active = (uint16*)allocator->allocate(sizeof(uint16) * max_frames);

    memset(per_frame_active, 0, 2 * max_frames);
}

void GPUProfiler::Shutdown()
{
    allocator->deallocate(timestamps, sizeof(GPUTimestamp) * max_frames * 32);
    allocator->deallocate(per_frame_active, sizeof(uint16) * max_frames);

    m_uFlags |= GPUProfilerFlags::isShutdown;
}

void GPUProfiler::Update(GPUDevice& gpu_device)
{
    gpu_device.m_uFlags |= (!(m_uFlags & GPUProfilerFlags::isPaused)) ? GPUDevice::Flags::TimestampsEnabled : 0;

    if (initial_frames_paused)
    {
        --initial_frames_paused;
        return;
    }

    if ((m_uFlags & GPUProfilerFlags::isPaused) && !(gpu_device.m_uFlags & GPUDevice::Flags::Resized))
        return;

    uint32 active_timestamps = gpu_device.GetGPUTimestamps(&timestamps[32 * current_frame]);
    per_frame_active[current_frame] = (uint16)active_timestamps;

    for (uint32 i = 0; i < active_timestamps; i++)
    {
        GPUTimestamp& timestamp = timestamps[32 * current_frame + i];

        uint64 hash = HashString(timestamp.name);
        uint32 color_index = name_color.at(hash);

        if (color_index == 0xffffffffu)
            color_index = (uint32)name_color.size();

            
    }

    // TODO:
}

void GPUProfiler::Draw()
{

}

} // namespace Graphics
} // namespace Raptor