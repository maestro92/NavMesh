#pragma once

#include "../PlatformShared/platform_shared.h"
#include <math.h>       /* atan2 */

#define PI 3.14159265

namespace Math
{

	float EPSILON = 1e-5;


	bool Equals(glm::vec3 v0, glm::vec3 v1)
	{
		return abs(v0.x - v1.x) < EPSILON && abs(v0.y - v1.y) < EPSILON && abs(v0.z - v1.z) < EPSILON;
	}

	float IsZero(float num)
	{
		return abs(num) < EPSILON;
	}

	float Cross(glm::vec2 v0, glm::vec2 v1)
	{
		return v0.x * v1.y - v1.x * v0.y;
	}

	float ConvertAngleTo0To360(float angle)
	{
		angle = fmod(angle, 360);
		if (angle < 0)
			angle += 360;
		return angle;
	}

	// angle needed to get form vector 0 to vector 1
	// gives an angle [0 - 360]
	float ComputeRotationAngle_XZPlane(glm::vec3 vec0, glm::vec3 vec1)
	{
		// the atan2 function return arctan y/x in the interval [-pi, +pi] radians
		double theta0 = atan2(vec0.z, vec0.x) * 180 / PI;
		double theta1 = atan2(vec1.z, vec1.x) * 180 / PI;

	//	std::cout << "			theta0 " << theta0 << std::endl;
	//	std::cout << "			theta1 " << theta1 << std::endl;

		// handle angle wrap around
		float diff = theta0 - theta1;
		/*
		while (angle < 360)
		{
			angle += 360;
		}
		*/
	//	std::cout << "			angle " << angle << std::endl;
		diff = fmod(diff, 360);
		if (diff < 0)
			diff += 360;

		diff = 360 - diff;
	//	std::cout << "			angle " << angle << std::endl;

		return diff;
	}
}