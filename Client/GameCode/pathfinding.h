#pragma once

#include "../PlatformShared/platform_shared.h"
#include "nav_mesh.h"
#include "pathfinding_common.h"
#include "world.h"

#include "collision.h"

#include <queue>
#include <unordered_map>

namespace PathFinding
{
	NavMesh::DualGraphNode* GetNodeContainingPoint(
		NavMesh::DualGraph* pathingEnvironment,
		World* world,
		glm::vec3 point)
	{
		CDTriangulation::DelaunayTriangle* triangle = world->FindTriangleBySimPos(point);
		if (triangle == NULL)
		{
			return NULL;
		}
		else
		{
			pathingEnvironment->GetNode(triangle->id);
		}
	}

	struct PathfindingResult
	{
		bool valid;
		std::vector<glm::vec3> waypoints;
		std::vector<CDTriangulation::DelaunayTriangleEdge> portals;
	};


	struct AStarSearchNode
	{
		int polygonNodeId;

		// this is to remeber the path
		int fromPolygonNodeId;
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

	// play around with this formula
	// in the paper: https://liu.diva-portal.org/smash/get/diva2:1560399/FULLTEXT01.pdf
	// it says it's only using Euclidean distance between current node and goal node
	float HeuristicCost(glm::vec3 pos0, glm::vec3 pos1)
	{
		return glm::distance2(pos0, pos1);
	}

	std::vector<int> AStarSearch(NavMesh::DualGraph* dualGraph, NavMesh::DualGraphNode* polygonNode0, NavMesh::DualGraphNode* polygonNode1)
	{
		std::cout << "start" << polygonNode0->GetId() << std::endl;
		std::cout << "end" << polygonNode1->GetId() << std::endl;


		// from triangle 0, do a bfs with a distance heuristic until u reach triangle1
		std::priority_queue<AStarSearchNode, std::vector<AStarSearchNode>, AStarSearchNodeComparison> q;

		std::unordered_map<int, AStarSearchNode> visited;

		AStarSearchNode node = { polygonNode0->GetId(), -1, 0};
		q.push(node);
	

		AStarSearchNode destinationNode;
		while (!q.empty())
		{
			AStarSearchNode curNode = q.top();
			q.pop();

			visited[curNode.polygonNodeId] = curNode;

			if (curNode.polygonNodeId == polygonNode1->GetId())
			{
				destinationNode = curNode;
				break;
			}

			int curPolygonId = curNode.polygonNodeId;
			NavMesh::DualGraphNode* polygonNode = dualGraph->GetNode(curPolygonId);

		//	std::cout << ">>>>> visiting " << curPolygonId << std::endl;

			glm::vec3 fromPos = polygonNode->center;

			// go through neighbors

			CDTriangulation::DelaunayTriangle* triangle = polygonNode->triangle;
			for (int i = 0; i < ArrayCount(triangle->neighbors); i++)
			{
				int neighborId = triangle->neighbors[i];
			//	std::cout << "		neighborId " << neighborId << std::endl;

				if (neighborId == CDTriangulation::INVALID_NEIGHBOR)
				{
					continue;
				}

				NavMesh::DualGraphNode* neighbor = dualGraph->GetNode(neighborId);

				if (visited.find(neighbor->GetId()) != visited.end())
				{
					continue;
				}

				if (neighbor->triangle->isObstacle)
				{
					continue;
				}

				int heuristicCost = HeuristicCost(neighbor->center, polygonNode1->center);
				q.push({ neighborId, curPolygonId, heuristicCost });
			}
		}
		
		std::vector<int> polygonNodesPath;
		int curId = destinationNode.polygonNodeId;
		while (curId != -1)
		{
			polygonNodesPath.push_back(curId);

			AStarSearchNode node = visited[curId];
			curId = node.fromPolygonNodeId;
		}

		// reverse it
		int i0 = 0;
		int i1 = polygonNodesPath.size() - 1;
		while (i0 < i1)
		{
			int temp = polygonNodesPath[i0];
			polygonNodesPath[i0] = polygonNodesPath[i1];
			polygonNodesPath[i1] = temp;

			i0++;
			i1--;
		}

		return polygonNodesPath;
	}

	/*
	void CorrectPortalLeftRightEndpoints(std::vector<NavMesh::Edge>& portals, glm::vec3 start)
	{

		// re adjust left and right for all edges
		for (int i = 0; i < portals.size(); i++)
		{
			glm::vec3 v0 = portals[i].vertices[0];
			glm::vec3 v1 = portals[i].vertices[1];

			glm::vec3 dir0 = v0 - start;
			glm::vec3 dir1 = v1 - start;

			glm::vec3 middleVector = (dir0 + dir1) / (float)2;

			glm::vec3 originalCross = glm::cross(dir0, dir1);

			glm::vec3 cross0 = glm::cross(middleVector, dir0);
			glm::vec3 cross1 = glm::cross(dir1, middleVector);

			if (glm::dot(-originalCross, cross0) > 0 && glm::dot(-originalCross, cross1))
			{
				// do nothing
			}
			else
			{
				// swap it, so that [0] is left
				glm::vec3 temp = portals[i].vertices[0];
				portals[i].vertices[0] = portals[i].vertices[1];
				portals[i].vertices[1] = temp;
			}
		}
	}
	*/


