#include <cmath>
#include <assert.h>
#include <algorithm>
#include <limits>

#include "Box.h"

bool Box::intersectLocal( const ray& r, isect& i ) const
{
	double tfar = std::numeric_limits<double>::max();
	double tnear = -std::numeric_limits<double>::max();
	int tnear_axis = 0;

	for (int i = 0; i < 3; ++i)
	{
		if (r.getDirection()[i] == 0)
		{
			// parallel to plane
			if (r.getPosition()[i] < -0.5 || r.getPosition()[i] > 0.5)
			{
				return false;
			}
		}

		double t1 = (-0.5 - r.getPosition()[i]) / r.getDirection()[i];
		double t2 = (0.5 - r.getPosition()[i]) / r.getDirection()[i];
		if (t1 > t2)
		{
			std::swap(t1, t2);
		}
		if (t1 > tnear)
		{
			tnear = t1;
			tnear_axis = i;
		}
		if (t2 < tfar)
		{
			tfar = t2;
		}
		if (tnear > tfar || tfar < 0)
		{
			// missed || behind box
			return false;
		}
	}

	i.obj = this;
	i.t = tnear;
	switch (tnear_axis)
	{
	default:
		printf("Box::intersectLocal ???\n");
	case 0:
		i.N = vec3f((r.getDirection()[0] < 0.0) ? 1.0 : -1.0, 0.0, 0.0);
		break;

	case 1:
		i.N = vec3f(0.0, (r.getDirection()[1] < 0.0) ? 1.0 : -1.0, 0.0);
		break;

	case 2:
		i.N = vec3f(0.0, 0.0, (r.getDirection()[2] < 0.0) ? 1.0 : -1.0);
		break;
	}
	return true;
}
