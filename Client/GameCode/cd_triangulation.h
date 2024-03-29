#pragma once

#include "../PlatformShared/platform_shared.h"
#include "collision.h"
#include "triangulation.h"
#include "geometry_core.h"

#include <queue>          // std::queue
#include <iostream>
#include <vector>
#include <string>

typedef int TriangleId;


// constrained delaunay triangulation
namespace CDT
{
	const int NUM_TRIANGLE_VERTEX = 3;
	const int NUM_TRIANGLE_EDGES = 3;
	const int INVALID_NEIGHBOR = -1;
	const int INVALID_INDEX = -1;
	const int INVALID_VERTEX_INDEX = -1;
	const int INVALID_EDGE_INDEX = -1;


	// Just has a DebugId for conveniences
	struct Vertex
	{
		int id;
		glm::vec3 pos;
	};

	struct DelaunayTriangleEdge {
		int vertices[2];
		bool isConstrained;

		DelaunayTriangleEdge()
		{
			isConstrained = false;
		}

		static DelaunayTriangleEdge GetInvalidEdge()
		{
			DelaunayTriangleEdge edge;
			edge.vertices[0] = INVALID_VERTEX_INDEX;
			edge.vertices[1] = INVALID_VERTEX_INDEX;
			return edge;
		}

		static bool IsValidEdge(DelaunayTriangleEdge edge)
		{
			return edge.vertices[0] != INVALID_VERTEX_INDEX && edge.vertices[1] != INVALID_VERTEX_INDEX;
		}

		friend bool operator==(const DelaunayTriangleEdge& l, const DelaunayTriangleEdge& r)
		{
			return l.vertices[0] == r.vertices[0] && l.vertices[1] == r.vertices[1] ||
				l.vertices[0] == r.vertices[1] && l.vertices[1] == r.vertices[0];
		}

		friend bool operator!=(const DelaunayTriangleEdge& l, const DelaunayTriangleEdge& r)
		{
			return !(l == r);
		}
	};

	struct HalfWidthLine
	{
		glm::vec3 p0;
		glm::vec3 p1;
		bool isInside;	// if its inside the triangle
	};

	struct DebugConstrainedEdge
	{
		std::vector<Vertex> vertices;
	};

	struct DebugConstrainedEdgePolygon
	{
		std::vector<DebugConstrainedEdge> Edges;
	};

	struct DelaunayTriangle {
		TriangleId id;
		Vertex vertices[NUM_TRIANGLE_VERTEX];
		int neighbors[NUM_TRIANGLE_EDGES];	// neighbor triangle id, got a neighbor per edge, hence 3 neighbors
		bool isObstacle;

		DelaunayTriangleEdge edges[NUM_TRIANGLE_EDGES];
		float halfWidths[NUM_TRIANGLE_VERTEX];
		HalfWidthLine halfWidthLines[NUM_TRIANGLE_EDGES];
		


		DelaunayTriangle()
		{
			isObstacle = false;
		}
		
		friend bool operator==(const DelaunayTriangle& l, const DelaunayTriangle& r)
		{
			for (int i = 0; i < NUM_TRIANGLE_VERTEX; i++)
			{
				bool foundMatch = false;
				for (int j = 0; j < NUM_TRIANGLE_VERTEX; j++)
				{
					if (l.vertices[i].id == r.vertices[j].id)
					{
						foundMatch = true;
						break;
					}
				}

				if (!foundMatch)
				{
					return false;
				}
			}
			return true;
		}

		friend bool operator!=(const DelaunayTriangle& l, const DelaunayTriangle& r)
		{
			return !(l == r);
		}

		glm::vec3 GetCenter()
		{
			glm::vec3 center = glm::vec3(0);
			for (int i = 0; i < NUM_TRIANGLE_VERTEX; i++)
			{
				center += vertices[i].pos;
			}

			center = center / 3.0f;
			return center;
		}

		void ReorganizeVertices(int diff)
		{
			assert(-2 <= diff && diff <= 2);

			// cycle the vertices
			if (diff < 0)
			{
				for (int iter = 0; iter < -diff; iter++)
				{
					Vertex tempV = vertices[0];
					DelaunayTriangleEdge tempE = edges[0];
					int tempNeighbor = neighbors[0];

					for (int i = 0; i < NUM_TRIANGLE_VERTEX; i++)
					{
						if (i == NUM_TRIANGLE_VERTEX - 1)
						{
							vertices[i] = tempV;
							edges[i] = tempE;
							neighbors[i] = tempNeighbor;
						}
						else
						{
							vertices[i] = vertices[i + 1];
							edges[i] = edges[i+1];
							neighbors[i] = neighbors[i+1];
						}
					}
				}
			}
			else if (diff > 0)
			{
				assert(false);
			}
		}

		bool GetTwoEdgesThatCornersVertex(int vertexId, std::vector<DelaunayTriangleEdge>& output) const
		{
			for (int i = 0; i < NUM_TRIANGLE_VERTEX; i++)
			{
				if (vertices[i].id == vertexId)
				{
					DelaunayTriangleEdge a;
					if (i == 0)
					{
						a = edges[NUM_TRIANGLE_EDGES - 1];
					}
					else
					{
						a = edges[i - 1];
					}
					DelaunayTriangleEdge b = edges[i];

					output.push_back(a);
					output.push_back(b);

					return true;
				}
			}

			return false;
		}


		// e0 -> v2
		// e1 -> v0
		// e2 -> v1
		bool TryGetOppositeVertex(const DelaunayTriangleEdge& edge, Vertex& vertex) const
		{
			for (int i = 0; i < NUM_TRIANGLE_EDGES; i++)
			{
				if (edges[i] == edge)
				{
					vertex = vertices[(i + 2) % 3];
					return true;
				}
			}
			return false;
		}

		// v0 -> e1
		// v1 -> e2
		// v2 -> e0
		bool TryGetOppositeEdge(const Vertex& vertex, DelaunayTriangleEdge& edge) const
		{
			for (int i = 0; i < NUM_TRIANGLE_VERTEX; i++)
			{
				if (vertices[i].id == vertex.id)
				{
					edge = edges[(i + 1) % 3];
					return true;
				}
			}
			return false;
		}

		bool ContainsEdge(const DelaunayTriangleEdge& edge) const
		{
			for (int i = 0; i < NUM_TRIANGLE_EDGES; i++)
			{
				if (edges[i] == edge)
				{
					return true;
				}
			}
			return false;
		}

		int GetEdgeIndex(const DelaunayTriangleEdge& edge) const
		{
			for (int i = 0; i < NUM_TRIANGLE_EDGES; i++)
			{
				if (edges[i] == edge)
				{
					return i;
				}
			}
			return INVALID_INDEX;
		}

		std::vector<DelaunayTriangleEdge> GetOtherEdges(DelaunayTriangleEdge edge)
		{
			std::vector<DelaunayTriangleEdge> otherEdges;
			for (int i = 0; i < NUM_TRIANGLE_EDGES; i++)
			{
				if (edges[i] != edge)
				{
					otherEdges.push_back(edges[i]);
				}
			}
			return otherEdges;
		}

