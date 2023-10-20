#include "memory.h"
#include "ResourcePool.h"
#include "Constants.h"
#include "Debug.h"
#include "Log.h"

namespace Raptor
{
namespace Graphics
{

void ResourcePool::init(Allocator* allocator_, uint32 poolSize_, uint32 resourceSize_)
{
    allocator = allocator_;
    poolSize = poolSize_;
    resourceSize = resourceSize_;

    sizet size = poolSize * (resourceSize + sizeof(uint32));
    memory = (uint8*)allocator->allocate(size, 1, 0, 0);
    memset(memory, 0, size);

    freeIndices = (uint32*)(memory + poolSize * resourceSize);
    freeIndicesHead = 0;

    for (uint32 i = 0; i < poolSize; ++i)
    {
        freeIndices[i] = i;
    }

    usedIndices = 0;
}

void ResourcePool::shutdown()
{
    if (freeIndicesHead != 0)
    {
        Raptor::Debug::Log("[ResoucePool]: Warning: Resource Pool has unfreed resouces:\n");

        for (uint32 i = 0; i < freeIndicesHead; ++i)
        {
            Raptor::Debug::Log("\tResource %u\n", freeIndices[i]);
        }
    }

    ASSERT(usedIndices == 0);

    sizet size = poolSize * (resourceSize + sizeof(uint32));
    allocator->deallocate(memory, size);
}

uint32 ResourcePool::obtainResource()
{
    if (freeIndicesHead < poolSize)
    {
        const uint32 freeIndex = freeIndices[freeIndicesHead++];
        ++usedIndices;
        return freeIndex;
    }

    ASSERT_MESSAGE(false, "[ResourcePool]: Error: No more resources left.")
    return INVALID_INDEX;
}

void ResourcePool::releaseResource(uint32 handle)
{
    freeIndices[--freeIndicesHead] = handle;
    --usedIndices;
}

void ResourcePool::freeAllResources()
{
    freeIndicesHead = 0;
    usedIndices = 0;

    for (uint32 i = 0; i < poolSize; ++i)
    {
        freeIndices[i] = i;
    }
}

void* ResourcePool::accessResouce(uint32 handle)
{
    if (handle != INVALID_INDEX)
    {
        return &memory[handle * resourceSize];
    }
    return nullptr;
}

const void* ResourcePool::accessResource(uint32 handle) const
{
    if (handle != INVALID_INDEX)
    {
        return &memory[handle * resourceSize];
    }
    return nullptr;
}

} // namespace Graphics
} // namespace Raptor