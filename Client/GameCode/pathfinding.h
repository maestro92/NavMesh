#pragma once

#include "../PlatformShared/platform_shared.h"
#include "nav_mesh.h"
#include "pathfinding_common.h"
#include "world.h"

#include "collision.h"

#include <queue>
#include <deque>
#include <unordered_map>

namespace PathFinding
{
	struct EdgeIndex
	{
		int leftIndex;
		int rightIndex;

		EdgeIndex() {}

		EdgeIndex(int l, int r)
		{
			leftIndex = l;
			rightIndex = r;
		}
	};


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

	void JumpAndWalk()
	{
		// sample size   m = O(n ^ 1/4)

		// distance of the triangle to the query point q is calculated as the min distance of its three vertices to q
		// the triangle which scores the shortest distance is selected. 
		
	}




	NavMesh::DualGraphNode* GetNodeContainingPoint(
		NavMesh::DualGraph* pathingEnvironment,
		World* world,
		glm::vec3 point)
	{
		CDT::DelaunayTriangle* triangle = world->FindTriangleBySimPos(point);
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
		std::vector<int> nodePath;
	
		// could be different the end u passed in if orignal can't be reached
		glm::vec3 finalEnd;
	};

	/*
	struct VisitEdgeEntry
	{
		bool isValid;
		CDT::DelaunayTriangleEdge edge;
	};
	*/

	struct AStarSearchNode
	{
		int polygonNodeId;

		// this is to remeber the path
		int fromPolygonNodeId;
		int costFromStartNode; // g(n)
		int cost;		// f(n) = g(n) + h(n)
		glm::vec3 pos;

		CDT::DelaunayTriangleEdge sourceEdge;

	//	VisitEdgeEntry visitedEdges[3];

		AStarSearchNode() { polygonNodeId = -1; }

