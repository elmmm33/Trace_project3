// The main ray tracer.

#include <cmath>
#include <cstring>
#include <deque>

#include <Fl/fl_ask.H>

#include "RayTracer.h"
#include "scene/light.h"
#include "scene/material.h"
#include "scene/ray.h"
#include "fileio/read.h"
#include "fileio/parse.h"
#include "ui/TraceUI.h"
#include "global.h"
#include "vec_cone_generator.h"

using namespace std;

// Trace a top-level ray through normalized window coordinates (x,y)
// through the projection plane, and out into the scene.  All we do is
// enter the main ray-tracing method, getting things started by plugging
// in an initial ray weight of (0.0,0.0,0.0) and an initial recursion depth of 0.
vec3f RayTracer::trace( Scene *scene, double x, double y )
{
    ray r( vec3f(0,0,0), vec3f(0,0,0) );
    scene->getCamera()->rayThrough( x,y,r );

	TraceParam param;
	param.scene = scene;
	param.r = &r;
	param.thresh = vec3f(1.0, 1.0, 1.0);
	param.depth = 0;
	Material air;
	param.material_stack.push_front(&air);

	return traceRay(param).clamp();
}

// Do recursive ray tracing!  You'll want to insert a lot of code here
// (or places called from here) to handle reflection, refraction, etc etc.
vec3f RayTracer::traceRay(const TraceParam& param)
{
	if (param.thresh[0] <= traceUI->GetIntensityThreshold()
			&& param.thresh[1] <= traceUI->GetIntensityThreshold()
			&& param.thresh[2] <= traceUI->GetIntensityThreshold())
	{
		return vec3f();
	}

	isect i;
	if (param.scene->intersect(*param.r, i))
	{
		// An intersection occured!  We've got work to do.  For now,
		// this code gets the material for the surface that was intersected,
		// and asks that material to provide a color for the ray.

		// This is a great place to insert code for recursive ray tracing.
		// Instead of just returning the result of shade(), add some
		// more steps: add in the contributions from reflected and refracted
		// rays.

		if (IsLeavingObject(param, i))
		{
			// when we're leaving an object, the normal we get has to be
			// reversed
			i.N = -i.N;
		}

		const Material &m = i.getMaterial();
		const vec3f &shade = m.shade(param.scene, *param.r, i);
		const vec3f intensity = prod(shade, param.thresh);

		ReflectionParam reflect_param;
		reflect_param.i = &i;
		vec3f reflection = traceUI->IsEnableReflection()
				? traceReflection(param, reflect_param) : vec3f();

		RefractionParam refract_param;
		refract_param.i = &i;
		vec3f refraction = traceUI->IsEnableRefraction()
				? traceRefraction(param, refract_param) : vec3f();

		if (traceUI->IsEnableFresnel()
				&& (param.material_stack.front()->index != 1
					|| i.getMaterial().index != 1))
		{
			const double fresnel_coeff = GetFresnelCoeff(param, i);
			const double fresnel_ratio = traceUI->GetFresnelRatio();

			reflection = fresnel_ratio * fresnel_coeff * reflection
					+ (1 - fresnel_ratio) * reflection;
			refraction = fresnel_ratio * (1 - fresnel_coeff) * refraction
					+ (1 - fresnel_ratio) * refraction;
		}
		return intensity + reflection + refraction;
	}
	else
	{
		// No intersection.  This ray travels to infinity, so we color
		// it according to the background color, which in this (simple) case
		// is just black.

		return vec3f( 0.0, 0.0, 0.0 );
	}
}