	void GetEdgeVertices(
		CDTriangulation::DelaunayTriangleEdge edge,
		CDTriangulation::Graph* graph,
		CDTriangulation::Vertex& v0,
		CDTriangulation::Vertex& v1)
	{
		int id0 = edge.vertices[0];
		v0 = graph->GetVertexById(id0);

		int id1 = edge.vertices[1];
		v1 = graph->GetVertexById(id1);
	}


	// points are counter clock-wise in ZX plane
	void TrySetPortalPoints(
		std::vector<CDTriangulation::DelaunayTriangleEdge> edges, 
		int index, 
		CDTriangulation::Graph* graph, 
		glm::vec3& p0, 
		glm::vec3& p1)
	{
		if (0 <= index && index < edges.size())
		{
			CDTriangulation::DelaunayTriangleEdge edge = edges[index];

			CDTriangulation::Vertex v0, v1;
			GetEdgeVertices(edge, graph, v0, v1);

			p0 = v0.pos;
			p1 = v1.pos;
		}
	}
	

	
	// http://digestingduck.blogspot.com/2010/03/simple-stupid-funnel-algorithm.html
	std::vector<glm::vec3> Funnel(
		NavMesh::DualGraph* dualGraph, 
		CDTriangulation::Graph* graph, 
		std::vector<int> nodeIds, 
		glm::vec3 start, glm::vec3 end)
	{
		std::vector<glm::vec3> results;
		std::vector<CDTriangulation::DelaunayTriangleEdge> portals = dualGraph->GetPortalList(nodeIds);



		CDTriangulation::DelaunayTriangleEdge edge = portals[0];
		CDTriangulation::Vertex v0, v1;
		GetEdgeVertices(edge, graph, v0, v1);

		// we setup the first funnel
		glm::vec3 portalApex = start;

		// my vertices are counter clockwise
		glm::vec3 portalRightPoint = v0.pos;
		glm::vec3 portalLeftPoint = v1.pos;

		results.push_back(start);

		std::cout << "portalApex " << portalApex.x << " " << portalApex.y << std::endl;
		std::cout << "		portalRightPoint " << portalRightPoint.x << " " << portalRightPoint.y << std::endl;
		std::cout << "		portalLeftPoint " << portalLeftPoint.x << " " << portalLeftPoint.y << std::endl;

		int apexIndex = 0, leftIndex = 0, rightIndex = 0;
		glm::vec3 dirL, dirR;

		for (int i = 1; i < portals.size(); i++)
		{
			std::cout << "checking edges " << i << std::endl;
			
			// now we check edges 
			edge = portals[i];


			glm::vec3 vec = portalApex;
			std::cout << "		apex is " << vec.x << " " << vec.y << " " << vec.z << std::endl << std::endl;


			vec = portalLeftPoint;
			std::cout << "		LeftIndex to " << leftIndex << std::endl;
			std::cout << "		Left Point is " << vec.x << " " << vec.y << " " << vec.z << std::endl << std::endl;

			vec = portalRightPoint;
			std::cout << "		RightIndex to " << rightIndex << std::endl;
			std::cout << "		right point is " << vec.x << " " << vec.y << " " << vec.z << std::endl << std::endl;


			/*
			vec = portals[i].vertices[0];
			std::cout << "		newPortalRightPoint " << vec.x << " " << vec.y << " " << vec.z << std::endl;

			vec = portals[i].vertices[1];
			std::cout << "		newPortalLeftPoint " << vec.x << " " << vec.y << " " << vec.z << std::endl << std::endl;
			*/

			// first check the right vertex
			GetEdgeVertices(edge, graph, v0, v1);

			glm::vec3 newPortalRightPoint = v0.pos;
			glm::vec3 newPortalLeftPoint = v1.pos;

			if (Math::Equals(newPortalRightPoint, portalRightPoint))
			{
				portalRightPoint = newPortalRightPoint;
				rightIndex = i;
			}

			if (Math::Equals(newPortalLeftPoint, portalLeftPoint))
			{
				portalLeftPoint = newPortalLeftPoint;
				leftIndex = i;
			}


			dirL = portalLeftPoint - portalApex;
			dirR = portalRightPoint - portalApex;

			std::cout << "		dirL " << dirL.x << " " << dirL.y << std::endl;
			std::cout << "		dirR " << dirR.x << " " << dirR.y << std::endl;


			glm::vec3 dirNewR = newPortalRightPoint - portalApex;
			// we first check if the new newPortalRight is counter-clockwise of portalRightPoint
			// if not, the sign of the cross product will be different, and we just ignore

			float temp = Math::TriArea_XYPlane(dirR, dirNewR);

			if (Math::TriArea_XYPlane(dirR, dirNewR) >= 0)
			{
				// we then want to check that it's not counter-clockwise of portalLeftPoint
				if (Math::TriArea_XYPlane(dirNewR, dirL) > 0)
				{
					portalRightPoint = newPortalRightPoint;
					rightIndex = i;
				}
				else
				{	
					// right over left, insert left to path
					results.push_back(portalLeftPoint);

					// make the current left new apex
					portalApex = portalLeftPoint;
					apexIndex = leftIndex;

					leftIndex = i + 1;
					// we either restart on leftIndex Edge or rightIndex Edge
					TrySetPortalPoints(portals, leftIndex, graph, portalRightPoint, portalLeftPoint);
					rightIndex = leftIndex;

					// reset the index
					i = leftIndex;
					std::cout << "	resetting to " << i << std::endl;
					continue;
				}
			}


			// then check the left vertex

			glm::vec3 dirNewL = newPortalLeftPoint - portalApex;

			// we first check if the new newPortalLeft is clockwise of portalLeftPoint
			// if not, the sign of the cross product will be different, and we just ignore
			if (Math::TriArea_XYPlane(dirNewL, dirL) >= 0)
			{
				// we then want to check that it's not clockwise of portalRightPoint
				if (Math::TriArea_XYPlane(dirR, dirNewL) > 0)
				{
					portalLeftPoint = newPortalLeftPoint;
					leftIndex = i;
				}
				else
				{
					// right over left, insert left to path
					results.push_back(portalRightPoint);

					// make the current left new apex
					portalApex = portalRightPoint;
					apexIndex = rightIndex;

					rightIndex = i + 1;
					TrySetPortalPoints(portals, rightIndex, graph, portalRightPoint, portalLeftPoint);
					leftIndex = rightIndex;

					// reset the index
					i = rightIndex;
					std::cout << "	resetting to " << i << std::endl;
					continue;
				}
			}
		}
		std::cout << " >>>>>> ending " << apexIndex << " " << portals.size() << std::endl;
		std::cout << "		portalRightPoint " << portalRightPoint.x << " " << portalRightPoint.y << std::endl;
		std::cout << "		portalLeftPoint " << portalLeftPoint.x << " " << portalLeftPoint.y << std::endl;

		dirL = portalLeftPoint - portalApex;
		dirR = portalRightPoint - portalApex;

		if (apexIndex < portals.size() - 1)
		{
			if (glm::length2(dirL) < glm::length2(dirR))
			{
				results.push_back(portalLeftPoint);
			}
			else
			{
				results.push_back(portalRightPoint);
			}
		}

		results.push_back(end);

		return results;
	}
	

