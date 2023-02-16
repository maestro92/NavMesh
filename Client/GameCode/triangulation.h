#pragma once

#include "../PlatformShared/platform_shared.h"
#include "collision.h"

#include <iostream>
#include <vector>

namespace Triangulation
{

	// Just has a DebugId for conveniences
	struct Vertex
	{
		int id;
		glm::vec3 pos;
	};

	struct Edge
	{
		glm::vec3 vertices[2];
	};

	struct Triangle
	{
		// counter clockwise order
		glm::vec3 vertices[3];

		/*
		glm::vec3 v0;
		glm::vec3 v1;
		glm::vec3 v2;
		*/
	};

	struct Circle
	{
		glm::vec3 center;
		float radius;
	};

	struct DebugState
	{
		Triangle superTriangle;

		std::vector<glm::vec3> vertices;

		std::vector<Circle> circles;
	};


	/*
	got two methods here
	1.	EarClipping triangulation

	2.	Delaunay Triangulation
	
	
	*/

	// using the ear clipping algorithm
	// https://www.geometrictools.com/Documentation/TriangulationByEarClipping.pdf

	/*
	void findConvexReflexAndEars()
	{
		reflexVertices.clear();
		convexVertices.clear();
		ears.clear();
		for (int i = 0; i < vertices.size(); i++)
		{
			Vertex v = vertices[i];

			if (!isRemoved(v.id))
			{
				if (v.angle > 180)
				{
					reflexVertices.push_back(v.id);
				}
				else if (v.angle < 180)
				{
					convexVertices.push_back(v.id);
				}
				else
				{
					cout << "Something is off" << endl;
				}
			}
		}

		// is ear depends on the reflex and convex list, so we need to do them separately
		for (int i = 0; i < convexVertices.size(); i++)
		{
			Vertex v = vertices[convexVertices[i]];
			if (!isRemoved(v.id))
			{
				if (isEar(v.id))
				{
					ears.push_back(v.id);
				}
			}
		}

	}
	*/


	/*
	bool CheckIfOtherVerticesAreInTriangle(const std::vector<TrigulationVertex>& earTriangle)
	{
		// the paper says it's sufficient to just test against the reflex vertices.. soo 
		for (int i = 0; i < reflexVertices.size(); i++)
		{
			Vertex v = vertices[reflexVertices[i]];

			if (v == v0 || v == v1 || v == v2)
			{
				continue;
			}
			else
			{
				glm::vec2 test = v.pos;
				bool isInside = Collision::isPointInTriangle(test, v0.pos, v1.pos, v2.pos);
				if (isInside)
				{
					return true;
				}
			}
		}
		return false;
	}
	*/

	bool IsEar(std::vector<Vertex>& earTriangle, std::vector<Vertex>& vertices)
	{
		// check if other vertices are inside the triangle
		for (int i = 0; i < vertices.size(); i++)
		{		
			int id = vertices[i].id;

			if (id == earTriangle[0].id ||
				id == earTriangle[1].id ||
				id == earTriangle[2].id)
			{
				continue;
			}
			else
			{
				glm::vec3 testPoint = vertices[i].pos;

				if (vertices[i].id == 4)
				{
					int a = 1;
				}

				bool isInside = Collision::IsPointInsideTriangle_Barycentric_NotOnTheLine(testPoint, earTriangle[0].pos, earTriangle[1].pos, earTriangle[2].pos);
				if (isInside)
				{
					std::cout << "vertices " << vertices[i].id << " is inside " << std::endl;
					return false;
				}
			}
		}
		return true;
	}

	int FindIndex(std::vector<Vertex>& trigVertices, Vertex vertex)
	{
		for (int i = 0; i < trigVertices.size(); i++)
		{
			if (trigVertices[i].id == vertex.id)
			{
				return i;
			}
		}
		return -1;
	}

