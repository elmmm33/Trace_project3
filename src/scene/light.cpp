#include <cmath>
#include <cstdlib>
#include <algorithm>

#include "../ui/TraceUI.h"
#include "../global.h"
#include "light.h"

double DirectionalLight::distanceAttenuation( const vec3f& ) const
{
	// distance to light is infinite, so f(di) goes to 0.  Return 1.
	return 1.0;
}


vec3f DirectionalLight::shadowAttenuation( const vec3f& P ) const
{
    const vec3f &dir = getDirection(P);
    vec3f result(1.0, 1.0, 1.0);
	// push the point outwards a bit so that the ray won't hit itself
    vec3f point = P + dir * RAY_EPSILON;
    while (!result.iszero())
    {
        isect i;
        ray shadow_ray(point, dir);
        if (!scene->intersect(shadow_ray, i))
        {
            return result;
        }

        result = prod(result, i.getMaterial().kt);
        // slightly push the point forward to prevent hitting itself
        point = shadow_ray.at(i.t) + dir * RAY_EPSILON;
    }
    return result;
}

vec3f DirectionalLight::getColor( const vec3f& ) const
{
	// Color doesn't depend on P
	return color;
}

vec3f DirectionalLight::getDirection( const vec3f& ) const
{
	return -orientation;
}

PointLight::PointLight( Scene *scene, const vec3f& pos, const vec3f& color )
		: Light( scene, color ),
		  position( pos ),
		  constant_attenuation_coeff(0.0),
		  linear_attenuation_coeff(0.0),
		  quadratic_attenuation_coeff(0.0)
{}

double PointLight::distanceAttenuation( const vec3f& P ) const
{
	const double d2 = (P - position).length_squared();
	const double d = sqrt(d2);
	double divisor;
	if (traceUI->IsOverideDistance())
	{
		divisor = traceUI->GetDistanceConstant();
		divisor += traceUI->GetDistanceLinear() * d;
		divisor += traceUI->GetDistanceQuadratic() * d2;
	}
	else
	{
		divisor = constant_attenuation_coeff;
		divisor += linear_attenuation_coeff * d;
		divisor += quadratic_attenuation_coeff * d2;
	}
	return (divisor == 0.0) ? 1.0 : 1.0 / std::max<double>(divisor, 1.0);
}

vec3f PointLight::getColor( const vec3f& ) const
{
	// Color doesn't depend on P
	return color;
}

vec3f PointLight::getDirection( const vec3f& P ) const
{
	return (position - P).normalize();
}


vec3f PointLight::shadowAttenuation(const vec3f& P) const
{
	if (traceUI->IsEnableSoftShadow())
	{
		vec3f result(1.0, 1.0, 1.0);
		const double extend = 0.2;
		const unsigned kNumRays = 3;
		vec3f new_pos;
		for (unsigned i = 0; i < kNumRays; ++i)
		{
			new_pos[0] = position[0] + extend * ((double)i / kNumRays - 0.5);
			for (unsigned j = 0; j < kNumRays; ++j)
			{
				new_pos[1] = position[1] + extend * ((double)j / kNumRays - 0.5);
				for (unsigned k = 0; k < kNumRays; ++k)
				{
					new_pos[2] = position[2] + extend * ((double)k / kNumRays - 0.5);
					result += shadowAttenuation_(P, (new_pos - P).normalize());
				}
			}
		}
		return result / (kNumRays * kNumRays * kNumRays);
	}
	else
	{
		return shadowAttenuation_(P, getDirection(P));
	}
}

vec3f PointLight::shadowAttenuation_(const vec3f &P, const vec3f &dir) const
{
	// Shoot a shadow ray at the intersecion point towards this light source, in
	// case any object is intersected, ignore this light
    vec3f result(1.0, 1.0, 1.0);
	// push the point outwards a bit so that the ray won't hit itself
    vec3f point = P + dir * RAY_EPSILON;
    while (!result.iszero())
    {
        isect i;
        ray shadow_ray(point, dir);
    	const double light_t = (position - point).length();
        if (!scene->intersect(shadow_ray, i) || i.t >= light_t)
        {
        	// if no intersection or the object is behind the light
            return result;
        }

        result = prod(result, i.getMaterial().kt);
        // slightly push the point forward to prevent hitting itself
        point = shadow_ray.at(i.t) + dir * RAY_EPSILON;
    }
    return result;
}

void PointLight::setDistanceAttenuation(const double constant,
		const double linear, const double quadratic)
{
	constant_attenuation_coeff = constant;
	linear_attenuation_coeff = linear;
	quadratic_attenuation_coeff = quadratic;
}

vec3f AmbientLight::getColor( const vec3f& ) const
{
	// Color doesn't depend on P
	return color;
}
