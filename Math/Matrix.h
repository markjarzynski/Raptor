#pragma once
#include "Vector.h"

namespace Raptor
{
namespace Math
{

template<typename T>
class mat3
{
public:
    mat3()
        : _11(1), _12(0), _13(0),
          _21(0), _22(1), _23(0),
          _31(0), _32(0), _33(1) {}
    
    mat3(vec3<T> v0, vec3<T> v1, vec3<T> v2) 
        : v[0](v0), v[1](v1), v[2](v2), v[3](v3) {}
    
    mat3(T* m)
    {
        for (uint8 i = 0; i < 9; i++)
        {
            this->i[i] = m[i];
        }
    }

    ~mat3(){}

    T Determinant();

    union
    {
        T i[9];
        T m[3][3];
        vec3<T> v[3];
        struct {T _11, _12, _13, _21, _22, _23, _31, _32, _33;};
    };

}; // class mat3

template<typename T>
class mat4
{
public:
    mat4()
        : _11(0), _12(0), _13(0), _14(0), 
          _21(0), _22(0), _23(0), _24(0), 
          _31(0), _32(0), _33(0), _34(0), 
          _41(0), _42(0), _43(0), _44(0) {}

    mat4(T a, T b, T c, T d, T e, T f, T g, T h, T i, T j, T k, T l, T m, T n, T o, T p)
        : _11(a), _12(b), _13(c), _14(d), 
          _21(e), _22(f), _23(g), _24(h), 
          _31(i), _32(j), _33(k), _34(l), 
          _41(m), _42(n), _43(o), _44(p) {}
    
    mat4(vec4<T> v0, vec4<T> v1, vec4<T> v2, vec4<T> v3) 
        : v[0](v0), v[1](v1), v[2](v2), v[3](v3) {}

    mat4(T* m)
    {
        for (uint8 i = 0; i < 16; i++)
        {
            this->i[i] = m[i];
        }
    }

    mat4(mat4& m)
    {
        for (uint8 i = 0; i < 16; i++)
        {
            this->i[i] = m.i[i];
        }
    }

    ~mat4() {}

    vec4<T> operator [] (int i) const { return v[i]; }
    vec4<T> &operator [] (int i) { return v[i]; }

    mat4<T>* Zero();
    mat4<T>* Identity();

    mat4<T>* Scale(const vec3<T>& scale);

    mat4<T> Transpose();
    mat4<T> Inverse();
    mat4<T> Adjugate();
    T Determinant();

    mat3<T>* SubMat3(uint32 i, uint32 j);

    mat4<T>* FromQuaternion(const vec4<T>& q);
    mat4<T>* FromPerspective(T fov, T aspect, T near_, T far_);

    mat4<T>* LookAt(vec3<T>& eye, vec3<T>& center, vec3<T>& up);

    mat4<T> operator * (const mat4<T>& rhs);

    union 
    {
        T i[16];
        T m[4][4];
        vec4<T> v[4];
        struct {T _11, _12, _13, _14, _21, _22, _23, _24, _31, _32, _33, _34, _41, _42, _43, _44;};
    };

}; // class mat4

template<typename T> mat4<T> operator * (const mat4<T>& lhs, T rhs);
template<typename T> mat4<T> operator * (T lhs, const mat4<T>& rhs);

typedef mat4<float> mat4f;
typedef mat4<double> mat4d;

class Transform
{
public:

    Transform(){}
    Transform(vec3f scale, vec4f rotation, vec3f translation) :
        scale(scale), rotation(rotation), translation(translation) {}
    ~Transform(){}

    mat4f CalcMatrix();

    vec3f scale;
    vec4f rotation;
    vec3f translation;

}; // class Transform


} // namespace Math
} // namespace Raptor