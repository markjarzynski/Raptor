#include "Vector.h"

namespace Raptor
{
namespace Math
{

template vec3<float>& vec3<float>::Normalize();
template vec3<double>& vec3<double>::Normalize();

template vec2<float>  operator + (float lhs, const vec2<float>& rhs);
template vec2<float>  operator - (float lhs, const vec2<float>& rhs);
template vec2<float>  operator * (float lhs, const vec2<float>& rhs);
template vec2<float>  operator / (float lhs, const vec2<float>& rhs);
template vec2<float>  operator + (const vec2<float>& lhs, float rhs);
template vec2<float>  operator - (const vec2<float>& lhs, float rhs);
template vec2<float>  operator * (const vec2<float>& lhs, float rhs);
template vec2<float>  operator / (const vec2<float>& lhs, float rhs);
template vec2<double> operator + (double lhs, const vec2<double>& rhs);
template vec2<double> operator - (double lhs, const vec2<double>& rhs);
template vec2<double> operator * (double lhs, const vec2<double>& rhs);
template vec2<double> operator / (double lhs, const vec2<double>& rhs);
template vec2<double> operator + (const vec2<double>& lhs, double rhs);
template vec2<double> operator - (const vec2<double>& lhs, double rhs);
template vec2<double> operator * (const vec2<double>& lhs, double rhs);
template vec2<double> operator / (const vec2<double>& lhs, double rhs);

template vec3<float>  operator + (float lhs, const vec3<float>& rhs);
template vec3<float>  operator - (float lhs, const vec3<float>& rhs);
template vec3<float>  operator * (float lhs, const vec3<float>& rhs);
template vec3<float>  operator / (float lhs, const vec3<float>& rhs);
template vec3<float>  operator + (const vec3<float>& lhs, float rhs);
template vec3<float>  operator - (const vec3<float>& lhs, float rhs);
template vec3<float>  operator * (const vec3<float>& lhs, float rhs);
template vec3<float>  operator / (const vec3<float>& lhs, float rhs);
template vec3<double> operator + (double lhs, const vec3<double>& rhs);
template vec3<double> operator - (double lhs, const vec3<double>& rhs);
template vec3<double> operator * (double lhs, const vec3<double>& rhs);
template vec3<double> operator / (double lhs, const vec3<double>& rhs);
template vec3<double> operator + (const vec3<double>& lhs, double rhs);
template vec3<double> operator - (const vec3<double>& lhs, double rhs);
template vec3<double> operator * (const vec3<double>& lhs, double rhs);
template vec3<double> operator / (const vec3<double>& lhs, double rhs);

template vec4<float>  operator + (float lhs, const vec4<float>& rhs);
template vec4<float>  operator - (float lhs, const vec4<float>& rhs);
template vec4<float>  operator * (float lhs, const vec4<float>& rhs);
template vec4<float>  operator / (float lhs, const vec4<float>& rhs);
template vec4<float>  operator + (const vec4<float>& lhs, float rhs);
template vec4<float>  operator - (const vec4<float>& lhs, float rhs);
template vec4<float>  operator * (const vec4<float>& lhs, float rhs);
template vec4<float>  operator / (const vec4<float>& lhs, float rhs);
template vec4<double> operator + (double lhs, const vec4<double>& rhs);
template vec4<double> operator - (double lhs, const vec4<double>& rhs);
template vec4<double> operator * (double lhs, const vec4<double>& rhs);
template vec4<double> operator / (double lhs, const vec4<double>& rhs);
template vec4<double> operator + (const vec4<double>& lhs, double rhs);
template vec4<double> operator - (const vec4<double>& lhs, double rhs);
template vec4<double> operator * (const vec4<double>& lhs, double rhs);
template vec4<double> operator / (const vec4<double>& lhs, double rhs);

template vec2<float>  operator + (const vec2<float>&  lhs, const vec2<float>&  rhs);
template vec2<float>  operator - (const vec2<float>&  lhs, const vec2<float>&  rhs);
template vec2<double> operator + (const vec2<double>& lhs, const vec2<double>& rhs);
template vec2<double> operator - (const vec2<double>& lhs, const vec2<double>& rhs);

template vec3<float>  operator + (const vec3<float>&  lhs, const vec3<float>&  rhs);
template vec3<float>  operator - (const vec3<float>&  lhs, const vec3<float>&  rhs);
template vec3<double> operator + (const vec3<double>& lhs, const vec3<double>& rhs);
template vec3<double> operator - (const vec3<double>& lhs, const vec3<double>& rhs);

template vec4<float>  operator + (const vec4<float>&  lhs, const vec4<float>&  rhs);
template vec4<float>  operator - (const vec4<float>&  lhs, const vec4<float>&  rhs);
template vec4<double> operator + (const vec4<double>& lhs, const vec4<double>& rhs);
template vec4<double> operator - (const vec4<double>& lhs, const vec4<double>& rhs);

template float dot(vec2<float>&, vec2<float>&);
template float dot(vec3<float>&, vec3<float>&);
template float dot(vec4<float>&, vec4<float>&);

template vec3<float> cross(vec3<float>&, vec3<float>&);



} // namespace Math
} // namespace Raptor