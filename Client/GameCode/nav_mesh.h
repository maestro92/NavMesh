#pragma once


#include "../PlatformShared/platform_shared.h"
#include "math.h"

#include <iostream>
#include <vector>
#include <unordered_set>


namespace NavMesh
{
	static int DebugCounter = 0;




	struct Edge
	{
		glm::vec3 vertices[2];


		bool operator==(const Edge& other) const
		{
			// do epsilon test!
			return Math::Equals(vertices[0], other.vertices[0]) && Math::Equals(vertices[1], other.vertices[1]) ||
				Math::Equals(vertices[0], other.vertices[1]) && Math::Equals(vertices[1], other.vertices[0]);
		}
	};

	struct NewVertexInfo
	{
		glm::vec3 point;
		Edge edge;
	};

	struct Vertex
	{
		int debugId;
		glm::vec3 pos;
	};

	struct NavMeshPolygon
	{
		int debugId;
		// stored counter clockwise
		std::vector<glm::vec3> vertices;	

		NavMeshPolygon()
		{
			debugId = DebugCounter++;
		}

		glm::vec3 GetCentroid()
		{
			glm::vec3 sum;
			int size = vertices.size();
			for (int i = 0; i < size; i++)
			{
				sum += vertices[i];
			}

			float fSize = (float)size;
			return glm::vec3(sum.x / fSize,	sum.y / fSize, sum.z / fSize);
		}

		bool operator== (const NavMeshPolygon& p2) const {

			for (int i = 0; i < p2.vertices.size(); i++)
			{
				if (!Contains(p2.vertices[i]))
				{
					return false;
				}
			}

			return true;
		}

		bool Contains(glm::vec3 point) const
		{
			for (int i = 0; i < vertices.size(); i++)
			{
				if (Math::Equals(vertices[i], point)) {
					return true;
				}
			}
			return false;
		}

		// gets the edge between vertex and the next vertex
		Edge GetEdge(int vertexIndex)
		{
			assert(0 <= vertexIndex && vertexIndex < vertices.size());

			Edge edge;
			if (vertexIndex == vertices.size() - 1)
			{
				edge = { vertices[vertexIndex], vertices[0] };
			}
			else
			{
				edge = { vertices[vertexIndex], vertices[vertexIndex + 1] };
			}

			return edge;
		}
	};

	struct UnionPolygonGraphNode
	{
		glm::vec3 vertex;
		std::vector<glm::vec3> neighbors;

		bool ContainsNeighbor(glm::vec3 candidate)
		{
			for (int i = 0; i < neighbors.size(); i++)
			{
				if (Math::Equals(neighbors[i], vertex))
				{
					return true;
				}
			}
			return false;
		}

		void TryAddNeighbor(glm::vec3 candidate)
		{
			if (!ContainsNeighbor(candidate))
			{
				neighbors.push_back(candidate);
			}
		}
	};

	// assuming the polygon is mergeable
	struct UnionPolygonGraph
	{
		std::vector<UnionPolygonGraphNode> nodes;

		UnionPolygonGraphNode* GetGraphNode(glm::vec3 vertex)
		{
			for (int i = 0; i < nodes.size(); i++)
			{
				if (Math::Equals(nodes[i].vertex, vertex))
				{
					return &nodes[i];
				}
			}
			return NULL;
		}

		void AddPolygon(std::vector<glm::vec3>& vertices)
		{
			// validate we have all the nodes first
			for (int i = 0; i < vertices.size(); i++)
			{
				glm::vec3 vertex = vertices[i];

				// if we dont have this node yet, we create a new one
				UnionPolygonGraphNode* node = GetGraphNode(vertex);
				if (node == NULL)
				{
					UnionPolygonGraphNode newNode;
					newNode.vertex = vertex;
					nodes.push_back(newNode);
				}
			}

			// then add the neighbors
			for (int i = 0; i < vertices.size(); i++)
			{
				glm::vec3 vertex0 = vertices[i];
				glm::vec3 vertex1;
				// we only process the next vertex
				if (i == vertices.size() - 1)
				{
					vertex1 = vertices[0];
				}
				else
				{
					vertex1 = vertices[i + 1];
				}

				UnionPolygonGraphNode* node0 = GetGraphNode(vertex0);
				UnionPolygonGraphNode* node1 = GetGraphNode(vertex1);

				node0->TryAddNeighbor(vertex1);
				node1->TryAddNeighbor(vertex0);
			}
		}




