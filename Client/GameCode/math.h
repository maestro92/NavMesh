#pragma once

#include "../PlatformShared/platform_shared.h"
#include <math.h>       /* atan2 */
#include <sstream>
#include <algorithm>

#define PI 3.14159265


namespace Math
{
	float EPSILON = 1e-5;

	const float DEGREE_TO_RADIAN = 0.0174;    /// pi/180
	const float RADIAN_TO_DEGREE = 57.32;     /// 180/pi

	std::string IntToStr(int value)
	{
		std::stringstream ss;
		ss << value; // add number to string
		return ss.str();
	}

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

	bool Equals(float a, float b)
	{
		return IsZero(a - b);
	}

	float Cross(glm::vec2 v0, glm::vec2 v1)
	{
		return v0.x * v1.y - v1.x * v0.y;
	}

	inline float TriArea_XYPlane(const glm::vec3 d0, const glm::vec3 d1)
	{
		return d0.x * d1.y - d1.x * d0.y;
	}

	float Max(float a, float b, float c)
	{
		return std::max(std::max(a, b), c);
	}

	float Min(float a, float b, float c)
	{
		return std::min(std::min(a, b), c);
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
	float ComputeRotationAngle_XYPlane(glm::vec3 vec0, glm::vec3 vec1)
	{
		// the atan2 function return arctan y/x in the interval [-pi, +pi] radians
		double theta0 = atan2(vec0.y, vec0.x) * 180 / PI;
		double theta1 = atan2(vec1.y, vec1.x) * 180 / PI;

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

	glm::vec3 AngleToVector(float angle)
	{
		// -pi ~ pi
		return glm::vec3(cos(angle * PI / 180.0f), sin(angle * PI / 180.0f), 0);
	}

	float VectorToAngle(glm::vec3 vector)
	{
		return atan2(vector.y, vector.x) * 180 / PI;
	}

	glm::vec3 RotateVector(glm::vec3 vector, float rotationAngle, float& newAngle)
	{
		// optimize this 
		float angle = VectorToAngle(vector);
		newAngle = angle + rotationAngle;
		glm::vec3 newVector = AngleToVector(newAngle);
		return newVector;
	}

	float CalculateInteriorAngle(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2)
	{
		// orders matter


		glm::vec3 dPrev = glm::normalize(v1 - v0);
		glm::vec3 dNext = glm::normalize(v2 - v1);

		// the atan2 function return arctan y/x in the interval [-pi, +pi] radians
		double theta0 = atan2(dPrev.y, dPrev.x) * 180 / PI;
		double theta1 = atan2(dNext.y, dNext.x) * 180 / PI;

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

	bool IsObtuse(glm::vec3 vA, glm::vec3 vB, glm::vec3 vC)
	{
		float angleB = CalculateInteriorAngle(vA, vB, vC);
		return angleB > 90;
	}

	bool IsObtuseOrRight(glm::vec3 vA, glm::vec3 vB, glm::vec3 vC)
	{
		float angleB = CalculateInteriorAngle(vA, vB, vC);

		std::cout << "interior angle " << angleB << std::endl;

		return angleB >= 90;
	}

	bool IsAcute(glm::vec3 vA, glm::vec3 vB, glm::vec3 vC)
	{
		float angleB = CalculateInteriorAngle(vA, vB, vC);
		return angleB < 90;
	}

	void ClosestPointBetweenPointSegment(glm::vec3 point, glm::vec3 lineA, glm::vec3 lineB, float& t, glm::vec3& closestPoint)
	{
		glm::vec3 ab = lineB - lineA;

		t = glm::dot(point - lineA, ab) / glm::dot(ab, ab);

		if (t < 0.0f)
		{
			t = 0.0f;
		}
		if (t > 1.0f)
		{
			t = 1.0f;
		}
		closestPoint = lineA + t * ab;
	}
}