#pragma once

#include "../PlatformShared/platform_shared.h"
#include <vector>

// stands for geometry
namespace gmt {

	struct Polygon
	{
		std::vector<glm::vec3> vertices;
	};

	struct AABB
	{
		glm::vec3 min;
		glm::vec3 max;
	};

	struct Sphere
	{
		glm::vec3 center;
		float radius;
	};

	struct Plane
	{
		glm::vec3 normal;
		float dist;
	};

	struct Ray
	{
		glm::vec3 p;
		glm::vec3 dir;
	};

	struct Line
	{
		glm::vec3 p0;
		glm::vec3 p1;
	};
}
