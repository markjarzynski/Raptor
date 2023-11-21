#if defined(_MSC_VER)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <time.h>
#endif

#include "TimeService.h"

namespace Raptor
{
namespace Core
{
namespace Time
{

#if defined(_MSC_VER)
static LARGE_INTEGER sFrequency;
#endif

void Init()
{
#if defined(_MSC_VER)
    QueryPerformanceFrequency(&sFrequency);
#endif
}

int64 Now()
{
#if defined(_MSC_VER)
    LARGE_INTEGER time;
    QueryPerformanceFrequency(&time);

    const int64 microseconds = (time.QuadPart / sFrequency.QuadPart) * 1000000LL + (time.QuadPart % sFrequency.QuadPart) * 1000000LL / sFrequency.QuadPart;
#else
    timespec tp;
    clock_gettime( CLOCK_MONOTONIC, &tp );

    const uint64 now = tp.tv_sec * 1000000000 + tp.tv_nsec;
    const int64 microseconds = now / 1000;
#endif
    return microseconds;
}

double DeltaSeconds(int64 start_time, int64 end_time)
{
    return Seconds(end_time - start_time);
}

double Seconds(int64 time)
{
    return (double)time / 1000000.0;
}

} // namespace Time
} // namespace Core
} // namespace Raptor