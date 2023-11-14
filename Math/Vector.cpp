#include "Vector.h"
#include "Vector.inl"
#include <math.h>

namespace Raptor
{
namespace Math
{


float vec3<float>::Length() const
{
    return sqrtf(x*x + y*y + z*z);
}

double vec3<double>::Length() const
{
    return sqrt(x*x + y*y + z*z);
}

template<typename T>
inline vec3<T>& vec3<T>::Normalize()
{
    float n = ((T)1) / Length();

    this->x *= n;
    this->y *= n;
    this->z *= n;

    return *this;
}

template<typename T>
inline vec3<T> operator + (const vec3<T>& lhs, T rhs)
{
    return vec3<T>(lhs.x + rhs, lhs.y + rhs, lhs.z + rhs);
}

template<typename T>
inline vec3<T> operator + (T lhs, const vec3<T>& rhs)
{
    return vec3<T>(lhs + rhs.x, lhs + rhs.y, lhs + rhs.z);
}

template<typename T>
inline vec3<T> operator - (const vec3<T>& lhs, T rhs)
{
    return vec3<T>(lhs.x - rhs, lhs.y - rhs, lhs.z - rhs);
}

template<typename T>
inline vec3<T> operator - (T lhs, const vec3<T>& rhs)
{
    return vec3<T>(lhs - rhs.x, lhs - rhs.y, lhs - rhs.z);
}

template<typename T>
inline vec3<T> operator * (const vec3<T>& lhs, T rhs)
{
    return vec3<T>(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs);
}

template<typename T>
inline vec3<T> operator * (T lhs, const vec3<T>& rhs)
{
    return vec3<T>(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z);
}

template<typename T>
inline vec3<T> operator / (const vec3<T>& lhs, T rhs)
{
    return vec3<T>(lhs.x / rhs, lhs.y / rhs, lhs.z / rhs);
}

template<typename T>
inline vec3<T> operator / (T lhs, const vec3<T>& rhs)
{
    return vec3<T>(lhs / rhs.x, lhs / rhs.y, lhs / rhs.z);
}

template<typename T>
inline vec3<T> operator + (const vec3<T>& lhs, const vec3<T>& rhs)
{
    return vec3<T>(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
}

template<typename T>
inline vec3<T> operator - (const vec3<T>& lhs, const vec3<T>& rhs)
{
    return vec3<T>(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
}

template<typename T>
T dot(vec2<T>& v, vec2<T>& u)
{
    return v.x * u.x + v.y * u.y;
}

template<typename T>
T dot(vec3<T>& v, vec3<T>& u)
{
    return v.x * u.x + v.y * u.y + v.z * u.z;
}

template<typename T>
T dot(vec4<T>& v, vec4<T>& u)
{
    return v.x * u.x + v.y * u.y + v.z * u.z + v.w * u.w;
}

template<typename T>
vec3<T> cross(vec3<T>& v, vec3<T>& u)
{
    return vec3<T>(v.y * u.z - v.z * u.y,
                   v.z * u.x - v.x * u.z,
                   v.x * u.y - v.y * u.x);
}


} // namespace Math
} // namespace Raptor