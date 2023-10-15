#pragma once

#include <EASTL/allocator.h>
#include "Type.h"

namespace Raptor
{
namespace Graphics
{
struct GPUTimestamp
{
    uint32 start;
    uint32 end;
    double elapsedMS;
    uint16 parentIndex;
    uint16 depth;
    uint32 color;
    uint32 frameIndex;
    const char* name;
}; // struct GPUTimestamp

class GPUTimestampManager
{

    using Allocator = eastl::allocator;

public:
    GPUTimestampManager(Allocator* allocator, uint16 queriesPerFrame, uint16 maxFrames);
    ~GPUTimestampManager();

    bool HasValidQueries() const;
    void reset();
    uint32 resolve(uint32 currentFrame, GPUTimestamp* timestampsToFill);

    uint32 push(uint32 currentFrame, const char* name);
    uint32 pop(uint32 currentFrame);

private:

    Allocator* allocator = nullptr;
    GPUTimestamp* timestamps = nullptr;
    uint64* timestampsData = nullptr;

    uint32 queriesPerFrame = 0;
    uint32 currentQuery = 0;
    uint32 parentIndex = 0;
    uint32 depth = 0;

    uint32 maxFrames = 3;
    static const uint32 DATA_PER_QUERY = 2;

    bool currentFrameResolved = false;

}; // class GPUTimestampManager
} // namespace Graphics
} // namespace Raptor