	std::vector<Triangle> EarClippingTriangulation(std::vector<glm::vec3> vertices)
	{
		assert(vertices.size() >= 3);

		std::vector<Triangle> triangles;
		if (vertices.size() == 3)
		{
			return triangles;
		}


		// converting to 
		std::vector<Vertex> trigVertices;
		for (int i = 0; i < vertices.size(); i++)
		{
			trigVertices.push_back({ i, vertices[i] });
		}



		int v0 = 0;
		int v1 = 0;
		int v2 = 0;
		std::vector<Vertex> earTriangle;

		int iter = 0;

		// iteratively shave off ears.		
		while (trigVertices.size() > 3)
		{
			std::cout << "iter " << iter << " size is " << trigVertices.size() << std::endl;

			// we want to minimize the distance between the triangle vertices
			float bestDist = FLT_MAX;
			int bestEar = -1;
			std::vector<Vertex> bestEarTriangle;

			for (int i = 0; i < trigVertices.size(); i++)
			{
				v0 = i - 1;
				v1 = i;
				v2 = i + 1;

				if (i == 0)
				{
					v0 = trigVertices.size() - 1;
				}
				else if (i == trigVertices.size() - 1)
				{
					v2 = 0;
				}

				float angle2 = Math::CalculateInteriorAngle(trigVertices[v0].pos, trigVertices[v1].pos, trigVertices[v2].pos);
				// not a convex vertex
				if (angle2 >= 180)
				{
					continue;
				}

				earTriangle.clear();
				earTriangle.push_back(trigVertices[v0]);
				earTriangle.push_back(trigVertices[v1]);
				earTriangle.push_back(trigVertices[v2]);

				if (IsEar(earTriangle, trigVertices))
				{
					glm::vec3 edge0 = trigVertices[v0].pos - trigVertices[v1].pos;
					glm::vec3 edge1 = trigVertices[v2].pos - trigVertices[v1].pos;
					glm::vec3 edge2 = trigVertices[v0].pos - trigVertices[v2].pos;

					float dist = glm::dot(edge0, edge0) + glm::dot(edge1, edge1) + glm::dot(edge2, edge2);
					if (dist < bestDist)
					{
						bestDist = dist;
						bestEar = v1;
						bestEarTriangle = earTriangle;
					}
				}
			}

			assert(bestEar != -1);

			// remove the ear
			triangles.push_back({
				bestEarTriangle[0].pos,
				bestEarTriangle[1].pos,
				bestEarTriangle[2].pos
			});

			int index = FindIndex(trigVertices, trigVertices[bestEar]);
			trigVertices.erase(trigVertices.begin() + index);

			iter++;
		}

		// push the remaining three
		triangles.push_back({
			trigVertices[0].pos,
			trigVertices[1].pos,
			trigVertices[2].pos
		});

		return triangles;
	}


	
	Triangle CreateSuperTriangle(std::vector<glm::vec3> vertices)
	{
		// create super triangle

		// we first grab the rectangle containing all the points
		// then 
		glm::vec3 min = glm::vec3(FLT_MAX);
		glm::vec3 max = glm::vec3(FLT_MIN);
		Triangle superTriangle;
		for (int i = 0; i < vertices.size(); i++)
		{
			if (vertices[i].x < min.x)
			{
				min.x = vertices[i].x;
			}

			if (vertices[i].x > max.x)
			{
				max.x = vertices[i].x;
			}

			if (vertices[i].y < min.y)
			{
				min.y = vertices[i].y;
			}

			if (vertices[i].y > max.y)
			{
				max.y = vertices[i].y;
			}
		}

		/*
				   apex
				   .
				  / \
				 /   \
				/	  \
		    p2 /	   \	p3
			  .---------.
			 /|		    |\
			/ |		    | \
		   /  |		    |  \
		  /	  |         |   \
		 /	  .---------.    \
		   p0			  p1
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

		return { triangleBasePt0 , triangleBasePt1, apex, };
	}




	glm::vec2 FindCircumCenter(glm::vec2 a, glm::vec2 b, glm::vec2 c)
	{
		// fidn mid point of p0 and p1
		glm::vec2 p0 = (a + b) / (float)2;
		glm::vec2 d0 = b - a;
		d0 = glm::normalize(glm::vec2(-d0.y, d0.x));

		glm::vec2 p1 = (b + c) / (float)2;
		glm::vec2 d1 = c - b;
		d1 = glm::normalize(glm::vec2(-d1.y, d1.x));

		// intersection
		glm::vec2 intersectionPoint;
		Collision::GetRayRayIntersection2D(p0, d0, p1, d1, intersectionPoint);

		return intersectionPoint;
	}

	Circle FindCircumCircle(glm::vec2 a, glm::vec2 b, glm::vec2 c)
	{
		glm::vec2 center = FindCircumCenter(a, b, c);
		Circle circle;
		circle.center = glm::vec3(center.x, center.y, 0);
		circle.radius = glm::distance(a, center);
		return circle;
	}

	Circle FindCircumCircle(Triangle triangle)
	{
		glm::vec2 a = glm::vec2(triangle.vertices[0].x, triangle.vertices[0].y);
		glm::vec2 b = glm::vec2(triangle.vertices[1].x, triangle.vertices[1].y);
		glm::vec2 c = glm::vec2(triangle.vertices[2].x, triangle.vertices[2].y);

		return FindCircumCircle(a,b,c);
	}

	// https://www.youtube.com/watch?v=GctAunEuHt4&ab_channel=SCIco
	// this maxmizes the minimum of all the angles of the triangles in the triangulation
	// using the shou
	// https://gorillasun.de/blog/bowyer-watson-algorithm-for-delaunay-triangulation
	void DelaunayTraingulation(std::vector<glm::vec3> vertices, DebugState* triangulationDebug) {

		Triangle superTriangle = CreateSuperTriangle(vertices);
		triangulationDebug->superTriangle = superTriangle;

		// first add our super triangle
		std::vector<Triangle> invalidTriangles;
		invalidTriangles.push_back(superTriangle);


		// connect edge of super triangle to the first vertex. 
		Triangle curTriangle = superTriangle;
		glm::vec3 curVertex = vertices[0];

		Edge edge0 = { superTriangle.vertices[0], curVertex };
		Edge edge1 = { superTriangle.vertices[1], curVertex };
		Edge edge2 = { superTriangle.vertices[2], curVertex };

		// compute circumcircles of the new triangles
		Triangle newTriangle0 = { curVertex, curTriangle.vertices[0], curTriangle.vertices[1] };
		Triangle newTriangle1 = { curVertex, curTriangle.vertices[1], curTriangle.vertices[2] };
		Triangle newTriangle2 = { curVertex, curTriangle.vertices[2], curTriangle.vertices[0] };


		/*
		Triangle newTriangle3 = { glm::vec3(10, -10, 0), glm::vec3(0, 0, 0), glm::vec3(-10, -10, 0) };
		Circle circle3 = FindCircumCircle(newTriangle3);
		triangulationDebug->circles.push_back(circle3);
		*/



		
		Circle circle0 = FindCircumCircle(newTriangle0);
		Circle circle1 = FindCircumCircle(newTriangle1);
		Circle circle2 = FindCircumCircle(newTriangle2);

		triangulationDebug->circles.push_back(circle0);
		triangulationDebug->circles.push_back(circle1);
		triangulationDebug->circles.push_back(circle2);
		

		/*
		for (int i = 0; i < vertices.size(); i++)
		{
			glm::vec3 vertex = vertices[i];

			Edge edge0 = { superTriangle.vertices[0], vertex };
			Edge edge1 = { superTriangle.vertices[1], vertex };
			Edge edge2 = { superTriangle.vertices[2], vertex };


		}
		*/

	}

}