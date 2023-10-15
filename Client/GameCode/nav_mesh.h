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
		
		CDT::DelaunayTriangle* triangle;

		glm::vec3 center;
		std::vector<int> neighbors;
		
		int GetId()
		{
			if (triangle == NULL)
			{
				return -1;
			}
			return triangle->id;
		}
	};

	struct Portal {
		glm::vec3 left;
		glm::vec3 right;
	};

	struct DualGraph
	{
		std::vector<DualGraphNode> nodes;
		std::vector<DualGraphNode*> nodesById;

		DualGraph(std::vector<CDT::DelaunayTriangle>& triangles)
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
		}

		DualGraphNode* GetNode(int id)
		{
			assert(0 <= id && id < nodesById.size());
			return nodesById[id];
		}

		void GetEdgeVertices(
			CDT::DelaunayTriangleEdge edge,
			CDT::Graph* graph,
			Portal& portal)
		{
			int id0 = edge.vertices[0];
			portal.right = graph->GetVertexById(id0).pos;

			int id1 = edge.vertices[1];
			portal.left = graph->GetVertexById(id1).pos;
		}

		void AddToPortalList(CDT::Graph* graph, std::vector<int> nodeIds, std::vector<Portal>& portals)
		{
			for (int i = 1; i < nodeIds.size(); i++)
			{
				int nodeId0 = nodeIds[i - 1];
				int nodeId1 = nodeIds[i];

				DualGraphNode* node0 = GetNode(nodeId0);

				CDT::DelaunayTriangle* triangle = node0->triangle;
				for (int ni = 0; ni < ArrayCount(triangle->neighbors); ni++)
				{
					if (triangle->neighbors[ni] == nodeId1)
					{
						Portal portal;
						GetEdgeVertices(triangle->edges[ni], graph, portal);
						portals.push_back(portal);
					}
				}
			}
		}

		glm::vec3 GetNodeCenter(int nodeId)
		{
			DualGraphNode* node = GetNode(nodeId);
			return node->center;
		}
	};

};