		void GenerateEdges()
		{
			for (int i = 0; i < NUM_TRIANGLE_VERTEX; i++)
			{
				DelaunayTriangleEdge edge;

				if (i == NUM_TRIANGLE_VERTEX - 1)
				{
					edge.vertices[0] = vertices[i].id;
					edge.vertices[1] = vertices[0].id;
				}
				else
				{
					edge.vertices[0] = vertices[i].id;
					edge.vertices[1] = vertices[i + 1].id;
				}

				edges[i] = edge;
			}
		}

		int GetVertexIndex(int vertexId)
		{
			for (int i = 0; i < NUM_TRIANGLE_VERTEX; i++)
			{
				if (vertices[i].id == vertexId)
				{
					return i;
				}
			}
			return INVALID_INDEX;
		}

		Vertex GetVertexById(int vertexId)
		{
			return vertices[GetVertexIndex(vertexId)];
		}

		bool ContainsVertex(int vertexId)
		{
			return GetVertexIndex(vertexId) != INVALID_INDEX;
		}

		void TryRemoveNeighbor(int triangleId)
		{
			for (int i = 0; i < NUM_TRIANGLE_EDGES; i++)
			{
				if (neighbors[i] == triangleId)
				{
					neighbors[i] = INVALID_NEIGHBOR;
				}
			}
		}
	};

	struct Bin
	{
		int id;
		std::vector<int> points;
	};


	struct Graph
	{
		std::vector<glm::vec3> rawInputVertices;
		
		std::vector<gmt::Polygon> holes;
		std::vector<std::vector<Vertex>> intersectingEdges;
	//	std::vector<DebugConstrainedEdgePolygon> debugConstrainedEdgePolygons;

		std::vector<DelaunayTriangleEdge> constrainedEdges;
		std::vector<DelaunayTriangleEdge> debugAllConstrainedEdges;

		std::vector<DelaunayTriangle> triangles;
		std::vector<DelaunayTriangle*> trianglesById;
		std::vector<Vertex> masterVertexList;
		std::vector<bool> vertexConstrainedList;

		bool triangulated;

		std::vector<Triangulation::Circle> circles;

		DelaunayTriangle* highlightedTriangle;

		Vertex GetVertexById(int id)
		{
			assert(0 <= id && id < masterVertexList.size());
			return masterVertexList[id];
		}

		DelaunayTriangle* GetTriangleById(int id)
		{
			return trianglesById[id];
		}

		Graph()
		{
			triangulated = false;
		}

		void SetupForPathingFinding()
		{
			// compute the HalfWidth
			for (int i = 0; i < triangles.size(); i++)
			{
				CalculateWidthForTriangle(&triangles[i]);
			}
		}

		void CalculateWidthForTriangle(DelaunayTriangle* triangle)
		{
			if (triangle->id == 11)
			{
				int a = 1;
			}

			std::cout << ">>>> triangle " << triangle->id << std::endl;
			for (int i = 0; i < NUM_TRIANGLE_VERTEX; i++)
			{
				int ic = i;
				int ia = (i + 1) % NUM_TRIANGLE_VERTEX;
				int ib = (i + 2) % NUM_TRIANGLE_VERTEX;

				Vertex va = triangle->vertices[ia];
				Vertex vb = triangle->vertices[ib];
				Vertex vc = triangle->vertices[ic];


				DelaunayTriangleEdge ea;
				triangle->TryGetOppositeEdge(va, ea);
					
				DelaunayTriangleEdge eb;
				triangle->TryGetOppositeEdge(vb, eb);

				DelaunayTriangleEdge ec;
				triangle->TryGetOppositeEdge(vc, ec);

				HalfWidthLine halfWidthLine;
				halfWidthLine.p0 = vc.pos;
				halfWidthLine.isInside = false;

				float halfWidth = CalculateWidthForVertex(triangle, ea, eb, ec, va, vb, vc, halfWidthLine);
				triangle->halfWidths[i] = halfWidth;
				triangle->halfWidthLines[i] = halfWidthLine;

				std::cout << "			halfWidth " << halfWidth << std::endl;
			}
		}


		// https://skatgame.net/mburo/ps/thesis_demyen_2006.pdf
		// this calcualtes the width for vertexC,
		// ea and eb corners vertexC
		//
		// also note that va is the vertex opposite of ea
		// vb is the vertex opposite of eb
		// same with vc and ec
		float CalculateWidthForVertex(DelaunayTriangle* triangle, 
			DelaunayTriangleEdge ea, DelaunayTriangleEdge eb, DelaunayTriangleEdge ec,
			Vertex va, Vertex vb, Vertex vc, HalfWidthLine& halfWidthLine)
		{
			assert(ea.vertices[1] == eb.vertices[0]);
			assert(ea.vertices[1] == vc.id);

			std::cout << ">>>> Calculating vertex for " << vc.id << std::endl;

			float lengthA = Length(ea);
			float lengthB = Length(eb);

			// best distance so far
			float curLargestWidth = std::min(lengthA, lengthB);

			if (Math::IsObtuseOrRight(vc.pos, va.pos, vb.pos) || Math::IsObtuseOrRight(va.pos, vb.pos, vc.pos))
			{
				std::cout << "   Distance " << std::endl;
				return curLargestWidth;
			}
			else if (ec.isConstrained)	
			{
				std::cout << "   DistanceBetweenEdgeAndVertex " << std::endl;
				// if ec is a constrained edge in DelaunayTriangle
				glm::vec3 closestPoint;
				float dist = DistanceBetweenEdgeAndVertex(ec, vc, closestPoint);
				halfWidthLine.p1 = closestPoint;
				halfWidthLine.isInside = true;
				return dist;
			}
			else
			{
				std::cout << "   SearchWidth " << std::endl;
				return SearchWidth(triangle, vc, ec, curLargestWidth, halfWidthLine);
			}
		}

		float Length(DelaunayTriangleEdge edge)
		{
			Vertex v0 = GetVertexById(edge.vertices[0]);
			Vertex v1 = GetVertexById(edge.vertices[1]);

			glm::vec3 len = v0.pos - v1.pos;
			return glm::length(len);
		}

		float DistanceBetweenEdgeAndVertex(DelaunayTriangleEdge edge, Vertex v, glm::vec3& closestPoint)
		{
			Vertex va = GetVertexById(edge.vertices[0]);
			Vertex vb = GetVertexById(edge.vertices[1]);

			float t;
			Math::ClosestPointBetweenPointSegment(v.pos, va.pos, vb.pos, t, closestPoint);

			glm::vec3 len = v.pos - closestPoint;
			return glm::length(len);
		}

