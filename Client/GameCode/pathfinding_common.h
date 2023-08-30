#pragma once

#include "../PlatformShared/platform_shared.h"
#include <vector>
#include "nav_mesh.h"
#include "cd_triangulation.h"

namespace PathFinding
{
	struct Vector
	{
		glm::vec3 v0;
		glm::vec3 v1;
	};

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
		std::vector<NavMesh::Portal> portals;
		std::vector<NavMesh::Portal> modifiedPortals;

		std::vector<glm::vec3> newLeftVertices;
		std::vector<glm::vec3> newRightVertices;

		std::vector<Vector> leftAnePerp;
		std::vector<Vector> rightAnePerp;

	};
}
