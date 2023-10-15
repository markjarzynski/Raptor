#include "GPUTimestampManager.h"
#include "Log.h"

#include <memory.h>

namespace Raptor
{
namespace Graphics
{

GPUTimestampManager::GPUTimestampManager(Allocator* allocator, uint16 queriesPerFrame, uint16 maxFrames)
    : allocator(allocator), queriesPerFrame(queriesPerFrame), maxFrames(maxFrames)
{
    const sizet size = sizeof(GPUTimestamp) * queriesPerFrame * maxFrames + sizeof(uint64) * queriesPerFrame * maxFrames * DATA_PER_QUERY;
    uint8* memory = (uint8*)allocator->allocate(size, 1, 0, 0);

    timestamps = (GPUTimestamp*)memory;
    timestampsData = (uint64*)(memory + sizeof(GPUTimestamp) * queriesPerFrame * maxFrames);

    reset();
}

GPUTimestampManager::~GPUTimestampManager()
{
    const sizet size = sizeof(GPUTimestamp) * queriesPerFrame * maxFrames + sizeof(uint64) * queriesPerFrame * maxFrames * DATA_PER_QUERY;
    allocator->deallocate(timestamps, size);
}

bool GPUTimestampManager::HasValidQueries() const
{
    // Even number of queries means asymettrical queries, thus don't sample.
    return (currentQuery > 0) && (depth == 0);
}

void GPUTimestampManager::reset()
{
    currentQuery = 0;
    parentIndex = 0;
    currentFrameResolved = false;
    depth = 0;
}

uint32 GPUTimestampManager::resolve(uint32 currentFrame, GPUTimestamp* timestampsToFill)
{
    memcpy(timestampsToFill, &timestamps[currentFrame * queriesPerFrame], sizeof(GPUTimestamp) * currentQuery);
    return currentQuery;
}

uint32 GPUTimestampManager::push(uint32 currentFrame, const char* name)
{
    uint32 queryIndex = (currentFrame * queriesPerFrame) + currentQuery;

    GPUTimestamp& timestamp = timestamps[queryIndex];
    timestamp.parentIndex = (uint16)parentIndex;
    timestamp.start = queryIndex * 2;
    timestamp.end = timestamp.start + 1;
    timestamp.name = name;
    timestamp.depth = (uint16)depth++;

    parentIndex = currentQuery;
    ++currentQuery;

    return queryIndex * 2;
}

uint32 GPUTimestampManager::pop(uint32 currentFrame)
{
    uint32 queryIndex = (currentFrame * queriesPerFrame) + parentIndex;
    GPUTimestamp& timestamp = timestamps[queryIndex];

    parentIndex = timestamp.parentIndex;
    --depth;

    return queryIndex * 2 + 1;
}


} // namespace Graphics
} // namespace Raptor