		// we are searching the closest point on edge to vertex v
		float SearchWidth(DelaunayTriangle* triangle, Vertex v, DelaunayTriangleEdge edge, float curLargestWidth, HalfWidthLine& halfWidthLine)
		{
			Vertex v0 = GetVertexById(edge.vertices[0]);
			Vertex v1 = GetVertexById(edge.vertices[1]);

			if (Math::IsObtuseOrRight(v.pos, v0.pos, v1.pos) && Math::IsObtuseOrRight(v0.pos, v1.pos, v.pos))
			{
				return curLargestWidth;
			}

			glm::vec3 closestPoint;
			float distance2 = DistanceBetweenEdgeAndVertex(edge, v, closestPoint);
			if (distance2 > curLargestWidth)
			{
				return curLargestWidth;	// Essentially case 1
			}
			else if (edge.isConstrained)
			{
				halfWidthLine.p1 = closestPoint;
				halfWidthLine.isInside = true;
				return distance2; // Essentially case 2
			}
			else
			{
				int triangleId = FindOppositeTriangle(triangle, edge);
				if (triangleId != INVALID_INDEX)
				{
					DelaunayTriangle* oppositeTriangle = GetTriangleById(triangleId);

					std::vector<DelaunayTriangleEdge> otherEdges = oppositeTriangle->GetOtherEdges(edge);
					curLargestWidth = SearchWidth(oppositeTriangle, v, otherEdges[0], curLargestWidth, halfWidthLine);

					return SearchWidth(oppositeTriangle, v, otherEdges[1], curLargestWidth, halfWidthLine);
				}
				else
				{
					return curLargestWidth;
				}
			}
		}

		// optimize this
		int FindOppositeTriangle(DelaunayTriangle* triangle, DelaunayTriangleEdge edge)
		{
			for (int i = 0; i < triangles.size(); i++)
			{
				if (triangles[i].id == triangle->id)
				{
					continue;
				}
				else
				{
					if (triangles[i].ContainsEdge(edge))
					{
						return triangles[i].id;
					}
				}
			}
			return INVALID_INDEX;
		}

		/*
		void CalculateWidthForTriangle(DelaunayTriangle* triangle)
		{
			for (int i = 0; i < NUM_TRIANGLE_VERTEX; i++)
			{

			
				Vertex centerVertex = triangle->vertices[i];
				std::vector<DelaunayTriangleEdge> edges;
				triangle->GetTwoEdgesThatCornersVertex(centerVertex.id, edges);

				Vertex v0, v1;
				bool valid = true;
				if (!triangle->TryGetOppositeVertex(edges[0], v0))
				{
					valid = false;
				}
				
				if (!triangle->TryGetOppositeVertex(edges[1], v1))
				{
					valid = false;
				}

				if (valid)
				{
					float width = CalculateWidthForVertex(triangle, edges[0], edges[1], )
				}
				
			}
		}
		*/

	};

	/*
	std::vector<Vertex> NormalizePoints(
		std::vector<glm::vec3> vertices,
		glm::ivec2 mapSize)
	{
		std::vector<Vertex> normalizedVertices;

		glm::vec3 min = glm::vec3(FLT_MAX);
		for (int i = 0; i < vertices.size(); i++)
		{
			if (vertices[i].x < min.x)
			{
				min.x = vertices[i].x;
			}
			if (vertices[i].y < min.y)
			{
				min.y = vertices[i].y;
			}
		}


		for (int i = 0; i < vertices.size(); i++)
		{
			Vertex vertex;
			vertex.id = i;
			vertex.pos.x = (vertices[i].x - min.x) / mapSize.x;
			vertex.pos.y = (vertices[i].y - min.y) / mapSize.y;

			normalizedVertices.push_back(vertex);
			std::cout << vertex.pos.x << " " << vertex.pos.y << std::endl;
		}
		return normalizedVertices;
	}
	*/

	int GetPointIndex(std::vector<Vertex>& vertexArray, glm::vec3 point)
	{
		for (int j = 0; j < vertexArray.size(); j++)
		{
			if (Math::Equals(vertexArray[j].pos, point))
			{
				return j;
			}
		}
		return INVALID_INDEX;
	}


	// assume all points are between [0, 256]
	void CreateAndAddVertices(
		std::vector<glm::vec3> points, std::vector<Vertex>& vertexArray, int& idCounter)
	{
		for (int i = 0; i < points.size(); i++)
		{
			if (GetPointIndex(vertexArray, points[i]) != INVALID_INDEX)
			{
				continue;
			}

			Vertex vertex = { idCounter++, points[i] };
			vertexArray.push_back(vertex);
		}
	}

	// assume all points are between [0, 256]
	void CreateAndAddVerticesForHole(
		gmt::Polygon& hole, std::vector<Vertex>& masterVertexArray, int& idCounter, 
		std::vector<int>& holeVertices,
		std::vector<int>& newlyAddedVertices)
	{
		for (int i = 0; i < hole.vertices.size(); i++)
		{
			int index = GetPointIndex(masterVertexArray, hole.vertices[i]);
			if (index == INVALID_INDEX)
			{
				Vertex vertex = { idCounter++, hole.vertices[i] };
				newlyAddedVertices.push_back(vertex.id);

				index = masterVertexArray.size();
				masterVertexArray.push_back(vertex);
			}
			holeVertices.push_back(masterVertexArray[index].id);
		}
	}


	bool SharedVertex(DelaunayTriangle& a, DelaunayTriangle& b)
	{
		for (int i = 0; i < NUM_TRIANGLE_VERTEX; i++)
		{
			for (int j = 0; j < NUM_TRIANGLE_VERTEX; j++)
			{
				if (a.vertices[i].id == b.vertices[j].id)
				{
					return true;
				}
			}
		}
		return false;
	}


	std::vector<std::vector<Bin>> BuildPointBinGrid(std::vector<Vertex> normalizedVertices, glm::ivec2 mapDim)
	{
		// dim of of our m x m Grid
		int m = (int)(sqrt(sqrt(normalizedVertices.size())));

		glm::vec2 cellDim = glm::vec2(mapDim.x / (float)m, mapDim.y / (float)m);



		std::vector<std::vector<Bin>> pointBinGrid;
		for (int i = 0; i < m; i++)
		{
			std::vector<Bin> row(m);
			pointBinGrid.push_back(row);
		}


		// assign bin number to each bin
		for (int y = 0; y < m; y++)
		{
			for (int x = 0; x < m; x++)
			{
				int bin = 0;
				if (y % 2 == 0)
				{
					bin = y * m + x + 1;
				}
				else
				{
					bin = (y + 1) * m - x;
				}
				pointBinGrid[y][x].id = bin;
			}
		}

		for (int i = 0; i < normalizedVertices.size(); i++)
		{
			glm::vec3 point = normalizedVertices[i].pos;

			int bx = point.x / cellDim.x;
			int by = point.y / cellDim.y;

			pointBinGrid[by][bx].points.push_back(i);
		}

		return pointBinGrid;
	}