		AStarSearchNode(int polygonNodeIdIn, int fromPolygonNodeIdIn, int gCostIn, int fCostIn, glm::vec3 posIn, CDT::DelaunayTriangleEdge sourceEdgeIn)
		{
			polygonNodeId = polygonNodeIdIn;
			fromPolygonNodeId = fromPolygonNodeIdIn;
			costFromStartNode = gCostIn;
			cost = fCostIn;
			sourceEdge = sourceEdgeIn;
			pos = posIn;
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

	bool CheckTunnelWidth(CDT::DelaunayTriangle* curTriangle, 
		CDT::DelaunayTriangleEdge sourceEdge,
		CDT::DelaunayTriangle* neighbor, 
		CDT::DelaunayTriangleEdge dstEdge,
		float agentDiameter)
	{
	
		/*
		std::cout << ">>>> curTriangle " << curTriangle->id << ", neighbor is " << neighbor->id << std::endl;
		for (int i = 0; i < CDT::NUM_TRIANGLE_VERTEX; i++)
		{
			std::cout << "	curTriangle " << curTriangle->halfWidths[i] << std::endl;
		}
		*/
		if (!CDT::DelaunayTriangleEdge::IsValidEdge(sourceEdge))
		{
			return true;
		}

		int edgeIndex0 = curTriangle->GetEdgeIndex(sourceEdge);

		int edgeIndex1 = (edgeIndex0 - 1 + 3) % CDT::NUM_TRIANGLE_EDGES;
		CDT::DelaunayTriangleEdge edge1 = curTriangle->edges[edgeIndex1];
		if (dstEdge == edge1)
		{
			float halfWidth = curTriangle->halfWidths[edgeIndex0];
			return agentDiameter <= halfWidth;
		}

		edgeIndex1 = (edgeIndex0 + 1) % CDT::NUM_TRIANGLE_EDGES;
		edge1 = curTriangle->edges[edgeIndex1];
		if (dstEdge == edge1)
		{
			float halfWidth = curTriangle->halfWidths[edgeIndex1];
			return agentDiameter <= halfWidth;
		}

		return true;
	}

	/*
	bool HasVisitedNodeFromThisEdge(
		std::unordered_map<TriangleId, AStarSearchNode>& visited, 
		TriangleId triangleId,
		CDT::DelaunayTriangleEdge edge)
	{
		if (visited.find(triangleId) == visited.end())
		{
			return false;
		}
		
		AStarSearchNode node = visited[triangleId];
		for (int i = 0; i < ArrayCount(node.visitedEdges); i++) 
		{
			if (node.visitedEdges[i].isValid && node.visitedEdges[i].edge == edge)
			{
				return true;
			}
		}		

		return false;
	}
	*/


	// we shrink the edge to better estimate where the agent will enter in this edge 
	void ShrinkEdge(glm::vec3& v0, glm::vec3& v1, float agentRadius)
	{
		// we dont do an accurate shrinking. We just do a rough shrinking

		// TODO: make this fast
		glm::vec3 dx = v1 - v0;
		float distSquared = glm::dot(dx, dx);
		
		do
		{
			dx /= 2.0f;
			distSquared /= 4.0f;
		} 		
		while (distSquared > agentRadius * agentRadius);

		v0 += dx;
		v1 -= dx;

		/*
		
		float lengthSquared = glm::dot(dx, dx);
		if (lengthSquared > agentDiameterSqured)
		{
			v0v1 = glm::normalize(dx);
			v0 = v0 + dx * agentRadius;
			v1 = v1 - dx * agentRadius;
		}
		
		*/

	}

	bool CanReachEndInFinalNode(AStarSearchNode curAStarNode, CDT::DelaunayTriangle* triangle, glm::vec3 end, float aagentDiameter)
	{
		// get my final trajectory
		glm::vec3 trajectoryPoint0 = curAStarNode.pos;
		glm::vec3 trajectoryPoint1 = end;

		for (int i = 0; i < ArrayCount(triangle->halfWidthLines); i++)
		{
			CDT::HalfWidthLine halfWidthLine = triangle->halfWidthLines[i];
			if (halfWidthLine.isInside)
			{
				glm::vec2 intersectionPoint;
				if (Collision::GetLineLineIntersectionPoint_CheckOnlyXY2D(
					halfWidthLine.p0, 
					halfWidthLine.p1, 
					trajectoryPoint0, 
					trajectoryPoint1, 
					intersectionPoint))
				{
					if (triangle->halfWidths[i] < aagentDiameter)
					{
						return false;
					}
				}
			}
		}

		return true;
	}


	AStarSearchResult AStarSearch(NavMesh::DualGraph* dualGraph,
		float agentDiameter, 
		NavMesh::DualGraphNode* startNode, glm::vec3 start, 
		NavMesh::DualGraphNode* destNode, glm::vec3 end,
		PathFinding::DebugState* debugState)
	{
		std::cout << "start" << startNode->GetId() << std::endl;
		std::cout << "end" << destNode->GetId() << std::endl;

		
		float maxSearchRange = 150;

		bool hasFailedDstNodeRoute = false;

		debugState->aStarWaypoints.clear();

		float agentDiameterSquared = agentDiameter * agentDiameter;
		float agentRadius = agentDiameter / 2.0f;
		AStarSearchResult result;

		// from triangle 0, do a bfs with a distance heuristic until u reach triangle 1
		std::priority_queue<AStarSearchNode, std::vector<AStarSearchNode>, AStarSearchNodeComparison> q;

		// change this to an array
		std::unordered_map<TriangleId, AStarSearchNode> visited;


		CDT::DelaunayTriangleEdge edge = CDT::DelaunayTriangleEdge::GetInvalidEdge();

		float cost = HeuristicCost(start, end);
		AStarSearchNode node = AStarSearchNode(startNode->GetId(), -1, 0, cost, start, edge);
		q.push(node);
	
		bool reachedOriginalEnd = false;

		float curClosestDist = FLT_MAX;
		glm::vec3 curClosestEnd;
		AStarSearchNode destinationNode;

		while (!q.empty())
		{
			AStarSearchNode curAStarNode = q.top();
			q.pop();

			int curPolygonId = curAStarNode.polygonNodeId;
			NavMesh::DualGraphNode* curGraphNode = dualGraph->GetNode(curPolygonId);

			if (curPolygonId == 97)
			{
				std::cout << ">>>>> visiting " << curPolygonId << std::endl;
				std::cout << "			" << curAStarNode.pos.x << " " << curAStarNode.pos.y << std::endl;
			}

			float distToDest = HeuristicCost(curAStarNode.pos, end);
			if (distToDest < curClosestDist)
			{
				curClosestDist = distToDest;
				destinationNode = curAStarNode;
				curClosestEnd = curAStarNode.pos;
			}

			// go through neighbors
			visited[curAStarNode.polygonNodeId] = curAStarNode;

			CDT::DelaunayTriangle* triangle = curGraphNode->triangle;
			if (curAStarNode.polygonNodeId == destNode->GetId())
			{
				if (CanReachEndInFinalNode(curAStarNode, triangle, end, agentDiameter))
				{
					destinationNode = curAStarNode;
					result.finalEnd = end;
					reachedOriginalEnd = true;
					break;
				}
				else
				{
					continue;
				}
			}

			// if it's too far away, we just ignore
			if (distToDest > maxSearchRange)
			{
				continue;
			}


			for (int i = 0; i < ArrayCount(triangle->neighbors); i++)
			{
				int neighborId = triangle->neighbors[i];
			//	std::cout << "		neighborId " << neighborId << std::endl;

				if (neighborId == CDT::INVALID_NEIGHBOR)
				{
					continue;
				}

				NavMesh::DualGraphNode* neighbor = dualGraph->GetNode(neighborId);

				// the destination node can be visited multiple times from different edges
				if (neighbor->GetId() != destNode->GetId() &&
					visited.find(neighbor->GetId()) != visited.end())
				{
					continue;
				}

				if (neighbor->triangle->isObstacle)
				{
					continue;
				}

				CDT::DelaunayTriangleEdge edge = triangle->edges[i];


				if (!CheckTunnelWidth(triangle, curAStarNode.sourceEdge, neighbor->triangle, edge, agentDiameter))
				{
					continue;
				}


				CDT::Vertex v0 = triangle->GetVertexById(edge.vertices[0]);
				CDT::Vertex v1 = triangle->GetVertexById(edge.vertices[1]);

				glm::vec3 pos0 = v0.pos;
				glm::vec3 pos1 = v1.pos;
				ShrinkEdge(pos0, pos1, agentRadius);

				glm::vec3 newPos = Collision::ClosestPointOnLine(pos0, pos1, curAStarNode.pos);



				float gCost = curAStarNode.costFromStartNode + HeuristicCost(curAStarNode.pos, newPos);
				float hCost = HeuristicCost(newPos, end);

				float fCost = gCost + hCost;	// total cost

				/*
				std::cout << "			costFromStartNode " << curAStarNode.costFromStartNode << std::endl;
				std::cout << "			gCost " << gCost << std::endl;
				std::cout << "			hCost " << hCost << std::endl;

				std::cout << "			fCost " << fCost << std::endl;
				*/

				// AStarSearchNode(int polygonNodeIdIn, int fromPolygonNodeIdIn, int costIn, int costFromStartNodeIn, glm::vec3 posIn, CDT::DelaunayTriangleEdge sourceEdgeIn)

				q.push(AStarSearchNode(
					neighborId, 
					curPolygonId, 
					gCost, 
					fCost, newPos, edge));
			}
		}

		if (!reachedOriginalEnd)
		{
			result.finalEnd = curClosestEnd;
		}


		std::vector<int> polygonNodesPath;	
		AStarSearchNode node2 = destinationNode;

		if (node2.fromPolygonNodeId == -1)
		{
			// that means we traveling within the start/end node
			return result;
		}
		else
		{
			int curId = node2.polygonNodeId;
			while (curId != -1)
			{
				polygonNodesPath.push_back(curId);

				debugState->aStarWaypoints.push_back(node2.pos);

				node2 = visited[curId];
				curId = node2.fromPolygonNodeId;
			}


			// reverse it
			int i0 = 0;
			int i1 = polygonNodesPath.size() - 1;
			while (i0 < i1)
			{
				int temp = polygonNodesPath[i0];
				polygonNodesPath[i0] = polygonNodesPath[i1];
				polygonNodesPath[i1] = temp;


				glm::vec3 tempPos = debugState->aStarWaypoints[i0];
				debugState->aStarWaypoints[i0] = debugState->aStarWaypoints[i1];
				debugState->aStarWaypoints[i1] = tempPos;


				i0++;
				i1--;
			}

			result.nodePath = polygonNodesPath;

			return result;
		}


	}


	struct FunnelResult
	{
		std::vector<glm::vec3> waypoints;
	};


	
	void AddSmoothedPoints(
		std::vector<glm::vec3>& results,
		Funnel& funnel,
		std::vector<NavMesh::Portal>& portals,
		std::vector<glm::vec3>& vertices,
		std::vector<EdgeIndex>& indices,
		bool isLeft,
		float agentRadius)
	{
		int vertexIndex = isLeft ? funnel.leftIndex : funnel.rightIndex;

		if (vertexIndex == 0 || vertexIndex == portals.size() - 1)
		{
			glm::vec3 vertex = isLeft ? funnel.GetLeftPoint() : funnel.GetRightPoint();
			results.push_back(vertex);
		}
		else
		{
			EdgeIndex edgeIndex = indices[vertexIndex];

			int i = isLeft ? edgeIndex.leftIndex : edgeIndex.rightIndex;

			// for each vertex. gets its neighbor edges
			glm::vec3 p0 = vertices[i - 1];
			glm::vec3 p1 = vertices[i];
			glm::vec3 p2 = vertices[i + 1];

			// Get the average of neighbor edge directions
			// I think I can just directly get the perp (instead of following the blog and getting the averageNeighborEdge first)
			glm::vec3 dir1 = glm::normalize(p1 - p0);
			glm::vec3 dir2 = glm::normalize(p2 - p1);

			glm::vec3 averageNeighborEdge = dir1 + dir2;

			glm::vec3 supportingVector = isLeft ? glm::vec3(0, 0, 1) : glm::vec3(0, 0, -1);

			glm::vec3 averageNeighborEdgePerp = glm::cross(averageNeighborEdge, supportingVector);
			averageNeighborEdgePerp = glm::normalize(averageNeighborEdgePerp);

			// we want to create a smooth path around the original portal point
			glm::vec3 vertex = isLeft ? portals[vertexIndex].left : portals[vertexIndex].right;
			glm::vec3 newVertex = vertex + averageNeighborEdgePerp * agentRadius;

			results.push_back(newVertex);
		}
		
	}
	
	
	// portals here include start and end.
	// we want to do the smoothed paths on the original Portals
	FunnelResult Funnel_withSmoothPaths(
		std::vector<NavMesh::Portal> portals,
		std::vector<NavMesh::Portal> originalPortals,
		glm::vec3 start, glm::vec3 end,
		std::vector<glm::vec3> leftVertices,
		std::vector<glm::vec3> rightVertices,
		std::vector<EdgeIndex> indices,
		float agentRadius)
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
					// try smooth the path around the corner
					AddSmoothedPoints(
						results,
						funnel,
						originalPortals,
						leftVertices,
						indices,
						true,
						agentRadius);
						

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
					// try smooth the path around the corner
					AddSmoothedPoints(
						results,
						funnel,
						originalPortals,
						rightVertices,
						indices,
						false,
						agentRadius);
						

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
		if (results.size() > 1)
		{
			if (!Math::Equals(results[results.size() - 1], end))
			{
				results.push_back(end);
			}
		}

		FunnelResult funnelResult;
		funnelResult.waypoints = results;

		return funnelResult;
	}
	


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
		if (results.size() > 1)
		{
			if (!Math::Equals(results[results.size() - 1], end))
			{
				results.push_back(end);
			}
		}

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
		CDT::Graph* graph, 
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


