#pragma once

#include "../PlatformShared/platform_shared.h"
#include "nav_mesh.h"
#include "collision.h"

#include <queue>

namespace PathFinding
{
	NavMesh::NavMeshPolygon* GetPolygonContainingPoint(
		std::vector<NavMesh::NavMeshPolygon>& pathingEnvironment, 
		glm::vec3 point)
	{
		for (int i = 0; i < pathingEnvironment.size(); i++)
		{
			NavMesh::NavMeshPolygon* polygon = &pathingEnvironment[i];
			if (Collision::IsPointInsideTriangle2(
				point, 
				polygon->vertices[0], 
				polygon->vertices[1], 
				polygon->vertices[2]))
			{
				return polygon;
			}
		}
		return NULL;
	}

	struct PathfindingResult
	{
		bool valid;
		std::vector<glm::vec3> waypoints;
	};


	struct AStarSearchNode
	{
		glm::vec3 curPos;
		glm::vec3 fromPos;
		int cost;
	};

	class AStarSearchNodeComparison
	{
	public:
		// moderling the < operator. Determine who goes in front of the array.
		// min heap, min is gonna be at the end of the array
		// so whoever is larger goes in the front
		bool operator() (const AStarSearchNode& lhs, const AStarSearchNode& rhs) const
		{
			return lhs.cost > rhs.cost;
		}
	};

	void AStarSearch(NavMesh::DualGraph* dualGraph, NavMesh::NavMeshPolygon* triangle0, NavMesh::NavMeshPolygon* triangle1)
	{
		// from triangle 0, do a bfs with a distance heuristic until u reach triangle1

		std::priority_queue<AStarSearchNode, std::vector<AStarSearchNode>, AStarSearchNodeComparison> q;

		std::unordered_set<int> visited;

	//	q.push(triangle0)
	}



	PathfindingResult FindPath(std::vector<NavMesh::NavMeshPolygon>& pathingEnvironment, 
		NavMesh::DualGraph* dualGraph, glm::vec3 start, glm::vec3 goal)
	{
		PathfindingResult result;

		NavMesh::NavMeshPolygon* triangle0 = GetPolygonContainingPoint(pathingEnvironment, start);
		NavMesh::NavMeshPolygon* triangle1 = GetPolygonContainingPoint(pathingEnvironment, goal);

		if (triangle0 == NULL || triangle1 == NULL) {
			result.valid = false;
			return result;
		}

		result.valid = true;
		if (triangle0 == triangle1)
		{
			// return a straight line between them

			result.waypoints.push_back(start);
			result.waypoints.push_back(goal);
			return result;
		}
		else
		{

		}

	}
}