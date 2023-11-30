#include "Matrix.h"
#include "Vector.h"

namespace Raptor
{
namespace Math
{

template mat4<float> mat4<float>::operator * (const mat4<float>& rhs);
template vec3<float> operator * (mat3<float>& lhs, const vec3<float>& rhs);


template mat3<float>* mat3<float>::Zero();
template mat3<float>* mat3<float>::Identity();

template mat4<float>* mat4<float>::Zero();
template mat4<float>* mat4<float>::Identity();

template mat4<float>* mat4<float>::Scale(const vec3<float>& scale);

template mat4<float> mat4<float>::Transpose();
template mat4<float> mat4<float>::Inverse();
template mat4<float> mat4<float>::Adjugate();
template float mat4<float>::Determinant();

template mat4<float>* mat4<float>::FromQuaternion(const vec4<float>& q);
template mat4<float>* mat4<float>::FromPerspective(float fov, float aspect, float near_, float far_);

template mat4<float>* mat4<float>::LookAt(vec3<float>& eye, vec3<float>& center, vec3<float>& up);

} // namespace Math
} // namespace Raptor