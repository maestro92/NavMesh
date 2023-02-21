#pragma once
#pragma once

#include "../PlatformShared/platform_shared.h"
#include "collision.h"
#include "triangulation.h"

#include <iostream>
#include <vector>

namespace Voronoi
{



	struct Vertex
	{
		int id;
		glm::vec3 pos;

		friend bool operator==(const Vertex& l, const Vertex& r)
		{
			return Math::Equals(l.pos, r.pos);
		}

	};

	/*
	struct Edge
	{
		glm::vec3 vertices[2];

		friend bool operator==(const Edge& l, const Edge& r)
		{
			return Math::Equals(l.vertices[0], r.vertices[0]) && Math::Equals(l.vertices[1], r.vertices[1]) ||
				Math::Equals(l.vertices[0], r.vertices[1]) && Math::Equals(l.vertices[1], r.vertices[0]);
		}

		void DebugLog()
		{
			std::cout << vertices[0].x << " " << vertices[0].y << " ---> " << vertices[1].x << " " << vertices[1].y << std::endl;
		}
	};
	*/

	struct Cell
	{
		Vertex center;
		std::vector<Vertex> neighbors;

		void TryAddNeighbor(Vertex neighbor)
		{
			for (int i = 0; i < neighbors.size(); i++)
			{
				if (neighbors[i] == neighbor)
				{
					return;
				}
			}

			neighbors.push_back(neighbor);
		}
	};

	struct DebugState
	{
		std::vector<Cell> cells;
	};

	void TryToAddVoronoiEdge(Vertex a, Vertex b, std::vector<Cell>& voronoiCells)
	{
		bool foundA = false, foundB = false;
//		Cell* cellA = NULL;
//		Cell* cellB = NULL;

		int indexA = -1;
		int indexB = -1;

		for (int i = 0; i < voronoiCells.size(); i++)
		{
			if (voronoiCells[i].center == a)
			{
				foundA = true;
				indexA = i;
			}

			if (voronoiCells[i].center == b)
			{
				foundB = true;
				indexB = i;
			}
		}

		if (!foundA)
		{
			Cell cell;
			cell.center = a;
			voronoiCells.push_back(cell);
			indexA = voronoiCells.size() - 1;
		}

		if (!foundB)
		{
			Cell cell;
			cell.center = b;
			voronoiCells.push_back(cell);
			indexB = voronoiCells.size() - 1;
		}

		voronoiCells[indexA].TryAddNeighbor(b);
		voronoiCells[indexB].TryAddNeighbor(a);
	}

	void GenerateVoronoiGraph(std::vector<Triangulation::Triangle> triangles, DebugState* debugState)
	{
		// for ever delaunay edge, compute voronoi edge
		// the voroni edge is the edge connecting the circumcenters of two neighboring delaunay triangles
		std::vector<Cell> voronoiCells;
		
		for (int i = 0; i < triangles.size(); i++)
		{
			std::cout << " i " << i << std::endl;
			Triangulation::Triangle a = triangles[i];

			std::vector<Triangulation::Triangle> neighbors;
			for (int j = i + 1; j < triangles.size(); j++)
			{
				Triangulation::Triangle b = triangles[j];

				if (Triangulation::IsNeighbor(a, b))
				{
					neighbors.push_back(b);
				}
			}
			
			// the circumcenter is also the voronoi vertex. 
			Triangulation::Circle circle = Triangulation::FindCircumCircle(a);

			Vertex voronoiVertex;
			voronoiVertex.pos = glm::vec3(circle.center.x, circle.center.y, 0);
		
			for (int j = 0; j < neighbors.size(); j++)
			{
				Triangulation::Triangle neighbor = neighbors[j];

				Triangulation::Circle circle2 = Triangulation::FindCircumCircle(neighbor);
				Vertex vertex2;
				vertex2.pos = glm::vec3(circle2.center.x, circle2.center.y, 0);

				TryToAddVoronoiEdge(voronoiVertex, vertex2, voronoiCells);
			}
		}

		debugState->cells = voronoiCells;
	}

}