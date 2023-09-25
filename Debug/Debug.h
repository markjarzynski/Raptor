#pragma once

#include "Log.h"

namespace Raptor
{
namespace Debug
{

#define DEBUG_BREAK __debugbreak();

#define ASSERT(condition) if (!(condition)) { DEBUG_BREAK }
#define ASSERT_MESSAGE(condition, message, ...) if (!(condition)) { Raptor::Debug::Log(message, __VA_ARGS__); DEBUG_BREAK }

} // namespace Debug
} // namespace Raptor