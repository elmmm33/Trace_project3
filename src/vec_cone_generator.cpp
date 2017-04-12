#include "vecmath/vecmath.h"

#include "vec_cone_generator.h"

#ifndef M_PI
#define M_PI 3.141592653589793238462643383279502
#endif

VecConeGenerator::VecConeGenerator(const vec3f &center, const double x)
		: m_center(center),
		  m_x(x)
{
	const vec3f z_vec(0.0, 0.0, 1.0);
	const vec3f &v = z_vec.cross(center);
	const double sin = v.length();
	const double cos = z_vec.dot(center);
	mat3f vx(vec3f(0, -v[2], v[1]),
			vec3f(v[2], 0, -v[0]),
			vec3f(-v[1], v[0], 0));
	m_transform = mat3f() + vx + vx * vx * ((1 - cos) / (sin * sin));
}

vec3f VecConeGenerator::Generate() const
{
	const double x = rand() / (double)RAND_MAX * m_x;
	const double theta = rand() / (double)RAND_MAX * 2 * M_PI;
	vec3f v;
	v[2] = cos(x);
	const double r = sqrt(1 - v[2] * v[2]);
	v[0] = r * cos(theta);
	v[1] = r * sin(theta);
	return m_transform * v;
}
