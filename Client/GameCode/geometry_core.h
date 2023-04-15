#pragma once

#include "../PlatformShared/platform_shared.h"
#include <vector>

namespace GeoCore {

	struct Polygon
	{
		std::vector<glm::vec3> vertices;
	};

	struct AABB
	{
		glm::vec3 min;
		glm::vec3 max;
	};

}
