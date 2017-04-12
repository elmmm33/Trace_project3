#ifndef VEC_CONE_GENERATOR_H_
#define VEC_CONE_GENERATOR_H_

#include "vecmath/vecmath.h"

class VecConeGenerator
{
public:
	VecConeGenerator(const vec3f &center, const double x);

	vec3f Generate() const;

private:
	vec3f m_center;
	double m_x;
	mat3f m_transform;
};

#endif /* VEC_CONE_GENERATOR_H_ */