	DelaunayTriangle CreateSuperTriangle(std::vector<Vertex>& vertices)
	{
		// create super triangle

		// we first grab the rectangle containing all the points
		// then 
		glm::vec3 min = glm::vec3(FLT_MAX);
		glm::vec3 max = glm::vec3(FLT_MIN);
		DelaunayTriangle superTriangle;
		for (int i = 0; i < vertices.size(); i++)
		{
			if (vertices[i].pos.x < min.x)
			{
				min.x = vertices[i].pos.x;
			}

			if (vertices[i].pos.x > max.x)
			{
				max.x = vertices[i].pos.x;
			}

			if (vertices[i].pos.y < min.y)
			{
				min.y = vertices[i].pos.y;
			}

			if (vertices[i].pos.y > max.y)
			{
				max.y = vertices[i].pos.y;
			}
		}


		/*
				   apex
				   .
				  / \
				 /   \
				/     \
			p2 /       \    p3
			  .---------.
			 /|         |\
			/ |         | \
		   /  |         |  \
		  /   |         |   \
	  t0 /    .---------.    \ t1
		   p0             p1


		*/


		// leave some buffer
		min = min - glm::vec3(10, 10, 0);
		max = max + glm::vec3(10, 10, 0);

		// grab out four points
		glm::vec3 p0 = min;
		glm::vec3 p1 = glm::vec3(max.x, min.y, 0);

		glm::vec3 p2 = glm::vec3(min.x, max.y, 0);
		glm::vec3 p3 = max;

		float height = max.y - min.y;
		float apexX = (min.x + max.x) / 2;
		float apexY = max.y + height;
		glm::vec3 apex = glm::vec3(apexX, apexY, 0);

		glm::vec3 dir0 = glm::normalize(apex - p2);
		glm::vec3 dir1 = glm::normalize(apex - p3);

		float oneOverStepY = 1 / dir0.y;
		glm::vec3 triangleBasePt0 = p2 - dir0 * oneOverStepY * height;

		oneOverStepY = 1 / dir1.y;
		glm::vec3 triangleBasePt1 = p3 - dir1 * oneOverStepY * height;

		DelaunayTriangle triangle;
		triangle.vertices[0].pos = triangleBasePt0;
		triangle.vertices[1].pos = triangleBasePt1;
		triangle.vertices[2].pos = apex;
		triangle.GenerateEdges();

		triangle.neighbors[0] = INVALID_NEIGHBOR;
		triangle.neighbors[1] = INVALID_NEIGHBOR;
		triangle.neighbors[2] = INVALID_NEIGHBOR;

		return triangle;
	}


/*
				   V2	
				   .
				  / \
				 /   \
				/     \
			   /       \    
			  /         \
			 /           \
			/      P      \
		   /               \
		  /                 \
	     /___________________\ 
		V0                    V1
*/


	std::vector<DelaunayTriangle> SplitTriangleInto3NewTriangles(int& triangleCounter, Vertex curVertex, DelaunayTriangle& containingTriangle)
	{
		// create three new triangles
		DelaunayTriangle newTriangle0;
		// we are just going to reuse this triangle id

		newTriangle0.id = containingTriangle.id;
		newTriangle0.vertices[0] = curVertex;
		newTriangle0.vertices[1] = containingTriangle.vertices[0];
		newTriangle0.vertices[2] = containingTriangle.vertices[1];
		newTriangle0.GenerateEdges();

		DelaunayTriangle newTriangle1;
		newTriangle1.id = triangleCounter++;
		newTriangle1.vertices[0] = curVertex;
		newTriangle1.vertices[1] = containingTriangle.vertices[1];
		newTriangle1.vertices[2] = containingTriangle.vertices[2];
		newTriangle1.GenerateEdges();

		DelaunayTriangle newTriangle2;
		newTriangle2.id = triangleCounter++;
		newTriangle2.vertices[0] = curVertex;
		newTriangle2.vertices[1] = containingTriangle.vertices[2];
		newTriangle2.vertices[2] = containingTriangle.vertices[0];
		newTriangle2.GenerateEdges();

		// set the adjacency infor
		newTriangle0.neighbors[0] = newTriangle2.id;
		newTriangle0.neighbors[1] = containingTriangle.neighbors[0];
		newTriangle0.neighbors[2] = newTriangle1.id;

		newTriangle1.neighbors[0] = newTriangle0.id;
		newTriangle1.neighbors[1] = containingTriangle.neighbors[1];
		newTriangle1.neighbors[2] = newTriangle2.id;

		newTriangle2.neighbors[0] = newTriangle1.id;
		newTriangle2.neighbors[1] = containingTriangle.neighbors[2];
		newTriangle2.neighbors[2] = newTriangle0.id;

		std::vector<DelaunayTriangle> triangles;
		triangles.push_back(newTriangle0);
		triangles.push_back(newTriangle1);
		triangles.push_back(newTriangle2);
		return triangles;
	}

	int GetTriangleIndex(std::vector<DelaunayTriangle>& triangles, int id)
	{
		for (int i = 0; i < triangles.size(); i++)
		{
			if (triangles[i].id == id)
			{
				return i;
			}
		}
		return INVALID_INDEX;
	}

	DelaunayTriangle& GetTriangleReference(std::vector<DelaunayTriangle>& triangles, int id)
	{
		for (int i = 0; i < triangles.size(); i++)
		{
			if (triangles[i].id == id)
			{
				return triangles[i];
			}
		}

		// shouldn't be here
		assert(false);
		return triangles[0];
	}

	Triangulation::Circle FindCircumCircle(DelaunayTriangle triangle)
	{
		glm::vec2 a = glm::vec2(triangle.vertices[0].pos.x, triangle.vertices[0].pos.y);
		glm::vec2 b = glm::vec2(triangle.vertices[1].pos.x, triangle.vertices[1].pos.y);
		glm::vec2 c = glm::vec2(triangle.vertices[2].pos.x, triangle.vertices[2].pos.y);

		return Triangulation::FindCircumCircle(a, b, c);
	}


	void ReplaceNeighbor(DelaunayTriangle& triangle, int prevNeighbor, int newNeighbor)
	{
		for (int i = 0; i < NUM_TRIANGLE_EDGES; i++)
		{
			if (triangle.neighbors[i] == prevNeighbor)
			{
				triangle.neighbors[i] = newNeighbor;
			}
		}
	}


	/*     
		   v2                            v2

		   /|\                           / \
	   C  / | \  D                   C  /   \  D
 		 /  |  \                       /  A  \
	  P / A | B \ v3   --------->   P /_______\ v3
		\   |   /                     \       /
		 \  |  /                       \  B  /
	   F  \ | /  E                   F  \   /  E 
 		   \|/                           \ /

		   v1                            v1
	
	C,D,F,E are neighbor triangles

	this is assuming v1, v2 is the shared edge	
	*/

