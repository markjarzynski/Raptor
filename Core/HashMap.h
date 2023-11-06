#pragma once

#include <EASTL/hash_map.h>

namespace Raptor
{
namespace Core
{
template<typename Key, typename T>
using HashMap = eastl::hash_map<Key,T>;

template<typename Key, typename T>
using Pair = eastl::pair<Key, T>;

} // namespace Core
} // namespace Raptor