#include "VectorMath.h"

namespace Raptor
{
namespace Math
{

template<typename T>
T dot(vec2<T> v, vec2<T> u)
{
    return v.x * u.x + v.y * u.y;
}

template<typename T>
T dot(vec3<T> v, vec3<T> u)
{
    return v.x * u.x + v.y * u.y + v.z * u.z;
}

template<typename T>
T dot(vec4<T> v, vec4<T> u)
{
    return v.x * u.x + v.y * u.y + v.z * u.z + v.w * u.w;
}

template<typename T>
vec3<T>* cross(vec3<T> v, vec3<T> u)
{
    vec3<T> w;
    w.x = v.y * u.z - v.z * u.y;
    w.y = v.z * u.x - v.x * u.z;
    w.z = v.x * u.y - v.y * u.x;
    return w;
}

} // namespace Math
} // namespace Raptor