#include "Matrix.h"
#include "Matrix.inl"
#include <math.h>

namespace Raptor
{
namespace Math
{

template<typename T>
T mat3<T>::Determinant()
{
    return _11 * (_22*_33 - _23*_32) -
           _21 * (_12*_33 - _13*_32) +
           _31 * (_12*_23 - _13*_22);
}

template<typename T>
mat4<T>* mat4<T>::Zero()
{
    _11 = _12 = _13 = _14 = _21 = _22 = _23 = _24 = _31 = _32 = _33 = _34 = _41 = _42 = _43 = _44 = static_cast<T>(0);
    return this;
}

template<typename T>
mat4<T>* mat4<T>::Identity()
{
    _11 = _22 = _33 = _44 = static_cast<T>(1);
    _12 = _13 = _14 = _21 = _23 = _24 = _31 = _32 = _34 = _41 = _42 = _43 = static_cast<T>(0);

    return this;
}

template<typename T>
mat4<T>* mat4<T>::Scale(const vec3<T>& scale)
{
    _11 *= scale.x;
    _22 *= scale.y;
    _33 *= scale.z;
    return this;
}

template<typename T>
mat4<T> mat4<T>::Transpose()
{
    mat4<T> transpose {_11, _21, _31, _41, _12, _22, _32, _42, _13, _23, _33, _43, _14, _24, _34, _44};
    return transpose;
}

template<typename T>
mat4<T> mat4<T>::Inverse()
{
    T det = Determinant();
    mat4<T> adjugate = Adjugate();

    return adjugate * (1.f / det);
}

template<typename T>
T mat4<T>::Determinant()
{
    return _11 * (_22*_33*_44 + _23*_34*_42 + _24*_32*_43 - _24*_33*_42 - _23*_32*_44 - _22*_34*_43) -
           _21 * (_12*_33*_44 + _13*_34*_42 + _14*_32*_43 - _13*_33*_42 - _13*_32*_44 - _12*_34*_43) +
           _31 * (_12*_23*_44 + _13*_24*_42 + _14*_22*_43 - _14*_23*_42 - _13*_22*_44 - _12*_24*_43) -
           _41 * (_12*_23*_34 + _13*_24*_32 + _14*_22*_33 - _14*_23*_32 - _13*_22*_34 - _12*_24*_33);
}

template<typename T>
mat4<T> mat4<T>::Adjugate()
{
    mat4<T> out;// {};

    out._11 = SubMat3(1,1)->Determinant();
    out._12 = SubMat3(1,2)->Determinant();
    out._13 = SubMat3(1,3)->Determinant();
    out._14 = SubMat3(1,4)->Determinant();

    out._21 = SubMat3(2,1)->Determinant();
    out._22 = SubMat3(2,2)->Determinant();
    out._23 = SubMat3(2,3)->Determinant();
    out._24 = SubMat3(2,4)->Determinant();

    out._31 = SubMat3(3,1)->Determinant();
    out._32 = SubMat3(3,2)->Determinant();
    out._33 = SubMat3(3,3)->Determinant();
    out._34 = SubMat3(3,4)->Determinant();

    out._41 = SubMat3(4,1)->Determinant();
    out._42 = SubMat3(4,2)->Determinant();
    out._43 = SubMat3(4,3)->Determinant();
    out._44 = SubMat3(4,4)->Determinant();

    return out;
}

// Returns a 3x3 Matrix with the i row and j column removed.
template<typename T>
mat3<T>* mat4<T>::SubMat3(uint32 i, uint32 j)
{
    mat3<T>* out = new mat3<T>();

    for (uint32 m = 0, s = 0; m < 4; m++)
    {
        if (m + 1 != i)
        {
            for (uint32 n = 0, t = 0; n < 4; n++)
            {
                if (n + 1 != j)
                {
                    out->m[s][t] = this->m[m][n];
                    t++;
                }
            }
            s++;
        }
    }

    return out;
}


template<typename T>
mat4<T>* mat4<T>::FromQuaternion(const vec4<T>& q)
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

template<typename T>
mat4<T>* mat4<T>::FromPerspective(T fov, T aspect, T near, T far)
{
    Zero();

    float c = 1.f / tanf(fov * 0.5f);
    float fn = 1.f / (near - far);

    _11 = c / aspect;
    _22 = c;
    _33 = -far * fn;
    _34 = 1.f;
    _43 = near * far * fn;

    return this;
}

template<typename T>
mat4<T>* mat4<T>::LookAt(vec3<T>& eye, vec3<T>& center, vec3<T>& up)
{
    Identity();

    vec3<T>& c = center - eye;
    c.Normalize();

    vec3<T>& a = cross(c, up);
    vec3<T>& b = cross(a, c);

    _11 = a.x;
    _12 = b.x;
    _13 = c.x;

    _21 = a.y;
    _22 = b.y;
    _23 = c.y;

    _31 = a.z;
    _32 = b.z;
    _33 = c.z;
    
    _41 = -dot(a, eye);
    _42 = -dot(b, eye);
    _43 = -dot(c, eye);

    return this;
}

template<typename T>
mat4<T> mat4<T>::operator * (const mat4<T>& rhs)
{
    mat4<T> result;

    result[0][0] = this->m[0][0] * rhs.m[0][0] + this->m[1][0] * rhs.m[0][1] + this->m[2][0] * rhs.m[0][2] + this->m[3][0] * rhs.m[0][3];
    result[0][1] = this->m[0][1] * rhs.m[0][0] + this->m[1][1] * rhs.m[0][1] + this->m[2][1] * rhs.m[0][2] + this->m[3][1] * rhs.m[0][3];
    result[0][2] = this->m[0][2] * rhs.m[0][0] + this->m[1][2] * rhs.m[0][1] + this->m[2][2] * rhs.m[0][2] + this->m[3][2] * rhs.m[0][3];
    result[0][3] = this->m[0][3] * rhs.m[0][0] + this->m[1][3] * rhs.m[0][1] + this->m[2][3] * rhs.m[0][2] + this->m[3][3] * rhs.m[0][3];
    result[1][0] = this->m[0][0] * rhs.m[1][0] + this->m[1][0] * rhs.m[1][1] + this->m[2][0] * rhs.m[1][2] + this->m[3][0] * rhs.m[1][3];
    result[1][1] = this->m[0][1] * rhs.m[1][0] + this->m[1][1] * rhs.m[1][1] + this->m[2][1] * rhs.m[1][2] + this->m[3][1] * rhs.m[1][3];
    result[1][2] = this->m[0][2] * rhs.m[1][0] + this->m[1][2] * rhs.m[1][1] + this->m[2][2] * rhs.m[1][2] + this->m[3][2] * rhs.m[1][3];
    result[1][3] = this->m[0][3] * rhs.m[1][0] + this->m[1][3] * rhs.m[1][1] + this->m[2][3] * rhs.m[1][2] + this->m[3][3] * rhs.m[1][3];
    result[2][0] = this->m[0][0] * rhs.m[2][0] + this->m[1][0] * rhs.m[2][1] + this->m[2][0] * rhs.m[2][2] + this->m[3][0] * rhs.m[2][3];
    result[2][1] = this->m[0][1] * rhs.m[2][0] + this->m[1][1] * rhs.m[2][1] + this->m[2][1] * rhs.m[2][2] + this->m[3][1] * rhs.m[2][3];
    result[2][2] = this->m[0][2] * rhs.m[2][0] + this->m[1][2] * rhs.m[2][1] + this->m[2][2] * rhs.m[2][2] + this->m[3][2] * rhs.m[2][3];
    result[2][3] = this->m[0][3] * rhs.m[2][0] + this->m[1][3] * rhs.m[2][1] + this->m[2][3] * rhs.m[2][2] + this->m[3][3] * rhs.m[2][3];
    result[3][0] = this->m[0][0] * rhs.m[3][0] + this->m[1][0] * rhs.m[3][1] + this->m[2][0] * rhs.m[3][2] + this->m[3][0] * rhs.m[3][3];
    result[3][1] = this->m[0][1] * rhs.m[3][0] + this->m[1][1] * rhs.m[3][1] + this->m[2][1] * rhs.m[3][2] + this->m[3][1] * rhs.m[3][3];
    result[3][2] = this->m[0][2] * rhs.m[3][0] + this->m[1][2] * rhs.m[3][1] + this->m[2][2] * rhs.m[3][2] + this->m[3][2] * rhs.m[3][3];
    result[3][3] = this->m[0][3] * rhs.m[3][0] + this->m[1][3] * rhs.m[3][1] + this->m[2][3] * rhs.m[3][2] + this->m[3][3] * rhs.m[3][3];

    /*
    for (uint32 i = 0; i < 4; i++)
    {
        for (uint32 j = 0; j < 4; j++)
        {
            result[i][j] = 0;
            for (uint32 k = 0; k < 4; k++)
            {
                result[i][j] += this->m[i][k] * rhs.m[k][j];
            }
        }
    }
    */

    return result;
}

template<typename T>
mat4<T> operator * (const mat4<T>& lhs, T rhs)
{
    return mat4<T>(
        lhs.i[0]  * rhs,
        lhs.i[1]  * rhs,
        lhs.i[2]  * rhs,
        lhs.i[3]  * rhs,
        lhs.i[4]  * rhs,
        lhs.i[5]  * rhs,
        lhs.i[6]  * rhs,
        lhs.i[7]  * rhs,
        lhs.i[8]  * rhs,
        lhs.i[9]  * rhs,
        lhs.i[10] * rhs,
        lhs.i[11] * rhs,
        lhs.i[12] * rhs,
        lhs.i[13] * rhs,
        lhs.i[14] * rhs,
        lhs.i[15] * rhs);
}

template<typename T>
mat4<T> operator * (T lhs, const mat4<T>& rhs)
{
    return mat4<T>(
        lhs * rhs.i[0],
        lhs * rhs.i[1],
        lhs * rhs.i[2],
        lhs * rhs.i[3],
        lhs * rhs.i[4],
        lhs * rhs.i[5],
        lhs * rhs.i[6],
        lhs * rhs.i[7],
        lhs * rhs.i[8],
        lhs * rhs.i[9],
        lhs * rhs.i[10],
        lhs * rhs.i[11],
        lhs * rhs.i[12],
        lhs * rhs.i[13],
        lhs * rhs.i[14],
        lhs * rhs.i[15]);
}


mat4f Transform::CalcMatrix()
{
    mat4f trans_mat; trans_mat.Identity();
    trans_mat._41 = translation.x;
    trans_mat._42 = translation.y;
    trans_mat._43 = translation.z;

    mat4f scale_mat; scale_mat.Identity();
    scale_mat._11 = scale.x;
    scale_mat._22 = scale.y;
    scale_mat._33 = scale.z;

    mat4f rot_mat; rot_mat.Identity();
    rot_mat.FromQuaternion(rotation);

    return trans_mat * rot_mat * scale_mat;
}

} // namespace Math
} // namespace Raptor