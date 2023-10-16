#pragma once

#include "Log.h"

namespace Raptor
{
namespace Debug
{

#if defined(_MSC_VER)
#define DEBUG_BREAK __debugbreak();
#elif defined(__aarch64__) && defined(__APPLE__)
#define DEBUG_BREAK __builtin_debugtrap();
#endif

#define ASSERT(condition) if (!(condition)) { DEBUG_BREAK }
#define ASSERT_MESSAGE(condition, message, ...) if (!(condition)) { Raptor::Debug::Log(message, __VA_ARGS__); DEBUG_BREAK }

} // namespace Debug
} // namespace Raptor