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
}