	void SwapDiagonalEdges(DelaunayTriangle& a, DelaunayTriangle& b, std::vector<DelaunayTriangle>& triangles)
	{
		// reorient a and b so that it fits the convention
		
		int index = 0;		
		for (int i = 0; i < NUM_TRIANGLE_VERTEX; i++)
		{
			if (!b.ContainsVertex(a.vertices[i].id))
			{
				index = i;
				break;
			}
		}

		// we want to move the non-shared vertex to [0]
		int diff = 0 - index;
		a.ReorganizeVertices(diff);


		Vertex p = a.vertices[0];
		Vertex v1 = a.vertices[1];
		Vertex v2 = a.vertices[2];
		Vertex v3;
		for (int i = 0; i < NUM_TRIANGLE_VERTEX; i++)
		{
			// if (b.vertices[i].id != v1.id && b.vertices[i].id != v2.id)
			
			if(!a.ContainsVertex(b.vertices[i].id))
			{
				v3 = b.vertices[i];
				break;
			}
		}


		int c = a.neighbors[2];
		int f = a.neighbors[0];

		index = b.GetVertexIndex(v3.id);
		assert(index != INVALID_INDEX);
		int d = b.neighbors[index];

		index = b.GetVertexIndex(v1.id);
		assert(index != INVALID_INDEX);
		int e = b.neighbors[index];


		DelaunayTriangle newA;
		newA.id = a.id;
		newA.vertices[0] = p;
		newA.vertices[1] = v3;
		newA.vertices[2] = v2;
		newA.GenerateEdges();

		DelaunayTriangle newB;
		newB.id = b.id;
		newB.vertices[0] = p;
		newB.vertices[1] = v1;
		newB.vertices[2] = v3;
		newB.GenerateEdges();

		newA.neighbors[0] = b.id;
		newA.neighbors[1] = d;
		newA.neighbors[2] = c;

		newB.neighbors[0] = f;
		newB.neighbors[1] = e;
		newB.neighbors[2] = a.id;

		a = newA;
		b = newB;

		// also fix Triangle F, D neighbor info
		if (f != INVALID_NEIGHBOR)
		{
			DelaunayTriangle& trigF = GetTriangleReference(triangles, f);
			ReplaceNeighbor(trigF, a.id, b.id);
		}

		if (d != INVALID_NEIGHBOR)
		{
			DelaunayTriangle& trigD = GetTriangleReference(triangles, d);
			ReplaceNeighbor(trigD, b.id, a.id);
		}
	}

	void MarkConstrainedEdge(DelaunayTriangle* triangle, DelaunayTriangleEdge edge)
	{
		for (int i = 0; i < NUM_TRIANGLE_EDGES; i++)
		{
			if (triangle->edges[i] == edge)
			{
				triangle->edges[i].isConstrained = true;
			}
		}
	}

	bool ContainsEdge(DelaunayTriangleEdge edge, std::vector<DelaunayTriangle>& triangles, DelaunayTriangle* &containingTriangle)
	{
		for (int i = 0; i < triangles.size(); i++)
		{
			if (triangles[i].id == 11)
			{
				int a = 1;
			}

			for (int j = 0; j < NUM_TRIANGLE_EDGES; j++)
			{
				if (triangles[i].edges[j] == edge)
				{
					containingTriangle = &triangles[i];
					return true;
				}
			}
		}
		return false;
	}

	void FindAndMarkConstrainedEdges(DelaunayTriangleEdge edge, std::vector<DelaunayTriangle>& triangles)
	{
		for (int i = 0; i < triangles.size(); i++)
		{
			if (triangles[i].id == 11)
			{
				int a = 1;
			}

			for (int j = 0; j < NUM_TRIANGLE_EDGES; j++)
			{
				if (triangles[i].edges[j] == edge)
				{
					triangles[i].edges[j].isConstrained = true;
				}
			}
		}
	}


	void AddVertexToTriangulation(Vertex curVertex, 
		std::vector<DelaunayTriangle>& triangles, 
		int& triangleCounter, 
		std::vector<int>& triangleStack)
	{
		// 5. find an existing triangle that contains the point
		bool foundTriangle = false;
		DelaunayTriangle containingTriangle;

		for (int j = 0; j < triangles.size(); j++)
		{
			DelaunayTriangle triangleToCheck = triangles[j];

			glm::vec3 pt0 = triangleToCheck.vertices[0].pos;
			glm::vec3 pt1 = triangleToCheck.vertices[1].pos;
			glm::vec3 pt2 = triangleToCheck.vertices[2].pos;

			if (Collision::IsPointInsideTriangle_Barycentric(curVertex.pos, pt0, pt1, pt2))
			{
				containingTriangle = triangleToCheck;
				foundTriangle = true;
				break;
			}
		}

		assert(foundTriangle);

		// delete this enclosed triangle, but we are still using his Id
		for (int j = 0; j < triangles.size(); j++)
		{
			if (triangles[j].id == containingTriangle.id)
			{
				triangles.erase(triangles.begin() + j);
			}
		}

		int previousContainingTriangleId = containingTriangle.id;

		// Create 3 new triangles by connecting P to each of its vertices.
		std::vector<DelaunayTriangle> newTriangles = SplitTriangleInto3NewTriangles(triangleCounter, curVertex, containingTriangle);



		// 6. Initalize Stack. Place all three triangles on stack (this differs from the paper)
		// im using the method from the blog where im direclty push the triangle
		triangleStack.clear();
		for (int j = 0; j < newTriangles.size(); j++)
		{
			if (newTriangles[j].neighbors[1] != INVALID_NEIGHBOR)
			{
				// fix up the original neighbors
				DelaunayTriangle& neighbor = GetTriangleReference(triangles, newTriangles[j].neighbors[1]);
				ReplaceNeighbor(neighbor, previousContainingTriangleId, newTriangles[j].id);

				triangleStack.push_back(newTriangles[j].id);
			}

			triangles.push_back(newTriangles[j]);
		}

		/*
		if (returnEarly)
		{
			return;
		}
		*/

		// 7. while stack is not empty, execute swapping scheme
		while (triangleStack.size() > 0)
		{
			int mostRecentIndex = triangleStack.size() - 1;
			int curTriangleId = triangleStack[mostRecentIndex];
			triangleStack.erase(triangleStack.begin() + mostRecentIndex);

			DelaunayTriangle& curTriangle = GetTriangleReference(triangles, curTriangleId);

			DelaunayTriangleEdge sharedEdge = curTriangle.edges[1];
			int oppositeTriangleId = curTriangle.neighbors[1];
			if (oppositeTriangleId == INVALID_NEIGHBOR)
			{
				continue;
			}

			DelaunayTriangle& oppositeNeighbor = GetTriangleReference(triangles, oppositeTriangleId);

			// circum circle test
			Triangulation::Circle circle = FindCircumCircle(oppositeNeighbor);
			glm::vec2 center = glm::vec2(circle.center.x, circle.center.y);
			if (Collision::IsPointInsideCircle(center, circle.radius, glm::vec2(curVertex.pos.x, curVertex.pos.y)))
			{
				// do the swapping
				SwapDiagonalEdges(curTriangle, oppositeNeighbor, triangles);

				// add the new triangles back on the stack
				triangleStack.push_back(curTriangle.id);
				triangleStack.push_back(oppositeNeighbor.id);
			}
		}
	}