		std::vector<glm::vec3> GetOuterBoundary()
		{
			assert(nodes.size() > 0);

			UnionPolygonGraphNode* lowestNode = &nodes[0];
			for (int i = 0; i < nodes.size(); i++)
			{
				if (nodes[i].vertex.x < lowestNode->vertex.x)
				{
					lowestNode = &nodes[i];
				}
			}

			std::vector<glm::vec3> boundary;

			int iter = 0;
			UnionPolygonGraphNode* curNode = lowestNode;


			Edge prevEdge;
			glm::vec3 prevEdgeVector;	// normalized
			glm::vec3 nextVertex;
			double smallestAngle = 360;

			while (true)
			{
				std::cout << iter << std::endl;
				std::cout << "	Cur Node " << curNode->vertex << std::endl;

				if (iter != 0 && Math::Equals(curNode->vertex, lowestNode->vertex))
				{
					break;
				}
				else
				{
					boundary.push_back(curNode->vertex);
				}

				// for the first interation	
				// we start with the lowest left-bottom point. 
				// depending on your coordinate system, could be [minx miny] or [minx minz]
				// prioritize miny or minz
				// the fact that you are choosing the lowest y, that means you can then choose the next 
				// edge that you meet in the counter clockwise direction (becuz there isn't possibly a next edge
				// south of u). That is the smart thing about starting with the left-bottom point  
				if (iter == 0)
				{
					// we choose the next neighbor that has the smallest couter-clockwise angle
					// get all edges
					for (int i = 0; i < curNode->neighbors.size(); i++) 
					{
						std::cout << "		 " << curNode->neighbors[i] << std::endl;

						glm::vec3 vector = glm::normalize(curNode->neighbors[i] - curNode->vertex);
						double angle = atan2(vector.y, vector.x) * 180 / PI;
						std::cout << "		 angle " << angle << std::endl;

						if (angle < 0)
						{
							angle += 360;
						}

						std::cout << "		 angle " << angle << std::endl;

						if (angle < smallestAngle)
						{
							nextVertex = curNode->neighbors[i];
							smallestAngle = angle;
						}
					}
				}
				else
				{
					glm::vec3 dPrev = -prevEdgeVector;
					smallestAngle = 360;

					// get all edges
					for (int i = 0; i < curNode->neighbors.size(); i++)
					{
						if (Math::Equals(curNode->neighbors[i], prevEdge.vertices[0]))
						{
							continue;
						}

						std::cout << "		 " << curNode->neighbors[i] << std::endl;

						glm::vec3 vector = glm::normalize(curNode->neighbors[i] - curNode->vertex);


						glm::vec3 dNext = vector;

						float angle = Math::ComputeRotationAngle_XYPlane(dPrev, dNext);

						std::cout << "			angle " << angle << std::endl;

						if (angle < smallestAngle)
						{
							smallestAngle = angle;
							nextVertex = curNode->neighbors[i];
						}
					}
				}


				prevEdge = { curNode->vertex, nextVertex };
				prevEdgeVector = glm::normalize(prevEdge.vertices[1] - prevEdge.vertices[0]);
				curNode = GetGraphNode(nextVertex);
	

//				boundary.push_back(curNode->vertex);
				iter++;
			}

			return boundary;
		}
	};


	// https://stackoverflow.com/questions/563198/whats-the-most-efficent-way-to-calculate-where-two-line-segments-intersect/565282#565282
	// https://www.codeproject.com/Tips/862988/Find-the-Intersection-Point-of-Two-Line-Segments
	bool LineSegmentLineSegmentIntersection(glm::vec2 p0, glm::vec2 p1, glm::vec2 q0, glm::vec2 q1, 
			glm::vec2& intersectionPoint, float& intersectinTime0, float& intersectinTime1, bool considerCollinearOverlapAsIntersect)
	{
		glm::vec2 r = p1 - p0;
		glm::vec2 s = q1 - q0;

		float rxs = Math::Cross(r, s);
		float qpxr = Math::Cross(q0 - p0, r);
		float qpxs = Math::Cross(q0 - p0, s);

		// if r x s = 0 and (q-p) x r = 0, then the two lines are collinear
		if (Math::IsZero(rxs) && Math::IsZero(qpxr))
		{
			if (considerCollinearOverlapAsIntersect)
			{
				float qpdotr = glm::dot(q0 - p0, r);
				float rdotr = glm::dot(r, r);

				float qpdots = glm::dot(p0 - q0, s);
				float sdots = glm::dot(s, s);

				if ((0 <= qpdotr && qpdotr <= rdotr) ||
					(0 <= qpdots && qpdots <= sdots)) 
				{
					return true;
				}
			}
			return false;
		}

		if (Math::IsZero(rxs) && !Math::IsZero(qpxr))
		{
			return false;
		}

		float t = qpxs / rxs;
		float u = qpxr / rxs;

		if (!Math::IsZero(rxs) && (0 <= t && t <= 1) && (0 <= u && u <= 1))
		{
			intersectionPoint = p0 + r * t;
			intersectinTime0 = t;
			intersectinTime1 = u;
			return true;
		}

		return false;
	}




