#pragma once

#include "Types.h"
#include "wyhash.h"

namespace Raptor
{
namespace Core
{

inline uint64 HashBytes(void* data, sizet length, sizet seed = 0)
{
    return wyhash(data, length, seed, _wyp);
}

inline uint64 HashString(const char* string, sizet seed = 0)
{
    return wyhash(string, strlen(string), seed, _wyp);
}

}; // namespace Core
}; // namespace Raptor