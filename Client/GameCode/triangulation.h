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
		int id;
		glm::vec3 pos;
	};

	struct TrigulationTriangle
	{
		// counter clockwise order
		glm::vec3 v0;
		glm::vec3 v1;
		glm::vec3 v2;
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

				bool isInside = Collision::IsPointInsideTriangle_Barycentric(testPoint, earTriangle[0].pos, earTriangle[1].pos, earTriangle[2].pos);
				if (isInside)
				{
					std::cout << "vertices " << vertices[i].id << " is inside " << std::endl;
					return false;
				}
			}
		}
		return true;
	}

	/*
	std::vector<TrigulationVertex> FindAllEars(std::vector<TrigulationVertex>& vertices)
	{
		std::vector<TrigulationVertex> ears;

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

			if (IsEar(earTriangle, vertices))
			{
				ears.push_back(vertices[v1]);
			}
		}
	}
	*/

	int FindIndex(std::vector<TrigulationVertex>& trigVertices, TrigulationVertex vertex)
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


	std::vector<TrigulationTriangle> Triangulate(std::vector<glm::vec3> vertices)
	{
		assert(vertices.size() >= 3);

		std::vector<TrigulationTriangle> triangles;
		if (vertices.size() == 3)
		{
			return triangles;
		}


		// converting to 
		std::vector<TrigulationVertex> trigVertices;
		for (int i = 0; i < vertices.size(); i++)
		{
			trigVertices.push_back({ i, vertices[i] });
		}



		int v0 = 0;
		int v1 = 0;
		int v2 = 0;
		std::vector<TrigulationVertex> earTriangle;

		int iter = 0;

		// iteratively shave off ears.		
		while (trigVertices.size() > 3)
		{
			std::cout << "iter " << iter << " size is " << trigVertices.size() << std::endl;

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

				glm::vec3 edge0 = trigVertices[v0].pos - trigVertices[v1].pos;
				glm::vec3 edge1 = trigVertices[v2].pos - trigVertices[v1].pos;

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
					triangles.push_back({
							earTriangle[0].pos, 
							earTriangle[1].pos, 
							earTriangle[2].pos 
						});

					int index = FindIndex(trigVertices, trigVertices[v1]);

					trigVertices.erase(trigVertices.begin()+index);
					break;
				}
			}




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

}