vec3f RayTracer::traceReflection(const TraceParam& param,
		const ReflectionParam &rparam)
{
	const Material &m = rparam.i->getMaterial();
	if (m.kr.iszero() || param.depth >= traceUI->getDepth())
	{
		return vec3f();
	}

	// push the point outwards a bit so that the ray won't hit itself
	const vec3f &out_point = param.r->at(rparam.i->t) + rparam.i->N
			* RAY_EPSILON;
	const double dot_rn = rparam.i->N.dot(-param.r->getDirection());
	const vec3f &center_dir = (2.0 * dot_rn * rparam.i->N
			- -param.r->getDirection()).normalize();

	const int sample = traceUI->GetGlossyReflectionSample();
	if (sample == 0)
	{
		ray reflection_r(out_point, center_dir);

		TraceParam next_param;
		next_param.scene = scene;
		next_param.r = &reflection_r;
		next_param.thresh = prod(param.thresh, m.kr);
		next_param.depth = param.depth + 1;
		next_param.material_stack = param.material_stack;
		return traceRay(next_param);
	}
	else
	{
		vec3f intensity;
		VecConeGenerator vcg(center_dir, 0.1);
		for (int i = 0; i < sample; ++i)
		{
			const vec3f &dir = vcg.Generate();
			ray reflection_r(out_point, dir);

			TraceParam next_param;
			next_param.scene = scene;
			next_param.r = &reflection_r;
			next_param.thresh = prod(param.thresh, m.kr);
			next_param.depth = param.depth + 1;
			next_param.material_stack = param.material_stack;
			intensity += traceRay(next_param);
		}
		return intensity / sample;
	}
}

vec3f RayTracer::traceRefraction(const TraceParam &param,
		const RefractionParam &rparam)
{
	const Material &m = rparam.i->getMaterial();
	if (!m.kt.iszero() && param.depth < traceUI->getDepth())
	{
		deque<const Material*> mat_stack = param.material_stack;
		const Material *m2 = mat_stack.front();
		double ni, nt;
		vec3f push_point;
		if (IsLeavingObject(param, *rparam.i))
		{
			// leaving m
			const Material *outside = mat_stack[1];
			ni = m.index;
			nt = outside->index;
			mat_stack.pop_front();
		}
		else
		{
			// to something inside m2
			ni = m2->index;
			nt = m.index;
			mat_stack.push_front(&m);
		}
		const double nr = ni / nt;
		const double dot_rn = rparam.i->N.dot(-param.r->getDirection());
		// push the point inwards normal a bit so that the ray won't hit itself
		push_point = param.r->at(rparam.i->t) - rparam.i->N * RAY_EPSILON;

		const double root = 1 - nr * nr * (1 - dot_rn * dot_rn);
		if (root < 0.0)
		{
			// total internal reflection
			return vec3f();
		}
		else
		{
			const double coeff = nr * dot_rn - sqrt(root);
			const vec3f &refraction_dir = coeff * rparam.i->N - nr
					* -param.r->getDirection();
			ray refraction_r(push_point, refraction_dir);

			TraceParam next_param;
			next_param.scene = scene;
			next_param.r = &refraction_r;
			next_param.thresh = prod(param.thresh, m.kt);
			next_param.depth = param.depth + 1;
			next_param.material_stack = mat_stack;
			return traceRay(next_param);
		}
	}
	else
	{
		return vec3f();
	}
}

bool RayTracer::IsLeavingObject(const TraceParam& param, const isect &i) const
{
	return (&i.getMaterial() == param.material_stack.front());
}

double RayTracer::GetFresnelCoeff(const TraceParam& param, const isect &i) const
{
	// use the Schlick's approximation to calc the Fresnel coeff
	double ni, nt;
	if (IsLeavingObject(param, i))
	{
		ni = i.getMaterial().index;
		nt = param.material_stack.front()->index;
	}
	else
	{
		ni = param.material_stack.front()->index;
		nt = i.getMaterial().index;
	}
	double r0 = (ni - nt) / (ni + nt);
	r0 *= r0;
	const double dot_rn = i.N.dot(-param.r->getDirection());

	if (ni <= nt)
	{
		return r0 + (1 - r0) * pow(1 - dot_rn, 5);
	}
	else
	{
		const double nr = ni / nt;
		const double root = 1 - nr * nr * (1 - dot_rn * dot_rn);
		if (root < 0.0)
		{
			// TIR
			return 1.0;
		}
		else
		{
			const double cos_theta_t = sqrt(1 - (ni / nt));
			return r0 + (1 - r0) * pow(1 - cos_theta_t, 5);
		}
	}
}