	bool IsPointOnTheLeftOfLineSegment(glm::vec3 point, glm::vec3 a, glm::vec3 b)
	{
		glm::vec2 point2D = glm::vec2(point.x, point.y);
		glm::vec2 lineA = glm::vec2(a.x, a.y);
		glm::vec2 lineB = glm::vec2(b.x, b.y);
		return Collision::IsPointOnTheLeftOfLineSegment2D(point2D, lineA, lineB);
	}


	bool IsPointOnTheRightOfLineSegment(glm::vec3 point, glm::vec3 a, glm::vec3 b)
	{
		glm::vec2 point2D = glm::vec2(point.x, point.y);
		glm::vec2 lineA = glm::vec2(a.x, a.y);
		glm::vec2 lineB = glm::vec2(b.x, b.y);
		return Collision::IsPointOnTheRightOfLineSegment2D(point2D, lineA, lineB);
	}

	struct IntersectedEdge
	{
		int triangleId;	// this is for debugging only, dont use it for any computation
		DelaunayTriangleEdge edge;
	};

	std::vector<IntersectedEdge> GetEdgesInteserctedByEdge(
		DelaunayTriangleEdge constrainedEdge,
		std::vector<DelaunayTriangle>& triangles, 
		const std::vector<Vertex>& masterVertexArray)
	{
		Vertex vStart = masterVertexArray[constrainedEdge.vertices[0]];
		Vertex vEnd = masterVertexArray[constrainedEdge.vertices[1]];

		// 5.3.1 search for that triangle that "Contains" the beginning of the new edge constrainedEdge
		DelaunayTriangle containingTriangle;
		bool found = false;
		for (int i = 0; i < triangles.size(); i++)
		{
			std::vector<DelaunayTriangleEdge> edges;

			if (triangles[i].GetTwoEdgesThatCornersVertex(vStart.id, edges))
			{
				// std::cout << "		Examining " << triangles[i].id << std::endl;
				Vertex e0v0 = masterVertexArray[edges[0].vertices[0]];
				Vertex e0v1 = masterVertexArray[edges[0].vertices[1]];

				Vertex e1v0 = masterVertexArray[edges[1].vertices[0]];
				Vertex e1v1 = masterVertexArray[edges[1].vertices[1]];

				bool b0 = IsPointOnTheLeftOfLineSegment(vEnd.pos, e0v0.pos, e0v1.pos);
				bool b1 = IsPointOnTheLeftOfLineSegment(vEnd.pos, e1v0.pos, e1v1.pos);

				if (IsPointOnTheLeftOfLineSegment(vEnd.pos, e0v0.pos, e0v1.pos) &&
					IsPointOnTheLeftOfLineSegment(vEnd.pos, e1v0.pos, e1v1.pos)){
						
					containingTriangle = triangles[i];
					found = true;
					std::cout << "				containingTriangle " << containingTriangle.id << std::endl;
					break;
				}
			}
		}

		// 5.3.2 get all the triangle edges that intersects by the constrainedEdge
		DelaunayTriangle curTriangle = containingTriangle;	
		std::vector<IntersectedEdge> intersectingEdges;
		
		if (curTriangle.id == 26)
		{
			int a = 1;
		}
		
		if (!found)
		{
			return intersectingEdges;
		}

		while (true)
		{
			if (Collision::IsPointInsideTriangle_Barycentric(
				vEnd.pos,
				curTriangle.vertices[0].pos,
				curTriangle.vertices[1].pos,
				curTriangle.vertices[2].pos))
			{
				break;
			}

			std::cout << "		curTriangle.id " << curTriangle.id << std::endl;

			for (int i = 0; i < NUM_TRIANGLE_EDGES; i++)
			{
				DelaunayTriangleEdge edge = curTriangle.edges[i];

				Vertex v0 = masterVertexArray[edge.vertices[0]];
				Vertex v1 = masterVertexArray[edge.vertices[1]];

			//	std::cout << "				Examining edge v0 " << v0.pos.x << " " << v0.pos.y << std::endl;
			//	std::cout << "						" << v1.pos.x << " " << v1.pos.y << std::endl;

				if (IsPointOnTheRightOfLineSegment(vEnd.pos, v0.pos, v1.pos)) {
					
					glm::vec2 intersectionPoint;
					if (Collision::GetLineLineIntersectionPoint_CheckOnlyXY2D(vStart.pos, vEnd.pos, v0.pos, v1.pos, intersectionPoint))
					{
						intersectingEdges.push_back({ curTriangle.id, edge });
					
						int neighborTrigId = curTriangle.neighbors[i];

						curTriangle = GetTriangleReference(triangles, neighborTrigId);
						break;
					}
				}
			}
		}

		return intersectingEdges;
	}


	void GetQuadrilateralsFromNeighbors(
		DelaunayTriangle& a, DelaunayTriangle& b, DelaunayTriangleEdge& sharedEdge,
		std::vector<Vertex>& vertices, std::vector<glm::vec3>& rawPoints)
	{
		vertices.clear();
		rawPoints.clear();

		Vertex v;	
		int index;
		
		for (int i = 0; i < NUM_TRIANGLE_VERTEX; i++)
		{
			// find the vertex that's is not in the sharedEdge
			if (a.vertices[i].id != sharedEdge.vertices[0] && 
				a.vertices[i].id != sharedEdge.vertices[1])
			{
				v = a.vertices[i];
				index = i;
				break;
			}
		}
		// v1
		vertices.push_back(v);
		rawPoints.push_back(v.pos);

		// now we find the edge that starts with this guy
		DelaunayTriangleEdge edge = a.edges[index];

		// v2
		v = a.GetVertexById(edge.vertices[1]);
		vertices.push_back(v);
		rawPoints.push_back(v.pos);

		// v3
		for (int i = 0; i < NUM_TRIANGLE_VERTEX; i++)
		{
			// find the vertex that's is not in the sharedEdge
			if (b.vertices[i].id != sharedEdge.vertices[0] &&
				b.vertices[i].id != sharedEdge.vertices[1])
			{
				v = b.vertices[i];
				index = i;
				break;
			}
		}
		vertices.push_back(v);
		rawPoints.push_back(v.pos);

		// now we find the edge that starts with this guy
		edge = b.edges[index];

		// v4
		v = b.GetVertexById(edge.vertices[1]);
		vertices.push_back(v);
		rawPoints.push_back(v.pos);
	}

	Vertex GetVertexANotInTriangleB(DelaunayTriangle& a, DelaunayTriangle& b)
	{
		for (int i = 0; i < NUM_TRIANGLE_VERTEX; i++)
		{
			if (!b.ContainsVertex(a.vertices[i].id))
			{
				return a.vertices[i];
			}
		}
		assert(false);
		return a.vertices[0];
	}