	// https://stackoverflow.com/questions/563198/whats-the-most-efficent-way-to-calculate-where-two-line-segments-intersect/565282#565282
	// https://www.codeproject.com/Tips/862988/Find-the-Intersection-Point-of-Two-Line-Segments
	bool LineSegmentLineSegmentIntersection_NotCountingVertex(glm::vec2 p0, glm::vec2 p1, glm::vec2 q0, glm::vec2 q1,
		glm::vec2& intersectionPoint, float& intersectinTime0, float& intersectinTime1, bool considerCollinearOverlapAsIntersect)
	{
		glm::vec2 r = p1 - p0;
		glm::vec2 s = q1 - q0;

		float rxs = Math::Cross(r, s);
		float qpxr = Math::Cross(q0 - p0, r);
		float qpxs = Math::Cross(q0 - p0, s);

		// if r x s = 0 and (q-p) x r = 0, then the two lines are collinear
		if (Math::IsZero(rxs) && Math::IsZero(qpxr))
		{
			if (considerCollinearOverlapAsIntersect)
			{
				float qpdotr = glm::dot(q0 - p0, r);
				float rdotr = glm::dot(r, r);

				float qpdots = glm::dot(p0 - q0, s);
				float sdots = glm::dot(s, s);

				if ((0 <= qpdotr && qpdotr <= rdotr) ||
					(0 <= qpdots && qpdots <= sdots))
				{
					return true;
				}
			}
			return false;
		}

		if (Math::IsZero(rxs) && !Math::IsZero(qpxr))
		{
			return false;
		}

		float t = qpxs / rxs;
		float u = qpxr / rxs;

		if (!Math::IsZero(rxs) && (0 < t && t < 1) && (0 < u && u < 1))
		{
			intersectionPoint = p0 + r * t;
			intersectinTime0 = t;
			intersectinTime1 = u;
			return true;
		}

		return false;
	}



	/*
	float ClosestPtSegmentSegment(glm::vec3 p0, glm::vec3 p1, glm::vec3 q0, glm::vec3 q1, float& t, float& s, )
	*/


	std::vector<glm::vec3> GetNavMeshPolygonVertices(glm::vec3 min, glm::vec3 max)
	{
		/*
			y
			^
			|
			|
			|
			|
			|	    p0 ------------	p1
		   (-x,-y,-z)\  	(x,-y,-z)\
			|		  \			      \
			|		   \			   \
			|			p3 ------------ p2 (x,-y,z)
			|         (-x,-y,z)
			|
			------------------------------------------> x
			\
			 \
			  \
			   V z
		*/

		// counter clockwise. we using the right hand rule.
		// cross product pointing in +y direction 
		glm::vec3 p0 = glm::vec3(min.x, min.y, min.z);
		glm::vec3 p1 = glm::vec3(max.x, min.y, min.z);
		glm::vec3 p2 = glm::vec3(max.x, min.y, max.z);
		glm::vec3 p3 = glm::vec3(min.x, min.y, max.z);

		return { p2, p1, p0, p3 };
	}

	NavMeshPolygon CreatePolygonFromMinMax(glm::vec3 min, glm::vec3 max)
	{
		NavMeshPolygon polygon;
		polygon.vertices = GetNavMeshPolygonVertices(min, max);
		return polygon;
	}




	bool ContainsVertex(std::vector<glm::vec3>& list, glm::vec3 vertex)
	{
		for (int i = 0; i < list.size(); i++)
		{
			if (Math::Equals(list[i], vertex)) {
				return true;
			}
		}
		return false;
	}

