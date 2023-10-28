#pragma once
#include <EASTL/allocator.h>
#include "Types.h"

namespace Raptor
{
namespace Graphics
{
struct ResourcePool
{
    using Allocator = eastl::allocator;

    uint8* memory = nullptr;
    uint32* freeIndices = nullptr;
    Allocator* allocator = nullptr;

    uint32 freeIndicesHead = 0;
    uint32 poolSize = 16;
    uint32 resourceSize = 4;
    uint32 usedIndices = 0;

    void init(Allocator* allocator, uint32 poolSize, uint32 resourceSize);
    void shutdown();

    uint32 obtainResource();
    void releaseResource(uint32 handle);
    void freeAllResources();

    void* accessResource(uint32 handle);
    const void* accessResource(uint32 handle) const;

}; // struct ResourcePool
} // namespace Graphics
} // namespace Raptor