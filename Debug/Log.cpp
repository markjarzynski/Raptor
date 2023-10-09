#include "Log.h"
#include <EAStdC/EASprintf.h>

namespace Raptor
{
namespace Debug
{

void Log(char* message)
{
    EA::StdC::Fprintf(stderr, message);
}

void Log(const char* message...)
{
	va_list arguments;
	va_start(arguments, message);

    EA::StdC::Vfprintf(stderr, message, arguments);

	va_end(arguments);
}

} // namespace Debug
} // namespace Raptor