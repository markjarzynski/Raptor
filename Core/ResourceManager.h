#pragma once

#include <EASTL/allocator.h>
#include <EASTL/hash_map.h>
#include "Types.h"
#include "Debug.h"

namespace Raptor
{
namespace Core
{

using Allocator = eastl::allocator;

template<typename Key, typename T>
using HashMap = eastl::hash_map<Key,T>;

template<typename Key, typename T>
using Pair = eastl::pair<Key, T>;

class ResourceManager;

struct Resource
{
    uint64 references = 0;
    const char* name = nullptr;

    void AddReference() { references++; }
    void RemoveReference() { ASSERT(references != 0); references--; }

}; // struct Resource

struct ResourceCompiler
{

}; // ResourceCompiler

struct ResourceLoader
{
    virtual Resource* Get(const char* name) = 0;
    virtual Resource* Get(uint64 hash) = 0;
    virtual Resource* Unload(const char* name) = 0;
    virtual Resource* CreateFromFile(const char* name, const char* filename, ResourceManager* resource_manager) { return nullptr; }
}; // struct ResourceLoader

struct ResourceFilenameResolver
{
    virtual const char* GetBinaryPathFromName(const char* name) = 0;
}; // struct ResourceFilenameResolver

class ResourceManager
{
public:

    ResourceManager(Allocator& allocator, ResourceFilenameResolver* resolver);
    ~ResourceManager();

    template <typename T>
    T* Load(const char* name);

    template <typename T>
    T* Get(const char* name);

    template <typename T>
    T* Get(uint64 hash);

    template <typename T>
    T* Reload(const char* name);

    void SetLoader(const char* resource_type, ResourceLoader* loader);
    void SetCompiler(const char* resource_type, ResourceCompiler* compiler);

    HashMap<uint64, ResourceLoader*> loaders;
    HashMap<uint64, ResourceCompiler*> compilers;

    Allocator* allocator;
    ResourceFilenameResolver* filename_resolver;

}; // class ResourceManager

template<typename T>
inline T* ResourceManager::Load(const char* name)
{
    ResourceLoader* loader = loaders.at(T::type_hash);
    if (loader)
    {
        T* resource = (T*)loader->Get(name);
        if (resource)
            return resource;

        const char* path = filename_resolver->GetBinaryPathFromName(name);
        return (T*)loader->CreateFromFile(name, path, this);
    }
    return nullptr;
}

template <typename T>
inline T* ResourceManager::Get(const char* name)
{
    ResourceLoader* loader = loaders.at(T::type_hash);
    if (loader)
        return (T*)loader->Get(name);

    return nullptr;
}

template <typename T>
inline T* ResourceManager::Get(uint64 hash)
{
    ResourceLoader* loader = loaders.at(T::type_hash);
    if (loader)
        return (T*)loader->Get(hash);

    return nullptr;
}

template <typename T>
inline T* ResourceManager::Reload(const char* name)
{
    ResourceLoader* loader = loaders.at(T::type_hash);
    if (loader)
    {
        T* resource = (T*)loader->Get(name);
        if (resource)
        {
            loader->Unload(name);

            const char* path = filename_resolver->GetBinaryPathFromName(name);
            return (T*)loader->CreateFromFile(name, path, this);
        }
    }

    return nullptr;
}

} // namespace Core
} // namespace Raptor