#pragma once

#include "../PlatformShared/platform_shared.h"
#include <vector>
#include "nav_mesh.h"

namespace PathFinding
{
	struct DebugState
	{
		glm::vec3 start;
		glm::vec3 end;

		int startNodeId;
		int endNodeId;

		NavMesh::DualGraph* dualGraph;
		std::vector<glm::vec3> waypoints;
	};
}
