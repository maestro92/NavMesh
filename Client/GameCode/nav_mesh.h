#pragma once


#include "../PlatformShared/platform_shared.h"
#include "math.h"

#include <iostream>
#include <vector>
#include <unordered_set>


namespace NavMesh
{

	struct GraphNode
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

	struct NavMeshPolygon
	{
		// stored clockwise
		std::vector<glm::vec3> vertices;

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

	// assuming the polygon is mergeable
	struct Graph
	{
		std::vector<GraphNode> nodes;

		GraphNode* GetGraphNode(glm::vec3 vertex)
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
				GraphNode* node = GetGraphNode(vertex);
				if (node == NULL)
				{
					GraphNode newNode;
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

				GraphNode* node0 = GetGraphNode(vertex0);
				GraphNode* node1 = GetGraphNode(vertex1);

				node0->TryAddNeighbor(vertex1);
				node1->TryAddNeighbor(vertex0);
			}
		}




		std::vector<glm::vec3> GetOuterBoundary()
		{
			assert(nodes.size() > 0);

			GraphNode* lowestNode = &nodes[0];
			for (int i = 0; i < nodes.size(); i++)
			{
				if (nodes[i].vertex.z < lowestNode->vertex.z)
				{
					lowestNode = &nodes[i];
				}
			}

			std::vector<glm::vec3> boundary;

			int iter = 0;
			GraphNode* curNode = lowestNode;
			boundary.push_back(curNode->vertex);
			Edge prevEdge;
			glm::vec3 prevEdgeVector;	// normalized
			glm::vec3 nextVertex;
			double smallestAngle = 360;

			while (true)
			{
				std::cout << iter << std::endl;
				if (iter != 0 && Math::Equals(curNode->vertex, lowestNode->vertex))
				{
					break;
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
						glm::vec3 vector = glm::normalize(curNode->neighbors[i] - curNode->vertex);
						double angle = atan2(vector.z, vector.x) * 180 / PI;
						if (angle < 0)
						{
							angle += 360;
						}

						if (angle < smallestAngle)
						{
							nextVertex = curNode->neighbors[i];
							smallestAngle = angle;
							prevEdge = {curNode->vertex, nextVertex};
							prevEdgeVector = glm::normalize(nextVertex - curNode->vertex);
							curNode = GetGraphNode(nextVertex);
						}
					}
				}
				else
				{
					// get all edges
					for (int i = 0; i < curNode->neighbors.size(); i++)
					{
						if (Math::Equals(curNode->neighbors[i], prevEdge.vertices[0]))
						{
							continue;
						}

						glm::vec3 vector = glm::normalize(curNode->neighbors[i] - curNode->vertex);

						glm::vec3 dPrev = prevEdgeVector;
						glm::vec3 dNext = vector;

						// the atan2 function return arctan y/x in the interval [-pi, +pi] radians
						double theta0 = atan2(dPrev.y, dPrev.x) * 180 / PI;
						double theta1 = atan2(dNext.y, dNext.x) * 180 / PI;

						// handle angle wrap around
						float diff = theta0 - theta1;
						float angle = (180 + diff + 360);
						while (angle > 360)
						{
							angle -= 360;
						}
						
						if (angle < smallestAngle)
						{
							smallestAngle = angle;
							nextVertex = curNode->neighbors[i];
							prevEdge = { curNode->vertex, nextVertex };
							prevEdgeVector = glm::normalize(nextVertex - curNode->vertex);
							curNode = GetGraphNode(nextVertex);
						}
					}
				}


				boundary.push_back(curNode->vertex);
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
		NavMeshPolygon polygon = {
			GetNavMeshPolygonVertices(min, max)
		};
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
			for (int j = 0; i < polygon0->vertices.size(); j++)
			{
				Edge edge = (*polygon0).GetEdge(j);

				// if (newVertex.edge == edge)
				if (newVertex.edge == edge)
				{
					polygon0->vertices.insert(polygon0->vertices.begin() + j, newVertex.point);
					break;
				}
			}
		}
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


	struct MergePolygonResult
	{
		bool intersects;
		NavMeshPolygon polygon;
	};

	// https://stackoverflow.com/questions/2667748/how-do-i-combine-complex-polygons
	MergePolygonResult TryMergePolygon(NavMeshPolygon* polygon0, NavMeshPolygon* polygon1)
	{
		MergePolygonResult result;
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
				glm::vec2 p0 = glm::vec2(edge.vertices[0].x, edge.vertices[0].z);
				glm::vec2 p1 = glm::vec2(edge.vertices[1].x, edge.vertices[1].z);
				glm::vec2 p2 = glm::vec2(edge1.vertices[0].x, edge1.vertices[0].z);
				glm::vec2 p3 = glm::vec2(edge1.vertices[1].x, edge1.vertices[1].z);

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
			Graph graph;
			graph.AddPolygon(polygon0->vertices);
			graph.AddPolygon(polygon1->vertices);
			
			// then find the union polygon by tracing the outer boundary

		//	result.polygon.vertices = graph.GetOuterBoundary();
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

	void TryMergePolygons(std::vector<NavMeshPolygon>& polygons)
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

					MergePolygonResult result = TryMergePolygon(polygon0, polygon1);
					intersectionFound = result.intersects;

					if (result.intersects)
					{
					//	RemoveNavMeshPolygon(polygons, polygon0);
						RemoveNavMeshPolygon(polygons, polygon1);

					//	polygons.push_back(result.polygon);
						break;
					}
				}

				if (intersectionFound)
				{
					break;
				}
			}

			break;

			if (!intersectionFound) {
				break;
			}
		}
	}
};
