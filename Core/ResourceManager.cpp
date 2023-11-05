#pragma once

#include "ResourceManager.h"
#include "Hash.h"

namespace Raptor
{
namespace Core
{

using Allocator = eastl::allocator;

ResourceManager::ResourceManager(Allocator& allocator, ResourceFilenameResolver* resolver)
    : allocator(&allocator), filename_resolver(resolver)
{
    loaders.set_allocator(allocator);
    compilers.set_allocator(allocator);
}

ResourceManager::~ResourceManager()
{

}

void ResourceManager::SetLoader(const char* resource_type, ResourceLoader* loader)
{
    const uint64 hash = HashString(resource_type);
    Pair<uint64, ResourceLoader*> pair(hash, loader);
    loaders.insert(pair);
}

void ResourceManager::SetCompiler(const char* resource_type, ResourceCompiler* compiler)
{
    const uint64 hash = HashString(resource_type);
    Pair<uint64, ResourceCompiler*> pair(hash, compiler);
    compilers.insert(pair);
}



} // namespace Core
} // namespace Raptor