	// add it to list0
	std::vector<glm::vec3> CombineTwoListsForUniquePoints(std::vector<NewVertexInfo>& list0, std::vector<NewVertexInfo>& list1)
	{
		std::vector<glm::vec3> combinedList;

		for (int i = 0; i < list0.size(); i++)
		{
			combinedList.push_back(list0[i].point);
		}

		for (int i = 0; i < list1.size(); i++)
		{
			for (int j = 0; j < combinedList.size(); j++)
			{
				glm::vec3 newPoint = list1[i].point;
				if (ContainsVertex(combinedList, newPoint))
				{
					continue;
				}
				else
				{
					combinedList.push_back(newPoint);
				}
			}
		}

		return combinedList;
	}

	void AddNewVerticesToPolygons(std::vector<NewVertexInfo>& newVertices, NavMeshPolygon* polygon0)
	{
		// add the new vertices into polygons
		for (int i = 0; i < newVertices.size(); i++)
		{
			NewVertexInfo newVertex = newVertices[i];

			// add vertex to polygon
			for (int j = 0; j < polygon0->vertices.size(); j++)
			{
				Edge edge = (*polygon0).GetEdge(j);

				if (Collision::IsPointOnLine(edge.vertices[0], edge.vertices[1], newVertex.point))
				{
					polygon0->vertices.insert(polygon0->vertices.begin() + j + 1, newVertex.point);
					break;
				}
			}
		}

		std::cout << "sizes " << polygon0->vertices.size() << std::endl;
	}

	void TryAddNewVertexToList(float intersectionTime, std::vector<NewVertexInfo>& newVertices, Edge edge)
	{
		// if this intersectionPoint is a vertex, we dont add it
		if (intersectionTime != 0 && intersectionTime != 1)
		{
			glm::vec3 intersectionPoint3D = edge.vertices[0] + intersectionTime * (edge.vertices[1] - edge.vertices[0]);
			NewVertexInfo vertexInfo = {
				intersectionPoint3D,	// just using p0.y for now
				edge,

			};
			newVertices.push_back(vertexInfo);
		}
	}


	struct UnionPolygonResult
	{
		bool intersects;
		NavMeshPolygon polygon;
	};

	// https://stackoverflow.com/questions/2667748/how-do-i-combine-complex-polygons
	UnionPolygonResult TryUnionizePolygon(NavMeshPolygon* polygon0, NavMeshPolygon* polygon1)
	{
		UnionPolygonResult result;
		result.intersects = false;

		if ((*polygon0) == (*polygon1))
		{
			result.intersects = true;
			result.polygon = *polygon0;
			return result;
		}

		bool intersectionFound = false;

		std::vector<NewVertexInfo> newVertices0;
		std::vector<NewVertexInfo> newVertices1;

		for (int i = 0; i < polygon0->vertices.size(); i++)
		{		
			Edge edge = (*polygon0).GetEdge(i);
						
			for (int j = 0; j < polygon1->vertices.size(); j++)
			{
				Edge edge1 = (*polygon1).GetEdge(j);

				// we just doing a 2D line tests now!
				glm::vec2 p0 = glm::vec2(edge.vertices[0].x, edge.vertices[0].y);
				glm::vec2 p1 = glm::vec2(edge.vertices[1].x, edge.vertices[1].y);
				glm::vec2 p2 = glm::vec2(edge1.vertices[0].x, edge1.vertices[0].y);
				glm::vec2 p3 = glm::vec2(edge1.vertices[1].x, edge1.vertices[1].y);

				glm::vec2 intersectionPoint;
				float intersectionTime0, intersectionTime1;
				if (LineSegmentLineSegmentIntersection(p0, p1, p2, p3, intersectionPoint, intersectionTime0, intersectionTime1, true))
				{
					result.intersects = true;

					TryAddNewVertexToList(intersectionTime0, newVertices0, edge);
					TryAddNewVertexToList(intersectionTime1, newVertices1, edge1);
				}
			}
		}

		if (result.intersects)
		{
			AddNewVerticesToPolygons(newVertices0, polygon0);
			AddNewVerticesToPolygons(newVertices1, polygon1);

			// generate Graph with vertex and edge
			UnionPolygonGraph graph;
			graph.AddPolygon(polygon0->vertices);
			graph.AddPolygon(polygon1->vertices);
			
			// then find the union polygon by tracing the outer boundary

			result.polygon.vertices = graph.GetOuterBoundary();
		}
		else
		{
			// handle if p1 is inside p2

			// handle if p2 is inside p1
		}

		return result;
	}

