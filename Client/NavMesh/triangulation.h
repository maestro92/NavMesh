#pragma once

#include "../PlatformShared/platform_shared.h"
#include "collision.h"

#include <iostream>
#include <vector>

namespace Triangulation
{
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

	// Just has a DebugId for conveniences
	struct TrigulationVertex
	{
		int index;
		glm::vec3 pos;
	};

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

	bool IsEar(std::vector<TrigulationVertex>& earTriangle, std::vector<TrigulationVertex>& vertices)
	{
		// check if other vertices are inside the triangle
		for (int i = 0; i < vertices.size(); i++)
		{		
			int vertexIndex = earTriangle[i].index;

			if (vertexIndex == earTriangle[0].index || 
				vertexIndex == earTriangle[1].index || 
				vertexIndex == earTriangle[2].index)
			{
				continue;
			}
			else
			{
				glm::vec3 testPoint = earTriangle[i].pos;

				bool isInside = Collision::isPointInTriangle(testPoint, earTriangle[0].pos, earTriangle[1].pos, earTriangle[2].pos);
				if (isInside)
				{
					return true;
				}
			}
		}
		return false;
	}

	void FindAllEars(std::vector<TrigulationVertex>& vertices)
	{
		int v0 = 0;
		int v1 = 0;
		int v2 = 0;
		std::vector<TrigulationVertex> earTriangle;

		for (int i = 0; i < vertices.size(); i++)
		{
			v0 = i - 1;
			v1 = i;
			v2 = i + 1;

			if (i == 0)
			{
				v0 = vertices.size() - 1;
			}
			else if (i == vertices.size() - 1)
			{
				v2 = 0;
			}

			earTriangle.push_back(vertices[v0]);
			earTriangle.push_back(vertices[v1]);
			earTriangle.push_back(vertices[v2]);

		}
	}

	void Triangulate(std::vector<glm::vec3> vertices)
	{
		assert(vertices.size() > 3);

		// converting to 
		std::vector<TrigulationVertex> myVertices;
		for (int i = 0; i < vertices.size(); i++)
		{
			myVertices.push_back({ i, vertices[i] });
		}


		// first find all the ears.
	}

}