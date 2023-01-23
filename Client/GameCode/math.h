#pragma once

#include "../PlatformShared/platform_shared.h"
#include <math.h>       /* atan2 */

#define PI 3.14159265


namespace Math
{
	float EPSILON = 1e-5;

	std::ostream& operator<<(std::ostream& os, const glm::vec3& vec)
	{
		os << vec.x << " " << vec.y << " " << vec.z;
		return os;
	}

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

	inline float TriArea_ZXPlane(const glm::vec3 d0, const glm::vec3 d1)
	{
		return d0.z * d1.x - d1.z * d0.x;
	}

	float ConvertAngleTo0To360(float angle)
	{
		angle = fmod(angle, 360);
		if (angle < 0)
			angle += 360;
		return angle;
	}

	float ConvertAngleToN180ToP180(float angle)
	{
		angle = fmod(angle + 180, 360);
		if (angle < 0)
			angle += 360;
		return angle - 180;
	}


	// angle needed to get form vector 0 to vector 1
	// gives an counter-clockwise angle [0 - 360]
	float ComputeRotationAngle_XZPlane(glm::vec3 vec0, glm::vec3 vec1)
	{
		// the atan2 function return arctan y/x in the interval [-pi, +pi] radians
		double theta0 = atan2(vec0.x, vec0.z) * 180 / PI;
		double theta1 = atan2(vec1.x, vec1.z) * 180 / PI;

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


	float CalculateInteriorAngle(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2)
	{
		// orders matter


		glm::vec3 dPrev = glm::normalize(v1 - v0);
		glm::vec3 dNext = glm::normalize(v2 - v1);

		// the atan2 function return arctan y/x in the interval [-pi, +pi] radians
		double theta0 = atan2(dPrev.x, dPrev.z) * 180 / PI;
		double theta1 = atan2(dNext.x, dNext.z) * 180 / PI;

		// handle angle wrap around

		/*
			the idea is that
			theta0 is in -pi to pi
			theta1 is in -pi to pi
			when you calculate theta0 - theta1
			it can be -2pi to 2pi
			first we add 180, so that it is in
			-pi to 3pi
			then we add 360 to make -pi disappear.
			cuz if you think about it, when you have a angle in -pi to pi
			the mapping is
					angle = angle + 360
					if angle > 360
						angle -= 360
			this works cuz
			if our angle is negative
				adding 360 converts your angle (if it is negative) to positive.

			if our angle is possitive
				adding 360 is still the samething. The subsequent if statement clamps it in the [0, 360] range
			so same thing here, once our angle is in -pi to 3pi range
			we do the samething
			*/
		float diff = theta0 - theta1;
		float angle = (180 + diff + 360);
		while (angle > 360)
		{
			angle -= 360;
		}
		return angle;
	}

}