RayTracer::RayTracer()
{
	buffer = NULL;
	buffer_width = buffer_height = 256;
	scene = NULL;

	m_bSceneLoaded = false;
}


RayTracer::~RayTracer()
{
	delete [] buffer;
	delete scene;
}

void RayTracer::getBuffer( unsigned char *&buf, int &w, int &h )
{
	buf = buffer;
	w = buffer_width;
	h = buffer_height;
}

double RayTracer::aspectRatio()
{
	return scene ? scene->getCamera()->getAspectRatio() : 1;
}

bool RayTracer::sceneLoaded()
{
	return m_bSceneLoaded;
}

bool RayTracer::loadScene( const char* fn )
{
	try
	{
		scene = readScene( fn );
	}
	catch( const ParseError &pe )
	{
		fl_alert( "ParseError: %s\n", pe.getMsg().c_str() );
		return false;
	}

	if( !scene )
		return false;

	buffer_width = 256;
	buffer_height = (int)(buffer_width / scene->getCamera()->getAspectRatio() + 0.5);

	bufferSize = buffer_width * buffer_height * 3;
	buffer = new unsigned char[ bufferSize ];

	// separate objects into bounded and unbounded
	scene->initScene();

	// Add any specialized scene loading code here

	m_bSceneLoaded = true;

	return true;
}

void RayTracer::traceSetup( int w, int h )
{
	if( buffer_width != w || buffer_height != h )
	{
		buffer_width = w;
		buffer_height = h;

		bufferSize = buffer_width * buffer_height * 3;
		delete [] buffer;
		buffer = new unsigned char[ bufferSize ];
	}
	memset( buffer, 0, w*h*3 );
}

void RayTracer::traceLines( int start, int stop )
{
	vec3f col;
	if( !scene )
		return;

	if( stop > buffer_height )
		stop = buffer_height;

	for( int j = start; j < stop; ++j )
		for( int i = 0; i < buffer_width; ++i )
			tracePixel(i,j);
}

void RayTracer::tracePixel( int i, int j )
{
	vec3f col;

	if( !scene )
		return;

	double x = double(i)/double(buffer_width);
	double y = double(j)/double(buffer_height);

	if (traceUI->GetSuperSampling() > 0)
	{
		const int sample = traceUI->GetSuperSampling();
		const double pixel_w = 1.0 / buffer_width;
		const double pixel_h = 1.0 / buffer_height;
		const double sub_pixel_w = pixel_w / sample;
		const double sub_pixel_h = pixel_h / sample;
		for (int i = 0; i < sample; ++i)
		{
			const double base_y = y + ((double)i / sample - 0.5) * pixel_h;
			for (int j = 0; j < sample; ++j)
			{
				const double base_x = x + ((double)j / sample - 0.5) * pixel_w;

				const double jitter_y = (rand() / (double)RAND_MAX - 0.5)
						* sub_pixel_h + base_y;
				const double jitter_x = (rand() / (double)RAND_MAX - 0.5)
						* sub_pixel_w + base_x;
				col += trace(scene, jitter_x, jitter_y);
			}
		}
		col /= sample * sample;
	}
	else
	{
		col = trace(scene, x, y);
	}

	unsigned char *pixel = buffer + ( i + j * buffer_width ) * 3;

	pixel[0] = (int)( 255.0 * col[0]);
	pixel[1] = (int)( 255.0 * col[1]);
	pixel[2] = (int)( 255.0 * col[2]);
}
