#pragma once

#include "ResourceManager.h"

namespace Raptor
{
namespace Core
{

using Allocator = eastl::allocator;

ResourceManager::ResourceManager(Allocator& allocator)
    : allocator(&allocator)
{

}

ResourceManager::~ResourceManager()
{

}

} // namespace Core
} // namespace Raptor