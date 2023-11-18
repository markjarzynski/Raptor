#pragma once
#include "Types.h"

namespace Raptor
{
namespace Core
{
namespace Time
{

void Init();

int64 Now();

double DeltaSeconds(int64 start_time, int64 end_time);

double Seconds(int64 time);

} // namespace Time
} // namespace Core
} // namespace Raptor