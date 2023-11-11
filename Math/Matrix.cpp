#include "Matrix.h"

namespace Raptor
{
namespace Math
{

template<typename T>
mat4<T>* mat4<T>::MakeIdentity()
{
    _11 = _22 = _33 = _44 = static_cast<T>(1);
    _12 = _13 = _14 = _21 = _23 = _24 = _31 = _32 = _34 = _41 = _42 = _43 = static_cast<T>(0);

    return *this;
}

template<typename T>
mat4<T>* mat4<T>::Zero()
{
    _11 = _12 = _13 = _14 = _21 = _22 = _23 = _24 = _31 = _32 = _33 = _34 = _41 = _42 = _43 = _44 = static_cast<T>(0);
    return this;
}

template<typename T>
mat4<T>* mat4<T>::FromQuaternion(const vec4<T> q)
{
    T qxx = q.x * q.x;
    T qyy = q.y * q.y;
    T qzz = q.z * q.z;

    T qxy = q.x * q.y;
    T qxz = q.x * q.z;
    T qyz = q.y * q.z;

    T qwx = q.w * q.x;
    T qwy = q.w * q.y;
    T qwz = q.w * q.z;
    

    _11 = 1 - 2 * qyy - 2 * qzz;
    _12 = 2 * qxy - 2 * qwz;
    _13 = 2 * qxz + 2 * qwy;
    _14 = 0;

    _21 = 2 * qxy + 2 * qwz;
    _22 = 1 - 2 * qxx - 2 * qzz;
    _23 = 2 * qyz - 2 * qwx;
    _24 = 0;

    _31 = 2 * qxz - 2 * qwy;
    _32 = 2 * qyz + 2 * qwx;
    _33 = 1 - 2 * qxx - 2 * qyy;
    _34 = 0;

    _41 = 0;
    _42 = 0;
    _43 = 0;
    _44 = 1;

    return this;
}

mat4f Transform::CalcMatrix()
{
    mat4f trans_mat;
    trans_mat._41 = translation.x;
    trans_mat._42 = translation.y;
    trans_mat._43 = translation.z;

    mat4f scale_mat;
    scale_mat._11 = scale.x;
    scale_mat._22 = scale.y;
    scale_mat._33 = scale.z;

    mat4f rot_mat;
    rot_mat.FromQuaternion(rotation);

    return trans_mat * rot_mat * scale_mat;
}

} // namespace Math
} // namespace Raptor