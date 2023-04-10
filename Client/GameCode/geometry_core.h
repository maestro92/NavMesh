#pragma once

#include "../PlatformShared/platform_shared.h"
#include <vector>

namespace GeoCore {

	struct Polygon
	{
		std::vector<glm::vec3> vertices;
	};

}
