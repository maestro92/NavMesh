#pragma once


#include "../PlatformShared/platform_shared.h"
#include "math.h"

#include <iostream>
#include <vector>
#include <unordered_set>
#include "cd_triangulation.h"


namespace NavMesh
{
	// https://liu.diva-portal.org/smash/get/diva2:1560399/FULLTEXT01.pdf
	struct DualGraphNode {
		
		CDTriangulation::DelaunayTriangle* triangle;

		struct Neighbor
		{
			int id;
		//	std::vector<Edge> portals;
		};
		
		glm::vec3 center;
		std::vector<Neighbor> neighbors;
		
		int GetId()
		{
			if (triangle == NULL)
			{
				return -1;
			}
			return triangle->id;
		}

		void AddNeighbor(int neighbor)
		{
			for (int i = 0; i < neighbors.size(); i++)
			{
				if (neighbors[i].id == neighbor)
				{
					return;
				}
			}

			Neighbor neighborNode = { neighbor };
			neighbors.push_back(neighborNode);
		}


		/*
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
		*/
	};

	struct DualGraph
	{
		std::vector<DualGraphNode> nodes;
		std::vector<DualGraphNode*> nodesById;

		DualGraph(std::vector<CDTriangulation::DelaunayTriangle>& triangles)
		{
			int maxId;
			for (int i = 0; i < triangles.size(); i++)
			{
				DualGraphNode node;
				node.triangle = &triangles[i];
				node.center = triangles[i].GetCenter();
				nodes.push_back(node);

				maxId = std::max(maxId, node.GetId());
			}

			nodesById.resize(maxId + 1);

			for (int i = 0; i < nodes.size(); i++)
			{
				int id = nodes[i].GetId();
				nodesById[id] = &nodes[i];
			}

			
			for (int i = 0; i < nodes.size(); i++)
			{
				DualGraphNode& node = nodes[i];
				CDTriangulation::DelaunayTriangle* triangle = node.triangle;

				for (int j = 0; j < ArrayCount(triangle->neighbors); j++)
				{
					if (triangle->neighbors[j] != CDTriangulation::INVALID_NEIGHBOR)
					{
						node.AddNeighbor(triangle->neighbors[j]);
					}
				}	
			}			
		}

		DualGraphNode* GetNode(int id)
		{
			assert(0 <= id && id < nodesById.size());
			return nodesById[id];
		}

		/*
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
		*/
	};

};








