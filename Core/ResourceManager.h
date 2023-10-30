#pragma once

#include <EASTL/allocator.h>
#include <EASTL/hash_map.h>

namespace Raptor
{
namespace Core
{

using Allocator = eastl::allocator;

template<typename Key, typename T>
using HashMap = eastl::hash_map<Key,T>;

class ResourceManager
{

    // TODO

public:

    ResourceManager(Allocator& allocator);
    ~ResourceManager();

private:

    Allocator* allocator;

    //HashMap

}; // class ResourceManager
} // namespace Core
} // namespace Raptor