#pragma once

#include "../PlatformShared/platform_shared.h"
#include "collision.h"
#include "triangulation.h"

#include <iostream>
#include <vector>

// constrained delaunay triangulation
namespace CDTriangulation
{
	const int NUM_TRIANGLE_VERTEX = 3;
	const int NUM_TRIANGLE_EDGES = 3;
	const int INVALID_NEIGHBOR = -1;

	// Just has a DebugId for conveniences
	struct Vertex
	{
		int id;
		glm::vec3 pos;
	};

	struct DelaunayTriangleEdge {
		int vertices[2];
	};


	struct DelaunayTriangle {
		int id;
		Vertex vertices[3];
		int neighbors[3];	// neighbor triangle id

		DelaunayTriangleEdge edges[3];
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
			return -1;
		}

	};

	struct Bin
	{
		int id;
		std::vector<int> points;
	};


	struct DebugState
	{
		std::vector<DelaunayTriangle> triangles;
		std::vector<glm::vec3> vertices;

		std::vector<Triangulation::Circle> circles;
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

	// assume all points are between [0, 256]
	std::vector<Vertex> GetVerices(
		std::vector<glm::vec3> points)
	{
		std::vector<Vertex> vertices;
		for (int i = 0; i < points.size(); i++)
		{
			Vertex vertex = { i, points[i] };
			vertices.push_back(vertex);
			std::cout << vertex.pos.x << " " << vertex.pos.y << std::endl;
		}

		return vertices;
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



	DelaunayTriangle CreateSuperTriangle(std::vector<Vertex> vertices)
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
		return -1;
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
	
	*/

	void SwapDiagonalEdges(DelaunayTriangle& a, DelaunayTriangle& b)
	{
		Vertex p = a.vertices[0];
		Vertex v1 = a.vertices[1];
		Vertex v2 = a.vertices[2];
		Vertex v3;
		for (int i = 0; i < NUM_TRIANGLE_VERTEX; i++)
		{
			if (b.vertices[i].id != v1.id && b.vertices[i].id != v2.id)
			{
				v3 = b.vertices[i];
				break;
			}
		}


		int c = a.neighbors[2];
		int f = a.neighbors[0];

		int index = b.GetVertexIndex(v3.id);
		assert(index != -1);
		int d = b.neighbors[index];

		index = b.GetVertexIndex(v1.id);
		assert(index != -1);
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
	}



	// https://www.habrador.com/tutorials/math/14-constrained-delaunay/
	// https://forum.unity.com/threads/programming-tools-constrained-delaunay-triangulation.1066148/
	// assume all points are in [0, 256] range

	void ConstrainedDelaunayTriangulation(
		std::vector<glm::vec3> points,
		glm::ivec2 mapSize,
		DebugState* debugState)
	{
		debugState->vertices = points;

		int triangleCounter = 0;

		// not doing the normalization yet. do it later
		std::vector<Vertex> vertices = GetVerices(points);

		// build pointBinGrid
		std::vector<std::vector<Bin>> pointBinGrid = BuildPointBinGrid(vertices, mapSize);

		// super triangle
		DelaunayTriangle superTriangle = CreateSuperTriangle(vertices);
		superTriangle.id = triangleCounter++;

		// first add our super triangle to invalid list
		// we using this vector as a stack
		std::vector<int> triangleStack;
		for (int i = 0; i < NUM_TRIANGLE_VERTEX; i++)
		{
			superTriangle.vertices[i].id = vertices.size();
			vertices.push_back(superTriangle.vertices[i]);
		}

		// regenerate edges after assigning proper edges
		superTriangle.GenerateEdges();


		std::vector<DelaunayTriangle> triangles;
		triangles.push_back(superTriangle);

		std::vector<Vertex> verticesToAdd = GetVerices(points);
		std::vector<Triangulation::Circle> debugCircumCircles;

		// add the points one at a time
		for (int i = 0; i < verticesToAdd.size(); i++)
		{

			std::cout << "i " << i << std::endl;
			if (i == 5)
			{
				break;
			}


			Vertex curVertex = verticesToAdd[i];

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

			// Create 3 new triangles by connecting P to each of its vertices.
			std::vector<DelaunayTriangle> newTriangles = SplitTriangleInto3NewTriangles(triangleCounter, curVertex, containingTriangle);



			// 6. Initalize Stack. Place all three triangles on stack (this differs from the paper)
			// im using the method from the blog where im direclty push the triangle
			triangleStack.clear();
			for (int j = 0; j < newTriangles.size(); j++)
			{
				if (newTriangles[j].neighbors[1] != -1)
				{
					triangleStack.push_back(newTriangles[j].id);
				}

				triangles.push_back(newTriangles[j]);
			}

			// 7. while stack is not empty, execute swapping scheme
			while (triangleStack.size() > 0)
			{
				int mostRecentIndex = triangleStack.size() - 1;
				int curTriangleId = triangleStack[mostRecentIndex];
				triangleStack.erase(triangleStack.begin() + mostRecentIndex);

				DelaunayTriangle& curTriangle = GetTriangleReference(triangles, curTriangleId);

				int oppositeTriangleId = curTriangle.neighbors[1];
				if (oppositeTriangleId == -1)
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
					SwapDiagonalEdges(curTriangle, oppositeNeighbor);

					// add the new triangles back on the stack
					triangleStack.push_back(curTriangle.id);
					triangleStack.push_back(oppositeNeighbor.id);
				}
			}
		}

		/*
		// remove all triangles that share an edge or vertices with the original super triangle
		int curIter = 0;
		while (curIter < triangles.size())
		{
			if (SharedVertex(superTriangle, triangles[curIter]))
			{
				triangles.erase(triangles.begin() + curIter);
			}
			else
			{
				curIter++;
			}
		}
		*/


		for (int i = 0; i < triangles.size(); i++)
		{
			if (triangles[i].id == 1)
			{
				Triangulation::Circle circle = FindCircumCircle(triangles[i]);
				debugCircumCircles.push_back(circle);
			}
		}

		debugState->triangles = triangles;
		debugState->circles = debugCircumCircles;

	}
}
