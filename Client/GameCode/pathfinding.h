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
		std::vector<NavMesh::Portal> portals;
	};

	struct AStarSearchResult
	{
		bool reachedOriginalGoal;
		std::vector<int> nodePath;
	};


	struct AStarSearchNode
	{
		int polygonNodeId;

		// this is to remeber the path
		int fromPolygonNodeId;
		int cost;		// g(n) + f(n)
		int costFromStartNode; // this is g(n)
		CDTriangulation::DelaunayTriangleEdge sourceEdge;

		AStarSearchNode() {}

		AStarSearchNode(int polygonNodeIdIn, int fromPolygonNodeIdIn, int costIn, int costFromStartNodeIn, CDTriangulation::DelaunayTriangleEdge sourceEdgeIn)
		{
			polygonNodeId = polygonNodeIdIn;
			fromPolygonNodeId = fromPolygonNodeIdIn;
			cost = costIn;
			costFromStartNode = costFromStartNodeIn;
			sourceEdge = sourceEdgeIn;
		}
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
		glm::vec3 len = pos1 - pos0;
		return glm::fastLength(len);
	}

	bool CheckTunnelWidth(CDTriangulation::DelaunayTriangle* curTriangle, 
		CDTriangulation::DelaunayTriangleEdge sourceEdge,
		CDTriangulation::DelaunayTriangle* neighbor, 
		CDTriangulation::DelaunayTriangleEdge dstEdge,
		float agentDiameter)
	{
	
		std::cout << ">>>> curTriangle " << curTriangle->id << ", neighbor is " << neighbor->id << std::endl;
		for (int i = 0; i < CDTriangulation::NUM_TRIANGLE_VERTEX; i++)
		{
			std::cout << "	curTriangle " << curTriangle->halfWidths[i] << std::endl;
		}

		if (!CDTriangulation::DelaunayTriangleEdge::IsValidEdge(sourceEdge))
		{
			return true;
		}

		int edgeIndex0 = curTriangle->GetEdgeIndex(sourceEdge);

		int edgeIndex1 = (edgeIndex0 - 1 + 3) % CDTriangulation::NUM_TRIANGLE_EDGES;
		CDTriangulation::DelaunayTriangleEdge edge1 = curTriangle->edges[edgeIndex1];
		if (dstEdge == edge1)
		{
			float halfWidth = curTriangle->halfWidths[edgeIndex0];
			return agentDiameter <= halfWidth;
		}

		edgeIndex1 = (edgeIndex0 + 1) % CDTriangulation::NUM_TRIANGLE_EDGES;
		edge1 = curTriangle->edges[edgeIndex1];
		if (dstEdge == edge1)
		{
			float halfWidth = curTriangle->halfWidths[edgeIndex1];
			return agentDiameter <= halfWidth;
		}

		return true;
	}


	AStarSearchResult AStarSearch(NavMesh::DualGraph* dualGraph,
		float agentDiameter, 
		NavMesh::DualGraphNode* startNode, glm::vec3 start, 
		NavMesh::DualGraphNode* destNode, glm::vec3 end)
	{
		std::cout << "start" << startNode->GetId() << std::endl;
		std::cout << "end" << destNode->GetId() << std::endl;


		AStarSearchResult result;
		result.reachedOriginalGoal = false;

		// from triangle 0, do a bfs with a distance heuristic until u reach triangle 1
		std::priority_queue<AStarSearchNode, std::vector<AStarSearchNode>, AStarSearchNodeComparison> q;

		std::unordered_map<int, AStarSearchNode> visited;

		CDTriangulation::DelaunayTriangleEdge edge = CDTriangulation::DelaunayTriangleEdge::GetInvalidEdge();
		AStarSearchNode node = AStarSearchNode(startNode->GetId(), -1, 0, 0, edge);
		q.push(node);
	

		AStarSearchNode destinationNode;
		float curClosestDist = FLT_MAX;
		while (!q.empty())
		{
			AStarSearchNode curAStarNode = q.top();
			q.pop();

			visited[curAStarNode.polygonNodeId] = curAStarNode;


			if (curAStarNode.polygonNodeId == destNode->GetId())
			{
				destinationNode = curAStarNode;
				result.reachedOriginalGoal = true;
				break;
			}

			int curPolygonId = curAStarNode.polygonNodeId;
			NavMesh::DualGraphNode* curGraphNode = dualGraph->GetNode(curPolygonId);

			std::cout << ">>>>> visiting " << curPolygonId << std::endl;

			glm::vec3 fromPos;
			if (curAStarNode.polygonNodeId == startNode->GetId())
			{
				fromPos = start;
			}
			else
			{
				fromPos = curGraphNode->center;
			}

			float distToDest = HeuristicCost(fromPos, end);
			if (distToDest < curClosestDist)
			{
				curClosestDist = distToDest;
				destinationNode = curAStarNode;
			}

			// go through neighbors

			CDTriangulation::DelaunayTriangle* triangle = curGraphNode->triangle;
			for (int i = 0; i < ArrayCount(triangle->neighbors); i++)
			{
				int neighborId = triangle->neighbors[i];
				std::cout << "		neighborId " << neighborId << std::endl;

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

				CDTriangulation::DelaunayTriangleEdge edge = triangle->edges[i];

				if (!CheckTunnelWidth(triangle, curAStarNode.sourceEdge, neighbor->triangle, edge, agentDiameter))
				{
					continue;
				}

				float cost1 = HeuristicCost(fromPos, neighbor->center);
				float cost2 = HeuristicCost(neighbor->center, end);

				int costFromStartNode = curAStarNode.costFromStartNode + HeuristicCost(fromPos, neighbor->center);
				int totalCost = costFromStartNode + HeuristicCost(neighbor->center, end);

				std::cout << "			costFromStartNode " << costFromStartNode << std::endl;
				std::cout << "			cost1 " << cost1 << std::endl;
				std::cout << "			cost2 " << cost2 << std::endl;

				std::cout << "			totalCost " << totalCost << std::endl;

				q.push(AStarSearchNode(neighborId, curPolygonId, totalCost, costFromStartNode, edge));
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

		result.nodePath = polygonNodesPath;

		return result;
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

	struct FunnelResult
	{
		std::vector<glm::vec3> waypoints;
		std::vector<NavMesh::Portal> portals;
	};

	
	// http://digestingduck.blogspot.com/2010/03/simple-stupid-funnel-algorithm.html
	// https://gamedev.stackexchange.com/questions/68302/how-does-the-simple-stupid-funnel-algorithm-work
	// http://ahamnett.blogspot.com/2012/10/funnel-algorithm.html
	FunnelResult Funnel(
		NavMesh::DualGraph* dualGraph, 
		CDTriangulation::Graph* graph, 
		std::vector<int> aStarNodeIds,
		glm::vec3 start, glm::vec3 end)
	{
		std::vector<glm::vec3> results;

		std::vector<NavMesh::Portal> portals;

		NavMesh::Portal startPortal = { start, start };
		portals.push_back(startPortal);
		
		dualGraph->AddToPortalList(graph, aStarNodeIds, portals);

		NavMesh::Portal endPortal = { end, end };
		portals.push_back(endPortal);


		// we setup the first funnel
		glm::vec3 portalApex = start;

		// my vertices are counter clockwise
		glm::vec3 portalRightPoint = startPortal.right;
		glm::vec3 portalLeftPoint = startPortal.left;

		results.push_back(start);

		// std::cout << "portalApex " << portalApex.x << " " << portalApex.y << std::endl;
		// std::cout << "		portalRightPoint " << portalRightPoint.x << " " << portalRightPoint.y << std::endl;
		// std::cout << "		portalLeftPoint " << portalLeftPoint.x << " " << portalLeftPoint.y << std::endl;

		int apexIndex = 0, leftIndex = 0, rightIndex = 0;
		glm::vec3 dirL, dirR;

		for (int i = 1; i < portals.size(); i++)
		{
			// std::cout << "checking edges " << i << std::endl;
			
			// now we check edges 
			NavMesh::Portal portal = portals[i];

			/*
			glm::vec3 vec = portalApex;
			std::cout << "		apex is " << vec.x << " " << vec.y << " " << vec.z << std::endl << std::endl;


			vec = portalLeftPoint;
			std::cout << "		LeftIndex to " << leftIndex << std::endl;
			std::cout << "		Left Point is " << vec.x << " " << vec.y << " " << vec.z << std::endl << std::endl;

			vec = portalRightPoint;
			std::cout << "		RightIndex to " << rightIndex << std::endl;
			std::cout << "		right point is " << vec.x << " " << vec.y << " " << vec.z << std::endl << std::endl;


			vec = portals[i].vertices[0];
			std::cout << "		newPortalRightPoint " << vec.x << " " << vec.y << " " << vec.z << std::endl;

			vec = portals[i].vertices[1];
			std::cout << "		newPortalLeftPoint " << vec.x << " " << vec.y << " " << vec.z << std::endl << std::endl;
			*/

			// first check the right vertex
			glm::vec3 newPortalRightPoint = portal.right;
			glm::vec3 newPortalLeftPoint = portal.left;
			
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
			
			/*
			vec = portalLeftPoint;
			std::cout << "		LeftIndex2 to " << leftIndex << std::endl;
			std::cout << "		Left Point2 is " << vec.x << " " << vec.y << " " << vec.z << std::endl << std::endl;

			vec = portalRightPoint;
			std::cout << "		RightIndex2 to " << rightIndex << std::endl;
			std::cout << "		right point2 is " << vec.x << " " << vec.y << " " << vec.z << std::endl << std::endl;


			vec = newPortalLeftPoint;
			std::cout << "		newPortalLeftPoint is " << vec.x << " " << vec.y << " " << vec.z << std::endl << std::endl;

			vec = newPortalRightPoint;
			std::cout << "		newPortalRightPoint is " << vec.x << " " << vec.y << " " << vec.z << std::endl << std::endl;
			*/

			dirL = portalLeftPoint - portalApex;
			dirR = portalRightPoint - portalApex;

			// std::cout << "		dirL " << dirL.x << " " << dirL.y << std::endl;
			// std::cout << "		dirR " << dirR.x << " " << dirR.y << std::endl;


			glm::vec3 dirNewR = newPortalRightPoint - portalApex;
			// we first check if the new newPortalRight is counter-clockwise of portalRightPoint
			// if not, the sign of the cross product will be different, and we just ignore
			float temp = Math::TriArea_XYPlane(dirR, dirNewR);

			// when dirNewR is counter-clockwise of dirR, that means the funnel is getting more narrow
			if (Math::TriArea_XYPlane(dirR, dirNewR) >= 0)
			{
				// if portalApex and portalRightPoint, that means we just had a crossover and we just reset the portal
				// we then want to check that it's not counter-clockwise of portalLeftPoint
				// 
				if (Math::Equals(portalApex, portalRightPoint) || Math::TriArea_XYPlane(dirNewR, dirL) > 0)
				{
					portalRightPoint = newPortalRightPoint;
					rightIndex = i;
				}
				else
				{	
				//	std::cout << "	########### right crossing over left happening " << std::endl;

					// right over left, insert left to path
					results.push_back(portalLeftPoint);

					// make the current left new apex
					portalApex = portalLeftPoint;
					apexIndex = leftIndex;

					// reset the portal. We just reset the portal to itself
					// we will update the left and right point in the next iteration
					portalRightPoint = portalApex;
					portalLeftPoint = portalApex;

					leftIndex = apexIndex;
					rightIndex = apexIndex;

					// reset the index
					i = apexIndex;
				//	std::cout << "		resetting to " << i << std::endl;
					continue;
				}
			}


			// then check the left vertex

			glm::vec3 dirNewL = newPortalLeftPoint - portalApex;

			// we first check if the new newPortalLeft is clockwise of portalLeftPoint
			// if not, the sign of the cross product will be different, and we just ignore
			// when dirNewL is clockwise of dirL, that means the funnel is getting more narrow
			if (Math::TriArea_XYPlane(dirNewL, dirL) >= 0)
			{
				// we then want to check that it's not clockwise of portalRightPoint
				if (Math::Equals(portalApex, portalLeftPoint) || Math::TriArea_XYPlane(dirR, dirNewL) > 0)
				{
					portalLeftPoint = newPortalLeftPoint;
					leftIndex = i;
				//	std::cout << "	resetting to " << i << std::endl;
				}
				else
				{
				//	std::cout << "	########### left crossing over right happening " << std::endl;

					// right over left, insert left to path
					results.push_back(portalRightPoint);

					// make the current left new apex
					portalApex = portalRightPoint;
					apexIndex = rightIndex;


					// reset the portal. We just reset the portal to itself
					// we will update the left and right point in the next iteration
					portalRightPoint = portalApex;
					portalLeftPoint = portalApex;
					leftIndex = apexIndex;
					rightIndex = apexIndex;

					i = apexIndex;
				//	std::cout << "	resetting to " << i << std::endl;
					continue;
				}
			}
		}
/*
		std::cout << " >>>>>> ending " << apexIndex << " " << portals.size() << std::endl;
		std::cout << "		portalRightPoint " << portalRightPoint.x << " " << portalRightPoint.y << std::endl;
		std::cout << "		portalLeftPoint " << portalLeftPoint.x << " " << portalLeftPoint.y << std::endl;
*/

		// for the last destination 
		// for portalApex to end crosses the last portal, then we just draw a straight line
		// otherwise, w need to add the last portal as another point 
		//
	/*
		glm::vec2 intersectionPoint;
		if (Collision::GetLineLineIntersectionPoint_CheckOnlyXY2D(portalApex, end, portalLeftPoint, portalRightPoint, intersectionPoint))
		{
			results.push_back(end);
		}
		else
		{
			dirL = portalLeftPoint - portalApex;
			glm::vec3 leftToEnd = end - portalLeftPoint;

			dirR = portalRightPoint - portalApex;
			glm::vec3 rightToEnd = end - portalRightPoint;

			float leftCost = glm::fastLength(dirL) + glm::fastLength(leftToEnd);
			float rightCost = glm::fastLength(dirR) + glm::fastLength(rightToEnd);

			if (leftCost < rightCost)
			{
				results.push_back(portalLeftPoint);
			}
			else
			{
				results.push_back(portalRightPoint);
			}
			
			results.push_back(end);
		}
		*/
		results.push_back(end);

		FunnelResult funnelResult;
		funnelResult.waypoints = results;
		funnelResult.portals = portals;

		return funnelResult;
	}
	

	PathfindingResult FindPath(PathFinding::DebugState* debugState, float agentDiameter, World* world, glm::vec3 start, glm::vec3 goal)
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

			AStarSearchResult aStarResult = AStarSearch(dualGraph, agentDiameter, node0, start, node1, goal);

				/*
				for (int i = 0; i < aStarNodeIds.size(); i++)
				{
					std::cout << aStarNodeIds[i] << std::endl;
				}


				result.portals = dualGraph->GetPortalList(aStarNodeIds);

				for (int i = 0; i < result.portals.size(); i++)
				{
					std::cout << result.portals[i].vertices[0] << " " << result.portals[i].vertices[1] << std::endl;
				}


				debugState->portals = result.portals;

				result.waypoints.push_back(start);
				*/

				// AStarSearch may not get u to goal because agent may not fit in, so u may get a different destination.
			glm::vec3 finalDestination = goal;
			if (!aStarResult.reachedOriginalGoal)
			{
				int index = aStarResult.nodePath.size() - 1;
				finalDestination = dualGraph->GetNodeCenter(aStarResult.nodePath[index]);
			}

			FunnelResult funnelResult = Funnel(dualGraph, world->cdTriangulationGraph, aStarResult.nodePath, start, finalDestination);
			
			result.portals = funnelResult.portals;
			debugState->portals = result.portals;

			result.waypoints = funnelResult.waypoints;
		}

		return result;
	}
}