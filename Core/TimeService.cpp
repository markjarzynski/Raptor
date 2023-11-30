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

// Taken from the Rust code base: https://github.com/rust-lang/rust/blob/3809bbf47c8557bd149b3e52ceb47434ca8378d5/src/libstd/sys_common/mod.rs#L124
// Computes (value*numer)/denom without overflow, as long as both
// (numer*denom) and the overall result fit into i64 (which is the case
// for our time conversions).
static int64 int64_mul_div(int64 value, int64 numer, int64 denom) {
    const int64 q = value / denom;
    const int64 r = value % denom;
    // Decompose value as (value/denom*denom + value%denom),
    // substitute into (value*numer)/denom and simplify.
    // r < denom, so (denom*numer) is the upper bound of (r*numer)
    return q * numer + r * numer / denom;
}

int64 Now()
{
#if defined(_MSC_VER)
    LARGE_INTEGER time;
    QueryPerformanceCounter(&time);

    const int64 microseconds = int64_mul_div(time.QuadPart, 1000000LL, sFrequency.QuadPart);
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