	void RemoveNavMeshPolygon(std::vector<NavMeshPolygon>& polygons, NavMeshPolygon* polygon)
	{
		for (int i = 0; i < polygons.size(); i++)
		{
			if (polygons[i] == *polygon)
			{
				polygons.erase(polygons.begin() + i);
				return;
			}
		}
	}

	bool IntersectsWithPolygon(NavMeshPolygon& polygon, Edge edge)
	{
		for (int i = 0; i < polygon.vertices.size(); i++)
		{
			Edge edge0 = polygon.GetEdge(i);

			// we just doing a 2D line tests now!
			glm::vec2 p0 = glm::vec2(edge.vertices[0].x, edge.vertices[0].y);
			glm::vec2 p1 = glm::vec2(edge.vertices[1].x, edge.vertices[1].y);
			glm::vec2 p2 = glm::vec2(edge0.vertices[0].x, edge0.vertices[0].y);
			glm::vec2 p3 = glm::vec2(edge0.vertices[1].x, edge0.vertices[1].y);

			glm::vec2 intersectionPoint;
			float intersectionTime0, intersectionTime1;
			if (LineSegmentLineSegmentIntersection_NotCountingVertex(p0, p1, p2, p3, intersectionPoint, intersectionTime0, intersectionTime1, true))
			{
				return true;
			}
		}
		return false;
	}



	std::vector<glm::vec3> GetVertexListFromStartIndex(std::vector<glm::vec3>& list, glm::vec3 startVertex)
	{
		std::vector<glm::vec3> vertexList;
		int startIndex = 0;
		for (int i = 0; i < list.size(); i++)
		{
			if (Math::Equals(list[i], startVertex))
			{
				startIndex = i;
			}
		}

		int curIndex = startIndex;
		bool first = true;
		while (true)
		{
			if (curIndex == startIndex && first == false) {
				vertexList.push_back(list[curIndex]);
				break;
			}

			vertexList.push_back(list[curIndex]);
			curIndex++;
			first = false;

			if (curIndex == list.size())
			{
				curIndex = 0;
			}
		}

		return vertexList;
	}


	// edge[0] is the vertex from the polygon,
	// edge[1] is the vertex from the hole
	void AddHoleToPolygon(NavMeshPolygon& polygon, NavMeshPolygon& hole, Edge edge)
	{
		std::vector<glm::vec3> newVertices;
		int curIndex = 0;
		for (curIndex = 0; curIndex < polygon.vertices.size(); curIndex++)
		{
			newVertices.push_back(polygon.vertices[curIndex]);

			if (Math::Equals(polygon.vertices[curIndex], edge.vertices[0]))
			{
				break;
			}
		}

		std::vector<glm::vec3> reversedHoleVertices;
		for (int i = hole.vertices.size()-1; i >= 0; i--)
		{
			reversedHoleVertices.push_back(hole.vertices[i]);
		}

		std::vector<glm::vec3> holeVertices = GetVertexListFromStartIndex(reversedHoleVertices, edge.vertices[1]);

		// now add the hole vertices
		for (int j = 0; j < holeVertices.size(); j++)
		{
			newVertices.push_back(holeVertices[j]);
		}

		// we have to add edge[0] again. 
		for (int i = curIndex; i < polygon.vertices.size(); i++)
		{
			newVertices.push_back(polygon.vertices[i]);
		}

		polygon.vertices = newVertices;
	}


	void ConnectHole(NavMeshPolygon& polygon, NavMeshPolygon& hole)
	{
		bool connected = false;
		for (int i = 0; i < hole.vertices.size(); i++)
		{
			for (int j = 0; j < polygon.vertices.size(); j++)
			{
				Edge edge = { polygon.vertices[j], hole.vertices[i]};

				if (IntersectsWithPolygon(polygon, edge))
				{
					continue;
				}

				if (IntersectsWithPolygon(hole, edge))
				{
					continue;
				}

				connected = true;
				AddHoleToPolygon(polygon, hole, edge);
				break;
			}

			if (connected)
			{
				break;
			}
		}
	}

