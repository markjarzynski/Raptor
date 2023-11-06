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

template<typename T>
struct ResourcePoolTyped : public ResourcePool
{
    void init(Allocator* allocator, uint32 pool_size);
    void shutdown();

    T* obtain();
    void release(T* resource);

    T* get(uint32 index);
    const T* get(uint32 index) const;

}; // struct ResourcePoolTyped

template<typename T>
inline void ResourcePoolTyped<T>::init(Allocator* allocator, uint32 pool_size)
{
    ResourcePool::init(allocator, pool_size, sizeof(T));
}

template<typename T>
inline void ResourcePoolTyped<T>::shutdown()
{
    if (freeIndicesHead != 0)
    {
        Raptor::Debug::Log("[ResoucePool]: Warning: Resource Pool has unfreed resouces:\n");

        for (uint32 i = 0; i < freeIndicesHead; ++i)
        {
            Raptor::Debug::Log("\tResource %u, %s\n", freeIndices[i], get(freeIndices[i])->name);
        }
    }

    ResourcePool::shutdown();
}

template<typename T>
inline T* ResourcePoolTyped<T>::obtain()
{
    uint32 resource_index = ResourcePool::obtainResource();
    if (resource_index != uint32_max)
    {
        T* resource = get(resource_index);
        resource->pool_index = resource_index;
        return resource;
    }

    return nullptr;
}

template<typename T>
inline void ResourcePoolTyped<T>::release(T* resource)
{
    ResourcePool::release_resource(resource->pool_index);
}

template<typename T>
inline T* ResourcePoolTyped<T>::get(uint32 index)
{
    return (T*)ResourcePool::accessResource(index);
}

template<typename T>
inline const T* ResourcePoolTyped<T>::get(uint32 index) const
{
    return (const T*)ResourcePool::accessResource(index);
}




} // namespace Graphics
} // namespace Raptor