	// https://github.com/dotsnav/dotsnav
	// https://www.gamedev.net/forums/topic/548610-modified-funnel-algorithm/
	// http://ahamnett.blogspot.com/2012/10/funnel-algorithm.html
	FunnelResult ModifiedFunnelPath_ANEMethod(
		NavMesh::DualGraph* dualGraph,
		CDT::Graph* graph,
		std::vector<int> aStarNodeIds,
		glm::vec3 start, glm::vec3 end, 
		float agentRadius,
		PathFinding::DebugState* debugState)
	{
		std::vector<NavMesh::Portal> portals;
		std::vector<EdgeIndex> indices;

		NavMesh::Portal startPortal = { start, start };
		portals.push_back(startPortal);

		dualGraph->AddToPortalList(graph, aStarNodeIds, portals);

		NavMesh::Portal endPortal = { end, end };
		portals.push_back(endPortal);

		debugState->portals = portals;


		std::vector<glm::vec3> verticesL;
		std::vector<glm::vec3> verticesR;

		for (int i = 0; i < portals.size(); i++)
		{
			if (i == 0)
			{
				verticesL.push_back(portals[i].left);
				verticesR.push_back(portals[i].right);

				indices.push_back(EdgeIndex(i, i));
			}
			else
			{
				int index0 = verticesL.size() - 1;
				int index1 = verticesR.size() - 1;

				EdgeIndex edgeIndex = EdgeIndex(index0, index1);

				glm::vec3 leftV = verticesL[index0];
				glm::vec3 rightV = verticesR[index1];

				if (!Math::Equals(leftV, portals[i].left))
				{
					verticesL.push_back(portals[i].left);
					edgeIndex.leftIndex += 1;
				}

				if (!Math::Equals(rightV, portals[i].right))
				{
					verticesR.push_back(portals[i].right);
					edgeIndex.rightIndex += 1;
				}

				indices.push_back(edgeIndex);
			}
		}

		std::cout << "leftVertices " << verticesL.size() << std::endl;
		for (int i = 0; i < verticesL.size(); i++)
		{
			std::cout << verticesL[i].x << " " << verticesL[i].y << " " << verticesL[i].z << std::endl;
		}

		std::cout << "rightVertices " << verticesR.size() << std::endl;
		for (int i = 0; i < verticesR.size(); i++)
		{
			std::cout << verticesR[i].x << " " << verticesR[i].y << " " << verticesR[i].z << std::endl;
		}

		std::cout << "agentRadius " << agentRadius << std::endl;

		std::vector<PathFinding::Vector> leftPerp;
		std::vector<PathFinding::Vector> rightPerp;

		// https://www.youtube.com/watch?v=rWHrTrigIYo&ab_channel=jdtec01
		// start from 2:20
		// the first and last point is start and end, so we dont care
		for (int i = 1; i < verticesL.size() - 1 ; i++)
		{
			glm::vec3 oldVertex = verticesL[i];

			// for eac vertex. gets its neighbor edges
			glm::vec3 p0 = verticesL[i - 1];
			glm::vec3 p1 = verticesL[i];
			glm::vec3 p2 = verticesL[i + 1];

			// Get the average of neighbor edge directions
			// I think I can just directly get the perp (instead of following the blog and getting the averageNeighborEdge first)
			glm::vec3 dir1 = glm::normalize(p1 - p0);
			glm::vec3 dir2 = glm::normalize(p2 - p1);

			glm::vec3 averageNeighborEdge = dir1 + dir2;

			glm::vec3 averageNeighborEdgePerp = glm::cross(averageNeighborEdge, glm::vec3(0, 0, 1));
			averageNeighborEdgePerp = glm::normalize(averageNeighborEdgePerp);

			glm::vec3 newVertex = verticesL[i] + averageNeighborEdgePerp * agentRadius;
			verticesL[i] = newVertex;


			PathFinding::Vector vec;
			vec.v0 = oldVertex;
			vec.v1 = newVertex;

			leftPerp.push_back(vec);
		}


		for (int i = 1; i < verticesR.size() - 1; i++)
		{
			glm::vec3 oldVertex = verticesR[i];

			// for eac vertex. gets its neighbor edges
			glm::vec3 p0 = verticesR[i - 1];
			glm::vec3 p1 = verticesR[i];
			glm::vec3 p2 = verticesR[i + 1];

			// Get the average of neighbor edge directions
			// I think I can just directly get the perp (instead of following the blog and getting the averageNeighborEdge first)
			glm::vec3 dir1 = glm::normalize(p1 - p0);
			glm::vec3 dir2 = glm::normalize(p2 - p1);

			glm::vec3 averageNeighborEdge = dir1 + dir2;

			glm::vec3 averageNeighborEdgePerp = glm::cross(averageNeighborEdge, glm::vec3(0, 0, -1));
			averageNeighborEdgePerp = glm::normalize(averageNeighborEdgePerp);

			glm::vec3 newVertex = verticesR[i] + averageNeighborEdgePerp * agentRadius;
			verticesR[i] = newVertex;

			PathFinding::Vector vec;
			vec.v0 = oldVertex;
			vec.v1 = newVertex;

			rightPerp.push_back(vec);
		}

		debugState->newLeftVertices = verticesL;
		debugState->newRightVertices = verticesR;

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
				portal.left = verticesL[edgeIndex.leftIndex];
				portal.right = verticesR[edgeIndex.rightIndex];

				modifiedPortals.push_back(portal);
			}
		}

