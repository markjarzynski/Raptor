#pragma once

#include "Types.h"

namespace Raptor
{
namespace Math
{

template<typename T>
class vec2
{
public:
    vec2() : x(0), y(0) {}
    vec2(T x, T y) : x(x), y(y) {}
    vec2(vec2& v) : x(v.x), y(v.y) {}
    ~vec2() {}

    union
    {
        T v[2];
        struct {T x, y; };
    };
    
}; // class vec2

template<typename T>
class vec3
{
public:
    vec3() : x(0), y(0), z(0) {}
    vec3(T x, T y, T z) : x(x), y(y), z(z) {}
    vec3(vec3& v) : x(v.x), y(v.y), z(v.z) {}
    ~vec3() {}

    union
    {
        T v[3];
        struct { T x, y, z; };
    };

    vec3<T> operator + (const vec3& rhs) const
    {
        vec3<T> result;
        result.x = this->x + rhs.x;
        result.y = this->y + rhs.y;
        result.z = this->z + rhs.z;

        return result;
    }

}; // class vec3

template<typename T>
class vec4
{
public:
    vec4() : x(0), y(0), z(0), w(0) {}
    vec4(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}
    vec4(vec4& v) : x(v.x), y(v.y), z(v.z), w(v.w) {}
    ~vec4() {}

    union
    {
        T v[4];
        struct { T x, y, z, w; };
    };
}; // class vec4

template<typename T>
T dot(vec2<T> v, vec2<T> u);

template<typename T>
T dot(vec3<T> v, vec3<T> u);

template<typename T>
T dot(vec4<T> v, vec4<T> u);

template<typename T>
vec3<T>* cross(vec3<T> v, vec3<T> u);

typedef vec2<int32> vec2i;
typedef vec2<uint32> vec2ui;
typedef vec2<short> vec2s;
typedef vec2<float> vec2f;
typedef vec2<double> vec2d;

typedef vec3<int32> vec3i;
typedef vec3<uint32> vec3ui;
typedef vec3<short> vec3s;
typedef vec3<float> vec3f;
typedef vec3<double> vec3d;

typedef vec4<int32> vec4i;
typedef vec4<uint32> vec4ui;
typedef vec4<short> vec4s;
typedef vec4<float> vec4f;
typedef vec4<double> vec4d;

} // namespace Math
} // namespace Raptor