#pragma once

#include "Log.h"
#include "Debug.h"

namespace Raptor
{
namespace Debug
{

#define ASSERT(condition) if (!(condition)) { DEBUG_BREAK }
#define ASSERT_MESSAGE(condition, message, ...) if (!(condition)) { Raptor::Debug::Log(message, __VA_ARGS__); DEBUG_BREAK }

} // namespace Debug
} // namespace Raptor