	// step 4.1 ~ 4.3 from the paper
	void CheckDelaunayCriteriaInNewlyCreatedEdges(
		DelaunayTriangleEdge constrainedEdge, 
		std::vector<DelaunayTriangleEdge>& newEdges,
		std::vector<DelaunayTriangle>& triangles)
	{
		for (int i = 0; i < newEdges.size(); i++)
		{
			if (newEdges[i] == constrainedEdge)
			{
				continue;
			}

			std::vector<DelaunayTriangle*> sharedEdgeTriangles;
			for (int j = 0; j < triangles.size(); j++)
			{
				if (triangles[j].ContainsEdge(newEdges[i]))
				{
					sharedEdgeTriangles.push_back(&triangles[j]);
				}
			}

			assert(sharedEdgeTriangles.size() == 2);

			DelaunayTriangle& a = *sharedEdgeTriangles[0];
			DelaunayTriangle& b = *sharedEdgeTriangles[1];

			Vertex vertexA = GetVertexANotInTriangleB(a, b);
			Vertex vertexB = GetVertexANotInTriangleB(b, a);

			Triangulation::Circle circleA = FindCircumCircle(a);
			glm::vec2 centerA = glm::vec2(circleA.center.x, circleA.center.y);

			Triangulation::Circle circleB = FindCircumCircle(b);
			glm::vec2 centerB = glm::vec2(circleB.center.x, circleB.center.y);

			if (Collision::IsPointInsideCircle(centerA, circleA.radius, glm::vec2(vertexB.pos.x, vertexB.pos.y))||
				Collision::IsPointInsideCircle(centerB, circleB.radius, glm::vec2(vertexA.pos.x, vertexA.pos.y)))
			{
				SwapDiagonalEdges(a, b, triangles);
			}
		}
	}

	void MarkObstacles(Graph* debugState, std::vector<gmt::Polygon> holes)
	{
		for (int i = 0; i < debugState->triangles.size(); i++)
		{
			CDT::DelaunayTriangle* triangle = &debugState->triangles[i];
			glm::vec3 center = triangle->GetCenter();

			std::cout << center.x << " " << center.y << " " << std::endl;
			for (int j = 0; j < holes.size(); j++)
			{
				if (Collision::IsPointInsidePolygon2D(center, holes[j].vertices))
				{
					triangle->isObstacle = true;
					break;
				}
			}
		}
	}

	void MarkAndGetConstrainedEdgesFromHole(
		std::vector<int> holeVertices, 
		std::vector<Vertex>& masterVertexArray,
		std::vector<DelaunayTriangle>& triangles,
		std::vector<DelaunayTriangleEdge>& constrainedEdges,
		std::vector<DelaunayTriangleEdge>& allConstrainedEdges)
	{
		for (int i = 0; i < holeVertices.size(); i++)
		{
			DelaunayTriangleEdge edge;

			Vertex v0 = masterVertexArray[holeVertices[i]];
			edge.vertices[0] = v0.id;

			if (i == holeVertices.size() - 1)
			{
				Vertex v1 = masterVertexArray[holeVertices[0]];
				edge.vertices[1] = v1.id;
			}
			else
			{
				Vertex v1 = masterVertexArray[holeVertices[i + 1]];
				edge.vertices[1] = v1.id;
			}

			Vertex constrainedEdgeStart = masterVertexArray[edge.vertices[0]];
			Vertex constrainedEdgeEnd = masterVertexArray[edge.vertices[1]];

			std::vector<Vertex> constrainedVertices;
			constrainedVertices.push_back(constrainedEdgeStart);
			constrainedVertices.push_back(constrainedEdgeEnd);

			allConstrainedEdges.push_back(edge);

			DelaunayTriangle* containingTriangle = NULL;
			if (!ContainsEdge(edge, triangles, containingTriangle))
			{
				constrainedEdges.push_back(edge);
			}
			else
			{
				MarkConstrainedEdge(containingTriangle, edge);
			}

			// if we only have one edge, we just return after creating the first edge
			if (holeVertices.size() == 2)
			{
				break;
			}
		}
	}

	void MarkHoleOverlappingEdgesConstrained(
		std::vector<Vertex>& masterVertexArray,
		std::vector<DelaunayTriangle>& triangles,
		std::vector<DelaunayTriangleEdge>& allConstrainedEdges, 
		std::vector<bool>& vertexContrainedArray)
	{
		for (int i = 0; i < allConstrainedEdges.size(); i++)
		{
			DelaunayTriangleEdge constrainedEdge = allConstrainedEdges[i];
			FindAndMarkConstrainedEdges(constrainedEdge, triangles);

			vertexContrainedArray[constrainedEdge.vertices[0]] = true;
			vertexContrainedArray[constrainedEdge.vertices[1]] = true;
		}
	}