	PathfindingResult FindPath(PathFinding::DebugState* debugState, World* world, glm::vec3 start, glm::vec3 goal)
	{
		NavMesh::DualGraph* dualGraph = debugState->dualGraph;

		PathfindingResult result;

		NavMesh::DualGraphNode* node0 = GetNodeContainingPoint(dualGraph, world, start);
		NavMesh::DualGraphNode* node1 = GetNodeContainingPoint(dualGraph, world, goal);

		if (node0 == NULL || node1 == NULL) {
			result.valid = false;
			return result;
		}

		debugState->startNodeId = node0->GetId();
		debugState->endNodeId = node1->GetId();

		result.valid = true;
		if (node0 == node1)
		{
			// return a straight line between them
			result.waypoints.push_back(start);
			result.waypoints.push_back(goal);
		}
		else
		{
			/*
			std::vector<int> nodeIds = AStarSearch(dualGraph, node0, node1);
			result.waypoints.push_back(start);
			for (int i = 0; i < nodeIds.size(); i++)
			{
				int id = nodeIds[i];
				NavMesh::DualGraphNode* node = dualGraph->GetNode(id);
				result.waypoints.push_back(node->center);
			}
			result.waypoints.push_back(goal);
			*/

			
			std::vector<int> nodeIds = AStarSearch(dualGraph, node0, node1);
			for (int i = 0; i < nodeIds.size(); i++)
			{
				std::cout << nodeIds[i] << std::endl;
			}
			result.portals = dualGraph->GetPortalList(nodeIds);		
			
			for (int i = 0; i < result.portals.size(); i++)
			{
				std::cout << result.portals[i].vertices[0] << " " << result.portals[i].vertices[1] << std::endl;
			}

			debugState->portals = result.portals;

			result.waypoints.push_back(start);

			std::vector<glm::vec3> path = Funnel(dualGraph, world->cdTriangulationGraph, nodeIds, start, goal);
			for (int i = 0; i < path.size(); i++)
			{
				result.waypoints.push_back(path[i]);
			}
			result.waypoints.push_back(goal);
			
		}

		return result;
	}
}