		debugState->modifiedPortals = modifiedPortals;

		return Funnel_Core(modifiedPortals, start, end);
	}



	// just reducing the edges by radius, not doing the ANE method
	FunnelResult ModifiedFunnelPath2(
		NavMesh::DualGraph* dualGraph,
		CDT::Graph* graph,
		std::vector<int> aStarNodeIds,
		glm::vec3 start, glm::vec3 end,
		float agentRadius,
		PathFinding::DebugState* debugState)
	{
		std::vector<NavMesh::Portal> portals;

		NavMesh::Portal startPortal = { start, start };
		portals.push_back(startPortal);

		dualGraph->AddToPortalList(graph, aStarNodeIds, portals);

		NavMesh::Portal endPortal = { end, end };
		portals.push_back(endPortal);

		debugState->portals = portals;


		// unique vertices
		std::vector<glm::vec3> leftVertices;
		std::vector<glm::vec3> rightVertices;

		// index into the unique vertices. index.size() == portal.size()
		std::vector<EdgeIndex> indices;

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

		debugState->newLeftVertices = leftVertices;
		debugState->newRightVertices = rightVertices;



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

				portal = portals[i];

				glm::vec3 dir = glm::normalize(portal.right - portal.left);
				
				portal.left += dir * agentRadius;
				portal.right -= dir * agentRadius;

				modifiedPortals.push_back(portal);
			}
		}

		debugState->modifiedPortals = modifiedPortals;

		return Funnel_withSmoothPaths(modifiedPortals, portals, start, end, leftVertices, rightVertices, indices, agentRadius);
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

		AStarSearchResult aStarResult = AStarSearch(dualGraph, agentDiameter, node0, start, node1, goal, debugState);



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


		if (aStarResult.nodePath.size() > 0)
		{
			float radius = agentDiameter / 2.0f;
			// ModifiedFunnelPath_ANEMethod
			// ModifiedFunnelPath2
			FunnelResult funnelResult = ModifiedFunnelPath2(dualGraph, world->cdTriangulationGraph, aStarResult.nodePath, start, aStarResult.finalEnd, radius, debugState);
			result.waypoints = funnelResult.waypoints;
		}
		else
		{
			result.waypoints.push_back(start);
			result.waypoints.push_back(aStarResult.finalEnd);
		}

		// FunnelResult funnelResult = FunnelPath(dualGraph, world->cdTriangulationGraph, aStarResult.nodePath, start, finalDestination, debugState);

		/*
		if (funnelResult.waypoints.size() == 3)
		{
			for (int i = 0; i < funnelResult.waypoints.size(); i++)
			{
				std::cout << "waypoints " << funnelResult.waypoints[i].x << " " << funnelResult.waypoints[i].y << std::endl;
			}
				
			funnelResult = ModifiedFunnelPath(dualGraph, world->cdTriangulationGraph, aStarResult.nodePath, start, finalDestination, radius, debugState);
		}
		*/



		/*
		FunnelResult funnelResult = Funnel(dualGraph, world->cdTriangulationGraph, aStarResult.nodePath, start, finalDestination);
			
		result.portals = funnelResult.portals;
		debugState->portals = result.portals;

		float radius = agentDiameter / 2.0f  ;
		ModifiedFunnel(dualGraph, world->cdTriangulationGraph, aStarResult.nodePath, start, finalDestination, radius, debugState);

		result.waypoints = funnelResult.waypoints;
		*/
		

		return result;
	}
}