	void ConnectHoles(NavMeshPolygon& ground, std::vector<NavMeshPolygon>& holes)
	{
		for (int i = 0; i < holes.size(); i++)
		{
			ConnectHole(ground, holes[i]);
		}
	}

	void TryUnionizePolygons(std::vector<NavMeshPolygon>& polygons)
	{
		while (true)
		{
			int count = polygons.size();

			if (count == 1)
			{
				break;
			}

			bool intersectionFound = false;
			for (int i = 0; i < count; i++)
			{
				for (int j = i + 1; j < count; j++)
				{
					NavMeshPolygon* polygon0 = &polygons[i];
					NavMeshPolygon* polygon1 = &polygons[j];

					UnionPolygonResult result = TryUnionizePolygon(polygon0, polygon1);
					intersectionFound = result.intersects;

					if (result.intersects)
					{
						// want to remove from the back, so we remove j first, then i
						RemoveNavMeshPolygon(polygons, polygon1);
						RemoveNavMeshPolygon(polygons, polygon0);

						polygons.push_back(result.polygon);
						break;
					}
				}

				if (intersectionFound)
				{
					break;
				}
			}

			if (!intersectionFound) {
				break;
			}
		}
	}


	struct DualGraphNode {
		
		struct Neighbor
		{
			int id;
			std::vector<Edge> portals;
		};
		
		int id;
		glm::vec3 center;
		NavMeshPolygon* navMeshPolygon;
		std::vector<Neighbor> neighbors;
		
		void AddNeighbor(int neighbor, std::vector<Edge> sharedEdges)
		{
			for (int i = 0; i < neighbors.size(); i++)
			{
				if (neighbors[i].id == neighbor)
				{
					return;
				}
			}

			Neighbor neighborNode = { neighbor, sharedEdges };
			neighbors.push_back(neighborNode);
		}
	};

	struct DualGraph
	{
		std::vector<NavMeshPolygon> polygons;
		std::vector<DualGraphNode> nodes;

		DualGraph(std::vector<NavMeshPolygon>& polygonsIn)
		{
			for (int i = 0; i < polygonsIn.size(); i++)
			{
				NavMeshPolygon* polygon = &polygonsIn[i];

				DualGraphNode node;
				node.id = i;
				node.center = polygon->GetCentroid();
				node.navMeshPolygon = polygon;
				nodes.push_back(node);

				polygons.push_back(*polygon);
			}

			for (int i = 0; i < polygons.size(); i++)
			{
				NavMeshPolygon* polygon0 = &polygons[i];

				for (int j = i+1; j < polygons.size(); j++)
				{
					NavMeshPolygon* polygon1 = &polygons[j];
					
					std::vector<Edge> sharedEdges;
					if (AreNeighbors(polygon0, polygon1, sharedEdges))
					{
						nodes[i].AddNeighbor(j, sharedEdges);
						nodes[j].AddNeighbor(i, sharedEdges);
					}
				}
			}
		}

		DualGraphNode* GetNode(int id)
		{
			assert(0 <= id && id < nodes.size());
			return &nodes[id];
		}


		bool AreNeighbors(NavMeshPolygon* p0, NavMeshPolygon* p1, std::vector<Edge>& sharedEdges)
		{
			for (int i = 0; i < p0->vertices.size(); i++)
			{
				Edge edge0 = p0->GetEdge(i);

				for (int j = 0; j < p1->vertices.size(); j++)
				{
					Edge edge1 = p1->GetEdge(j);

					if (edge0 == edge1)
					{
						sharedEdges.push_back(edge0);
						return true;
					}
				}
			}

			return false;
		}


		void DedupAddPortal(std::vector<Edge>& portals, Edge edge)
		{
			for (int i = 0; i < portals.size(); i++)
			{
				if (portals[i] == edge)
				{
					return;
				}
			}
			portals.push_back(edge);
		}

		std::vector<Edge> GetPortalList(std::vector<int> nodeIds)
		{
			std::vector<Edge> portals;
			for (int i = 0; i < nodeIds.size(); i++)
			{
				int nodeId = nodeIds[i];
				DualGraphNode* node = &nodes[nodeId];

				for (int j = 0; j < node->neighbors.size(); j++)
				{
					DualGraphNode::Neighbor* neighbor = &node->neighbors[j];

					for (int k = 0; k < neighbor->portals.size(); k++)
					{
						DedupAddPortal(portals, neighbor->portals[k]);
					}
				}
			}
			return portals;
		}

	};

};
