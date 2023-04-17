#pragma once

#include "../PlatformShared/platform_shared.h"
#include <vector>
#include "nav_mesh.h"
#include "cd_triangulation.h"

namespace PathFinding
{
	struct DebugState
	{
		glm::vec3 start;
		bool hasSetStartPos;

		glm::vec3 end;
		bool hasSetEndPos;

		int startNodeId;
		int endNodeId;

		NavMesh::DualGraph* dualGraph;
		std::vector<glm::vec3> waypoints;
		std::vector<CDTriangulation::DelaunayTriangleEdge> portals;
	};
}