	// https://www.habrador.com/tutorials/math/14-constrained-delaunay/
	// https://forum.unity.com/threads/programming-tools-constrained-delaunay-triangulation.1066148/
	// assume all points are in [0, 256] range
	void ConstrainedDelaunayTriangulation(
		std::vector<glm::vec3> points,
		std::vector<gmt::Polygon> holes,
		glm::ivec2 mapSize,
		Graph* debugState)
	{
		debugState->rawInputVertices = points;
		debugState->holes = holes;

		int triangleCounter = 0;

		// not doing the normalization yet. do it later

		std::vector<Vertex> masterVertexArray;
		std::vector<bool> vertexConstrainedArray;

		int idCounter = 0;
		CreateAndAddVertices(points, masterVertexArray, idCounter);

		// build pointBinGrid
	//	std::vector<std::vector<Bin>> pointBinGrid = BuildPointBinGrid(masterVertexArray, mapSize);

		// super triangle
		DelaunayTriangle superTriangle = CreateSuperTriangle(masterVertexArray);
		superTriangle.id = triangleCounter++;


		std::vector<Vertex> verticesToAdd;
		for (int i = 0; i < masterVertexArray.size(); i++)
		{
			verticesToAdd.push_back(masterVertexArray[i]);
		}

		// also add the super triangle vertex to our master vertex array
		for (int i = 0; i < NUM_TRIANGLE_VERTEX; i++)
		{
			superTriangle.vertices[i].id = idCounter++;
			masterVertexArray.push_back(superTriangle.vertices[i]);
		}

	

		// regenerate edges after assigning proper edges
		superTriangle.GenerateEdges();


		// first add our super triangle to invalid list
		// we using this vector as a stack
		std::vector<int> triangleStack;
		std::vector<DelaunayTriangle> triangles;
		triangles.push_back(superTriangle);


		// add the points one at a time
		for (int i = 0; i < verticesToAdd.size(); i++)
		{
			AddVertexToTriangulation(verticesToAdd[i], triangles, triangleCounter, triangleStack);
		}

		
		// Holes creation:
		// not doing the normalization yet. do it later
		std::vector<std::vector<int>> masterHoleVertices;

		// 5.2 add the hole points, and create new triangles
		for (int j = 0; j < holes.size(); j++)
		{
			std::vector<int> holeVertices;
			std::vector<int> newlyAddedVertices;
			CreateAndAddVerticesForHole (holes[j], masterVertexArray, idCounter, holeVertices, newlyAddedVertices);

			for (int i = 0; i < newlyAddedVertices.size(); i++)
			{
				int vertexId = newlyAddedVertices[i];
				Vertex vertex = masterVertexArray[vertexId];
				AddVertexToTriangulation(vertex, triangles, triangleCounter, triangleStack);
			}
			masterHoleVertices.push_back(holeVertices);
		}

#if 1

		// 5.3 create the constrained edges
		for (int j = 0; j < holes.size(); j++)
		{
			std::vector<int> holeVertices = masterHoleVertices[j];
			std::vector<DelaunayTriangleEdge> constrainedEdges;
			std::vector<DelaunayTriangleEdge> allConstrainedEdges;

			MarkAndGetConstrainedEdgesFromHole(holeVertices, masterVertexArray, triangles, constrainedEdges, allConstrainedEdges);

			for (int ei = 0; ei < constrainedEdges.size(); ei++)
			{
				debugState->constrainedEdges.push_back(constrainedEdges[ei]);
			}

			for (int ei = 0; ei < allConstrainedEdges.size(); ei++)
			{
				debugState->debugAllConstrainedEdges.push_back(allConstrainedEdges[ei]);
			}


			// 5.3.1 search for triangle that contains the beginning of the new edge
			for (int i = 0; i < constrainedEdges.size(); i++)
			{
				Vertex constrainedEdgeStart = masterVertexArray[constrainedEdges[i].vertices[0]];
				Vertex constrainedEdgeEnd = masterVertexArray[constrainedEdges[i].vertices[1]];

			//	std::cout << "start " << constrainedEdgeStart.id << " " << constrainedEdgeStart.pos.x << " " << constrainedEdgeStart.pos.y << std::endl;
			//	std::cout << "End " << constrainedEdgeEnd.id << " " << constrainedEdgeEnd.pos.x << " " << constrainedEdgeEnd.pos.y << std::endl;


				std::queue<IntersectedEdge> intersectedEdgesQueue;
				{
					// 5.3.2 get all the edges that intersects
					std::vector<IntersectedEdge> intersectedEdges = GetEdgesInteserctedByEdge(constrainedEdges[i], triangles, masterVertexArray);
					std::string debugString;
					for (int j = 0; j < intersectedEdges.size(); j++)
					{
						debugString += std::to_string(intersectedEdges[j].triangleId) + " ";

						Vertex v0 = masterVertexArray[intersectedEdges[j].edge.vertices[0]];
						Vertex v1 = masterVertexArray[intersectedEdges[j].edge.vertices[1]];

						std::vector<Vertex> debugEdge = { v0, v1 };
						debugState->intersectingEdges.push_back(debugEdge);

						intersectedEdgesQueue.push(intersectedEdges[j]);
					}
				}


				std::vector<DelaunayTriangleEdge> newEdges;

				// 5.3.3 form quadrilaterals and swap intersected edges
				while (!intersectedEdgesQueue.empty())
				{
					IntersectedEdge intersectedEdge = intersectedEdgesQueue.front();
					intersectedEdgesQueue.pop();

					for (int j = 0; j < triangles.size(); j++)
					{
						DelaunayTriangle& triangle = triangles[j];
						if (triangle.ContainsEdge(intersectedEdge.edge))
						{
							int edgeIndex = triangle.GetEdgeIndex(intersectedEdge.edge);
							assert(edgeIndex != INVALID_INDEX);

							int neighborTrigId = triangle.neighbors[edgeIndex];

							DelaunayTriangle& neighborTriangle = GetTriangleReference(triangles, neighborTrigId);

							std::vector<Vertex> polygonVertices;
							std::vector<glm::vec3> polygonPoints;
							GetQuadrilateralsFromNeighbors(triangle, neighborTriangle, intersectedEdge.edge, polygonVertices, polygonPoints);

							// check if its convex
							if (Collision::IsConvex(polygonPoints))
							{

								// index 0 and 2 are hardcoded
								DelaunayTriangleEdge newEdge;
								newEdge.vertices[0] = polygonVertices[0].id;
								newEdge.vertices[1] = polygonVertices[2].id;



								// we check if the swapped edge still intersects the constraied edge
								// we first check if the edge share any vertex. if so we don't count them as intersecting
								bool shareVertices = polygonVertices[0].id == constrainedEdgeStart.id ||
									polygonVertices[1].id == constrainedEdgeStart.id ||
									polygonVertices[0].id == constrainedEdgeEnd.id ||
									polygonVertices[1].id == constrainedEdgeEnd.id;

								// now check the actual intersection
								glm::vec2 intersectionPoint;
								if (!shareVertices && Collision::GetLineLineIntersectionPoint_CheckOnlyXY2D(
									polygonPoints[0],
									polygonPoints[2],
									constrainedEdgeStart.pos,
									constrainedEdgeEnd.pos, intersectionPoint))
								{
									// we intersecting! We put it back
									intersectedEdgesQueue.push(intersectedEdge);
								}
								else
								{
									SwapDiagonalEdges(triangle, neighborTriangle, triangles);
									newEdges.push_back(newEdge);
								}
							}
							else
							{
								// put it back
								intersectedEdgesQueue.push(intersectedEdge);
							}

							break;
						}
					}


				}


				// 5.3.4 checking Delaunay constraint
				CheckDelaunayCriteriaInNewlyCreatedEdges(constrainedEdges[i], newEdges, triangles);
			}


		}
#endif	
		vertexConstrainedArray.resize(masterVertexArray.size());

		// 5.4 identify all the triangles in the constraint
		MarkHoleOverlappingEdgesConstrained(masterVertexArray, triangles, debugState->debugAllConstrainedEdges, vertexConstrainedArray);


		
		// 6.0 remove all triangles that share an edge or vertices with the original super triangle
		std::vector<int> removedTriangleIds;
		int curIter = 0;
		while (curIter < triangles.size())
		{
			if (SharedVertex(superTriangle, triangles[curIter]))
			{
				removedTriangleIds.push_back(triangles[curIter].id);
				triangles.erase(triangles.begin() + curIter);
			}
			else
			{
				curIter++;
			}
		}
		
		for (int i = 0; i < triangles.size(); i++)
		{
			for (int j = 0; j < removedTriangleIds.size(); j++)
			{
				triangles[i].TryRemoveNeighbor(removedTriangleIds[j]);
			}
		}
		
		debugState->triangles = triangles;
		
		int maxId = 0;
		for (int i = 0; i < debugState->triangles.size(); i++)
		{
			maxId = std::max(maxId, debugState->triangles[i].id);
			std::cout << "i " << i << " " << debugState->triangles[i].id << std::endl;
		}

		debugState->trianglesById.resize(maxId + 1);
		for (int i = 0; i < debugState->trianglesById.size(); i++)
		{
			debugState->trianglesById[i] = NULL;
		}
		
		for (int i = 0; i < debugState->triangles.size(); i++)
		{
			int id = debugState->triangles[i].id;
			debugState->trianglesById[id] = &debugState->triangles[i];
		}

		debugState->masterVertexList = masterVertexArray;
		debugState->vertexConstrainedList = vertexConstrainedArray;

		debugState->triangulated = true;

	}
}
