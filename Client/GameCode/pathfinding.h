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


	struct Funnel
	{
		std::vector<NavMesh::Portal> portals;
		int apexIndex = 0;
		glm::vec3 apex;


		// left point and left Index dont always point to the same point
		// when we reset portal, leftPoint/rightPoint will point to the apex
		// and leftIndex and rightIndex will point to different indices
		// things get fixed on the next iteration
		glm::vec3 leftPoint;
		glm::vec3 rightPoint;

		int leftIndex = 0;
		int rightIndex = 0;


		Funnel(std::vector<NavMesh::Portal> portalsIn)
		{
			portals = portalsIn;

			// assume first entry is the start;
			apex = portalsIn[0].left;

			leftPoint = apex;
			rightPoint = apex;

			apexIndex = 0;
			leftIndex = 0;
			rightIndex = 0;
		}

		glm::vec3 GetApex()	
		{	
			return apex;	
		}

		glm::vec3 GetLeftPoint()
		{
			return leftPoint;
		}

		glm::vec3 GetRightPoint()
		{
			return rightPoint;
		}

		glm::vec3 GetLeftSideDir()
		{
			return leftPoint - apex;
		}

		glm::vec3 GetRightSideDir()
		{
			return rightPoint - apex;;
		}

		void UpdateLeftSide(int index)
		{
			leftIndex = index;
			leftPoint = portals[index].left;
		}

		void UpdateRightSide(int index)
		{
			rightIndex = index;
			rightPoint = portals[index].right;
		}

		void ResetToLeftPoint()
		{
			// make the current left new apex
			apex = GetLeftPoint();
			apexIndex = leftIndex;

			rightPoint = apex;
			leftPoint = apex;

			// reset the portal. We just reset the portal to itself
			// we will update the left and right point in the next iteration
			leftIndex = apexIndex;
			rightIndex = apexIndex;
		}

		void ResetToRightPoint()
		{
			// make the current left new apex
			apex = GetRightPoint();
			apexIndex = rightIndex;

			rightPoint = apex;
			leftPoint = apex;

			// reset the portal. We just reset the portal to itself
			// we will update the left and right point in the next iteration
			leftIndex = apexIndex;
			rightIndex = apexIndex;
		}
	};


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
	};


	// portals here include start and end.
	FunnelResult Funnel_Core(
		std::vector<NavMesh::Portal> portals,
		glm::vec3 start, glm::vec3 end)
	{
		std::vector<glm::vec3> results;

		Funnel funnel(portals);
		results.push_back(start);

		for (int i = 1; i < portals.size(); i++)
		{
			// now we check edges 
			NavMesh::Portal portal = portals[i];

			// first check the right vertex
			glm::vec3 newPortalRightPoint = portal.right;
			glm::vec3 newPortalLeftPoint = portal.left;

			if (Math::Equals(newPortalRightPoint, funnel.GetRightPoint()))
			{
				funnel.UpdateRightSide(i);
			}

			if (Math::Equals(newPortalLeftPoint, funnel.GetLeftPoint()))
			{
				funnel.UpdateLeftSide(i);
			}

			glm::vec3 dirNewR = newPortalRightPoint - funnel.GetApex();
			// we first check if the new newPortalRight is counter-clockwise of portalRightPoint
			// if not, the sign of the cross product will be different, and we just ignore
			float temp = Math::TriArea_XYPlane(funnel.GetRightSideDir(), dirNewR);

			// when dirNewR is counter-clockwise of dirR, that means the funnel is getting more narrow
			if (Math::TriArea_XYPlane(funnel.GetRightSideDir(), dirNewR) >= 0)
			{
				// if portalApex and portalRightPoint, that means we just had a crossover and we just reset the portal
				// we then want to check that it's not counter-clockwise of portalLeftPoint
				// 
				if (Math::Equals(funnel.GetApex(), funnel.GetRightPoint()) || Math::TriArea_XYPlane(dirNewR, funnel.GetLeftSideDir()) > 0)
				{
					funnel.UpdateRightSide(i);
				}
				else
				{
					// right over left, insert left to path
					results.push_back(funnel.GetLeftPoint());

					funnel.ResetToLeftPoint();

					i = funnel.apexIndex;
					continue;
				}
			}

			// then check the left vertex
			glm::vec3 dirNewL = newPortalLeftPoint - funnel.GetApex();

			// we first check if the new newPortalLeft is clockwise of portalLeftPoint
			// if not, the sign of the cross product will be different, and we just ignore
			// when dirNewL is clockwise of dirL, that means the funnel is getting more narrow
			if (Math::TriArea_XYPlane(dirNewL, funnel.GetLeftSideDir()) >= 0)
			{
				// we then want to check that it's not clockwise of portalRightPoint
				if (Math::Equals(funnel.GetApex(), funnel.GetLeftPoint()) || Math::TriArea_XYPlane(funnel.GetRightSideDir(), dirNewL) > 0)
				{
					funnel.UpdateLeftSide(i);
				}
				else
				{
					// right over left, insert left to path
					results.push_back(funnel.GetRightPoint());

					funnel.ResetToRightPoint();

					i = funnel.apexIndex;
					continue;
				}
			}
		}

		// for the last destination 
		// for portalApex to end crosses the last portal, then we just draw a straight line
		// otherwise, w need to add the last portal as another point 
		//
		results.push_back(end);

		FunnelResult funnelResult;
		funnelResult.waypoints = results;

		return funnelResult;
	}

	
	// http://digestingduck.blogspot.com/2010/03/simple-stupid-funnel-algorithm.html
	// https://gamedev.stackexchange.com/questions/68302/how-does-the-simple-stupid-funnel-algorithm-work
	// http://ahamnett.blogspot.com/2012/10/funnel-algorithm.html
	// https://skatgame.net/mburo/ps/thesis_demyen_2006.pdf
	FunnelResult FunnelPath(
		NavMesh::DualGraph* dualGraph, 
		CDTriangulation::Graph* graph, 
		std::vector<int> aStarNodeIds,
		glm::vec3 start, glm::vec3 end, 
		PathFinding::DebugState* debugState)
	{
		std::vector<NavMesh::Portal> portals;

		NavMesh::Portal startPortal = { start, start };
		portals.push_back(startPortal);
		
		dualGraph->AddToPortalList(graph, aStarNodeIds, portals);

		NavMesh::Portal endPortal = { end, end };
		portals.push_back(endPortal);

		debugState->portals = portals;

		return Funnel_Core(portals, start, end);
	}
	
	struct EdgeIndex
	{
		int leftIndex;
		int rightIndex;

		EdgeIndex()	{}

		EdgeIndex(int l, int r)
		{
			leftIndex = l;
			rightIndex = r;
		}
	};

	// https://github.com/dotsnav/dotsnav
	// https://www.gamedev.net/forums/topic/548610-modified-funnel-algorithm/
	// http://ahamnett.blogspot.com/2012/10/funnel-algorithm.html
	FunnelResult ModifiedFunnelPath(
		NavMesh::DualGraph* dualGraph,
		CDTriangulation::Graph* graph,
		std::vector<int> aStarNodeIds,
		glm::vec3 start, glm::vec3 end, 
		float agentRadius,
		PathFinding::DebugState* debugState)
	{
		std::vector<glm::vec3> results;

		std::vector<NavMesh::Portal> portals;
		std::vector<EdgeIndex> indices;

		NavMesh::Portal startPortal = { start, start };
		portals.push_back(startPortal);

		dualGraph->AddToPortalList(graph, aStarNodeIds, portals);

		NavMesh::Portal endPortal = { end, end };
		portals.push_back(endPortal);

		debugState->portals = portals;


		std::vector<glm::vec3> leftVertices;
		std::vector<glm::vec3> rightVertices;

		for (int i = 0; i < portals.size(); i++)
		{
			if (i == 0)
			{
				leftVertices.push_back(portals[i].left);
				rightVertices.push_back(portals[i].right);

				indices.push_back(EdgeIndex(i, i));
			}
			else
			{
				int index0 = leftVertices.size() - 1;
				int index1 = rightVertices.size() - 1;

				EdgeIndex edgeIndex = EdgeIndex(index0, index1);

				glm::vec3 leftV = leftVertices[index0];
				glm::vec3 rightV = rightVertices[index1];

				if (!Math::Equals(leftV, portals[i].left))
				{
					leftVertices.push_back(portals[i].left);
					edgeIndex.leftIndex += 1;
				}

				if (!Math::Equals(rightV, portals[i].right))
				{
					rightVertices.push_back(portals[i].right);
					edgeIndex.rightIndex += 1;
				}

				indices.push_back(edgeIndex);
			}
		}

		std::cout << "leftVertices " << leftVertices.size() << std::endl;
		for (int i = 0; i < leftVertices.size(); i++)
		{
			std::cout << leftVertices[i].x << " " << leftVertices[i].y << " " << leftVertices[i].z << std::endl;
		}

		std::cout << "rightVertices " << rightVertices.size() << std::endl;
		for (int i = 0; i < rightVertices.size(); i++)
		{
			std::cout << rightVertices[i].x << " " << rightVertices[i].y << " " << rightVertices[i].z << std::endl;
		}

		std::cout << "agentRadius " << agentRadius << std::endl;

		std::vector<PathFinding::Vector> leftPerp;
		std::vector<PathFinding::Vector> rightPerp;

		// https://www.youtube.com/watch?v=rWHrTrigIYo&ab_channel=jdtec01
		// start from 2:20
		// the first and last point is start and end, so we dont care
		for (int i = 1; i < leftVertices.size() - 1 ; i++)
		{
			glm::vec3 oldVertex = leftVertices[i];

			// for eac vertex. gets its neighbor edges
			glm::vec3 p0 = leftVertices[i - 1];
			glm::vec3 p1 = leftVertices[i];
			glm::vec3 p2 = leftVertices[i + 1];

			// Get the average of neighbor edge directions
			// I think I can just directly get the perp (instead of following the blog and getting the averageNeighborEdge first)
			glm::vec3 dir1 = glm::normalize(p1 - p0);
			glm::vec3 dir2 = glm::normalize(p2 - p1);

			glm::vec3 averageNeighborEdge = dir1 + dir2;

			glm::vec3 averageNeighborEdgePerp = glm::cross(averageNeighborEdge, glm::vec3(0, 0, 1));
			averageNeighborEdgePerp = glm::normalize(averageNeighborEdgePerp);

			glm::vec3 newVertex = leftVertices[i] + averageNeighborEdgePerp * agentRadius;
			leftVertices[i] = newVertex;


			PathFinding::Vector vec;
			vec.v0 = oldVertex;
			vec.v1 = newVertex;

			leftPerp.push_back(vec);
		}


		for (int i = 1; i < rightVertices.size() - 1; i++)
		{
			glm::vec3 oldVertex = rightVertices[i];

			// for eac vertex. gets its neighbor edges
			glm::vec3 p0 = rightVertices[i - 1];
			glm::vec3 p1 = rightVertices[i];
			glm::vec3 p2 = rightVertices[i + 1];

			// Get the average of neighbor edge directions
			// I think I can just directly get the perp (instead of following the blog and getting the averageNeighborEdge first)
			glm::vec3 dir1 = glm::normalize(p1 - p0);
			glm::vec3 dir2 = glm::normalize(p2 - p1);

			glm::vec3 averageNeighborEdge = dir1 + dir2;

			glm::vec3 averageNeighborEdgePerp = glm::cross(averageNeighborEdge, glm::vec3(0, 0, -1));
			averageNeighborEdgePerp = glm::normalize(averageNeighborEdgePerp);

			glm::vec3 newVertex = rightVertices[i] + averageNeighborEdgePerp * agentRadius;
			rightVertices[i] = newVertex;

			PathFinding::Vector vec;
			vec.v0 = oldVertex;
			vec.v1 = newVertex;

			rightPerp.push_back(vec);
		}

		debugState->newLeftVertices = leftVertices;
		debugState->newRightVertices = rightVertices;

		debugState->leftAnePerp = leftPerp;
		debugState->rightAnePerp = rightPerp;



		std::vector<NavMesh::Portal> modifiedPortals;
		for (int i = 0; i < portals.size(); i++)
		{
			if (i == 0 || i == portals.size() - 1)
			{
				modifiedPortals.push_back(portals[i]);
			}
			else
			{
				EdgeIndex edgeIndex = indices[i];

				NavMesh::Portal portal;
				portal.left = leftVertices[edgeIndex.leftIndex];
				portal.right = rightVertices[edgeIndex.rightIndex];

				modifiedPortals.push_back(portal);
			}
		}

		debugState->modifiedPortals = modifiedPortals;

		return Funnel_Core(modifiedPortals, start, end);
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

			float radius = agentDiameter / 2.0f;
			FunnelResult funnelResult = ModifiedFunnelPath(dualGraph, world->cdTriangulationGraph, aStarResult.nodePath, start, finalDestination, radius, debugState);
			// FunnelResult funnelResult = FunnelPath(dualGraph, world->cdTriangulationGraph, aStarResult.nodePath, start, finalDestination, debugState);

			result.waypoints = funnelResult.waypoints;

			/*
			FunnelResult funnelResult = Funnel(dualGraph, world->cdTriangulationGraph, aStarResult.nodePath, start, finalDestination);
			
			result.portals = funnelResult.portals;
			debugState->portals = result.portals;

			float radius = agentDiameter / 2.0f  ;
			ModifiedFunnel(dualGraph, world->cdTriangulationGraph, aStarResult.nodePath, start, finalDestination, radius, debugState);

			result.waypoints = funnelResult.waypoints;
			*/
		}

		return result;
	}
}