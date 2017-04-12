#include <algorithm>
#include "ray.h"
#include "material.h"
#include "light.h"
#include "../ui/TraceUI.h"
#include "../global.h"

namespace
{

vec3f GetAmibientLightsIntensity(Scene *scene, const vec3f &point)
{
	vec3f result;
	for (auto *l : scene->GetAmbientLights())
	{
		result += l->getColor(point);
	}
	return result;
}

}

// Apply the phong model to this point on the surface of the object, returning
// the color of that point.
vec3f Material::shade( Scene *scene, const ray& r, const isect& i ) const
{
	const vec3f &point = r.at(i.t);

	vec3f result = ke;
	const vec3f &ambient_i = GetAmibientLightsIntensity(scene, point);
	result += prod(prod(ka, ambient_i), 1.0 - kt);

	for (auto *l : scene->GetLights())
	{
		const double dot_ln = i.N.dot(l->getDirection(point));
		if (dot_ln <= 0.0)
		{
			continue;
		}

		const vec3f &shadow_attenuation = traceUI->IsEnableShadow()
				? l->shadowAttenuation(point) : vec3f(1.0, 1.0, 1.0);
		if (shadow_attenuation.iszero())
		{
			continue;
		}
		const vec3f &attenuation = shadow_attenuation
				* l->distanceAttenuation(point);
		const vec3f &light_i = l->getColor(point);
		const vec3f diffuse = prod(kd * dot_ln, 1.0 - kt);

		const vec3f &reflection = (2.0 * dot_ln * i.N - l->getDirection(point))
				.normalize();
		const double dot_rv = std::max<double>(reflection.dot(-r.getDirection()),
				0.0);
		const double specular_coeff = pow(dot_rv, shininess * 128);
		const vec3f specular = ks * specular_coeff;

		const vec3f intensity_coeff = diffuse + specular;

		result += prod(prod(attenuation, light_i), intensity_coeff);
	}

	return result;
}
