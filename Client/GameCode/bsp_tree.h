#pragma once

#include "../PlatformShared/platform_shared.h"

#include <algorithm>
#include <iostream>
#include <vector>


struct BoundingBox
{
	glm::vec3 min;
	glm::vec3 max;
};

struct Plane
{
	glm::vec3 normal;
	float distance;

	void PrintDebug()
	{
		std::cout << "		normal " << normal << ", distance " << distance << std::endl;
	}

	bool operator==(const Plane& p) const
	{
		return normal == p.normal && distance == p.distance;
	}

	bool operator!=(const Plane& p) const
	{
		return !(*this == p);
	}
};


bool PlaneEqual(Plane p0, Plane p1)
{
	return p0.normal == p1.normal && p0.distance == p1.distance;
}


struct Edge
{
	glm::vec3 vertices[2];
};

/*
// Arrarize everything
struct BSPFace
{
//	uint16 planeIndex;		
//	uint16 planeNormalSide;

	Plane plane;
	std::vector<Edge> edges;
};


struct BSPLeaf
{
	glm::vec3 bboxMin;
	glm::vec3 bboxMax;

	std::vector<BSPFace> faces;
};
*/

int idCounter;
struct BspPolygon
{
	std::vector<glm::vec3> vertices;
	Plane plane;
	int id;

	BspPolygon() {}
	BspPolygon(glm::vec3* frontVerts, int numVerts)
	{
		id = idCounter++;
		for (int i = 0; i < numVerts; i++)
		{
			vertices.push_back(frontVerts[i]);
		}
		UpdatePlane();
	}

	void PrintDebug()
	{
		std::cout << "id is " << id << std::endl;
		for (int i = 0; i < vertices.size(); i++)
		{
			std::cout << "vertices[i] " << vertices[i] << std::endl;
		}

		plane.PrintDebug();
	}

	void UpdatePlane()
	{
		assert(vertices.size() >= 3);
		glm::vec3 v0 = vertices[1] - vertices[0];
		glm::vec3 v1 = vertices[2] - vertices[1];
		plane.normal = glm::normalize(glm::cross(v0, v1));
		plane.distance = glm::dot(vertices[0], plane.normal);
	}
};


struct Brush
{
	std::vector<BspPolygon> polygons;
	std::vector<bool> used;

	void PrintDebug()
	{
		std::cout << "Printing Brush" << std::endl;
		for (int i = 0; i < polygons.size(); i++)
		{
			std::cout << "	Brush " << i << std::endl;
			polygons[i].PrintDebug();
		}
	}
};


int nodeIdCounter;
struct BSPNode
{
	static int idCounter;
	// node only
	int id;
	Plane splitPlane;
	BspPolygon debugSplitPolygon;	// easier to render 

	// front = 0, back = 1/
	BSPNode* children[2];

	// leafs only
	std::vector<Brush> brushes;


	glm::vec3 bboxMin;
	glm::vec3 bboxMax;

	BSPNode() { id = nodeIdCounter++; }

	BSPNode(BSPNode* frontTree, BSPNode* backTree)
	{
		id = nodeIdCounter++;
		children[0] = frontTree;
		children[1] = backTree;
	}

	bool IsEmpty()
	{
		return brushes.size() == 0;
	}

	bool IsSolid()
	{
		return brushes.size() > 0;
	}

	bool IsLeafNode()
	{
		return children[0] == NULL && children[1] == NULL;
	}
};





enum SplittingPlaneResult
{
	NONE,
	POLYGON_FRONT,
	POLYGON_BACK,
	POLYGON_BOTH,
	POLYGON_COPLANNAR,	// all vertices is coplanar with the plane

	POINT_FRONT,
	POINT_BACK,
	POINT_ON_PLANE,

	BRUSH_FRONT,
	BRUSH_BACK,
	BRUSH_BOTH,
	BRUSH_COPLANNAR
};

SplittingPlaneResult ClassifyPointToPlane(glm::vec3 point, Plane splittingPlane)
{
	const float PLANE_THICKNESS_EPSILON = 0.1;
	float dist = glm::dot(splittingPlane.normal, point) - splittingPlane.distance;

	if (dist > PLANE_THICKNESS_EPSILON)
		return SplittingPlaneResult::POINT_FRONT;
	if (dist < -PLANE_THICKNESS_EPSILON)
		return SplittingPlaneResult::POINT_BACK;
	return SplittingPlaneResult::POINT_ON_PLANE;
}

SplittingPlaneResult ClassifyPolygonToPlane(BspPolygon* polygon, Plane splittingPlane)
{
	int numInFront = 0, numBehind = 0;

	for (int i = 0; i < polygon->vertices.size(); i++)
	{
		glm::vec3 v = polygon->vertices[i];
		switch (ClassifyPointToPlane(v, splittingPlane))
		{
			case SplittingPlaneResult::POINT_FRONT:
				numInFront++;
				break;
			case SplittingPlaneResult::POINT_BACK:
				numBehind++;
				break;
		}
	}

	if (numBehind != 0 && numInFront != 0)
		return SplittingPlaneResult::POLYGON_BOTH;
	if (numInFront != 0)
		return SplittingPlaneResult::POLYGON_FRONT;
	if (numBehind != 0 )
		return SplittingPlaneResult::POLYGON_BACK;

	return SplittingPlaneResult::POLYGON_COPLANNAR;
}


Plane GetOppositeFacingPlane(Plane originalPlane)
{
	Plane newPlane = originalPlane;

	if (newPlane.normal.x != 0)
	{
		newPlane.normal.x = -newPlane.normal.x;
	}

	if (newPlane.normal.y != 0)
	{
		newPlane.normal.y = -newPlane.normal.y;
	}

	if (newPlane.normal.z != 0)
	{
		newPlane.normal.z = -newPlane.normal.z;
	}

	newPlane.distance = -newPlane.distance;
	return newPlane;
}

SplittingPlaneResult ClassifyBrushToPlane(Brush brush, Plane splittingPlane)
{
	std::vector<BspPolygon>& polygons = brush.polygons;
	for (int i = 0; i < polygons.size(); i++)
	{
		if (PlaneEqual(polygons[i].plane, splittingPlane))
		{
			return SplittingPlaneResult::BRUSH_BACK;
		}

		Plane newPlane = GetOppositeFacingPlane(splittingPlane);

		if (PlaneEqual(polygons[i].plane, newPlane))
		{
			return SplittingPlaneResult::BRUSH_FRONT;
		}
	}
	return SplittingPlaneResult::NONE;
}


bool IsAxialPlane(Plane plane)
{
	return plane.normal == glm::vec3(1, 0, 0) || plane.normal == glm::vec3(-1, 0, 0) ||
			plane.normal == glm::vec3(0, 1, 0) || plane.normal == glm::vec3(0, -1, 0) ||
			plane.normal == glm::vec3(0, 0, 1) || plane.normal == glm::vec3(0, 0, -1);
}


float EvaluateSplittingPlane(int id, Plane splittingPlane, std::vector<Brush>& brushes, bool print = false)
{
	// std::cout << "id is " << id; 
	// splittingPlane.PrintDebug();
	int numInFront = 0, numInBack = 0, numInBoth = 0;
	for (int i = 0; i < brushes.size(); i++)
	{
		for (int j = 0; j < brushes[i].polygons.size(); j++)
		{
			SplittingPlaneResult result = ClassifyPolygonToPlane(&brushes[i].polygons[j], splittingPlane);

			switch (result)
			{
				case SplittingPlaneResult::POLYGON_COPLANNAR:	// coplanar polygons treated as being in front of plane
				case SplittingPlaneResult::POLYGON_FRONT:
					numInFront++;
					break;
				case SplittingPlaneResult::POLYGON_BACK:
					numInBack++;
					break;
				case SplittingPlaneResult::POLYGON_BOTH:
					numInBoth++;
					break;
			}
		}
	}

	// pick planes so as to minimize splitting of geometry and to attempt
	// to balance the geometry equall on both sides of the splitting plane. 
	float score = numInBoth + abs(numInFront - numInBack);

	// Prefer axial planes
	if (!IsAxialPlane(splittingPlane))
	{
		score += 5;
	}

	/*
	if (print)
	{
		std::cout << "			numInFront " << numInFront << ", numInBack " << numInBack << ", numInBoth " << numInBoth << ". score is " << score << std::endl;
	}
	*/
	return score;
}


void ExtendPlanePolygon(BspPolygon& polygon, BoundingBox bb)
{
	// get centroid

	glm::vec3 centroid;
	std::vector<glm::vec3> v;
	for (int i = 0; i < polygon.vertices.size(); i++)
	{
		centroid += polygon.vertices[i];
		v.push_back(polygon.vertices[i]);
	}

	centroid /= polygon.vertices.size();



	
	// move back to origin
	for (int i = 0; i < v.size(); i++)
	{
		v[i] -= centroid;
	}

	// enlarge all coordinates
	for (int i = 0; i < v.size(); i++)
	{
		v[i] *= 1.1;
	}

	// move it back to centroid pos
	for (int i = 0; i < v.size(); i++)
	{
		v[i] += centroid;
	}

	/*
	for (int i = 0; i < v.size(); i++)
	{
		for (int j = 0; j < 3; j++)
		{
			v[i][j] = std::min<float>(v[i][j], bb.max[j]);
			v[i][j] = std::max<float>(v[i][j], bb.min[j]);
		}
	}
	*/
	polygon.vertices = v;
}

// pick planes so as to minimize splitting of geometry and to attempt
// to balance the geometry equall on both sides of the splitting plane. 
bool PickSplittingPlane(std::vector<Brush>& brushes, BspPolygon& splitPolygon, Plane& splittingPlane)
{
	const float k = 0;

	bool found = false;
	int brushIndex = 0, polygonPlaneIndex = 0;
	float bestScore = FLT_MAX;

	BoundingBox bb;
	for (int i = 0; i < brushes.size(); i++)
	{
		std::vector<BspPolygon>& polygons = brushes[i].polygons;
		for (int j = 0; j < polygons.size(); j++)
		{
			std::vector<glm::vec3> vertices = polygons[j].vertices;
			for (int k = 0; k < vertices.size(); k++)
			{
				for (int it = 0; it < 3; it++)
				{
					bb.min[it] = std::min<float>(bb.min[it], vertices[k][it]);
					bb.max[it] = std::max<float>(bb.max[it], vertices[k][it]);
				}
			}
		}
	}

	// std::cout << "min " << bb.min << std::endl;
	// std::cout << "max " << bb.max << std::endl;

	// std::cout << "brushes.size() " << brushes.size() << std::endl;

	for (int i = 0; i < brushes.size(); i++)
	{
		std::vector<BspPolygon>& polygons = brushes[i].polygons;
		for (int j = 0; j < polygons.size(); j++)
		{
		//	std::cout << "Evaluating split plane " << std::endl;
		//	polygons[j].PrintDebug();
			if (brushes[i].used[j] == true)
				continue;

			float score = EvaluateSplittingPlane(polygons[j].id, polygons[j].plane, brushes, true);

			if (!IsAxialPlane(polygons[j].plane))
			{
				std::cout << "i " << i << " j " << j << std::endl;
				int a = 1;
			}

			if (score < bestScore)
			{
				bestScore = score;
				splittingPlane = polygons[j].plane;
				splitPolygon = polygons[j];
				splitPolygon.plane = splittingPlane;

				brushIndex = i;
				polygonPlaneIndex = j;

				found = true;
				// make it larger
				ExtendPlanePolygon(splitPolygon, bb);
			}
		}
	}

//	std::cout << "rampAxialPlane is " << planeFlags[6][1] << std::endl;

//	planeFlags[brushIndex][polygonPlaneIndex] = true;
	// std::cout << "brushIndex " << brushIndex << " polygonPlaneIndex " << polygonPlaneIndex << std::endl;
	
	// std::cout << "		brush polygonSize" << brushes[brushIndex].polygons.size() << std::endl;
	// std::cout << "		brush usedSize" << brushes[brushIndex].used.size() << std::endl;

	if (found)
	{
		brushes[brushIndex].used[polygonPlaneIndex] = true;

		// Set coplanar planes to false as well
		for (int i = 0; i < brushes.size(); i++)
		{
			std::vector<BspPolygon>& polygons = brushes[i].polygons;
			for (int j = 0; j < polygons.size(); j++)
			{
				SplittingPlaneResult result = ClassifyPolygonToPlane(&brushes[i].polygons[j], splittingPlane);

				switch (result)
				{
				case SplittingPlaneResult::POLYGON_COPLANNAR:	// coplanar polygons treated as being in front of plane

					if (i == 6 && j == 1)
					{
						int b = 1;
					}
					// this is not right
	//				planeFlags[i][j] = true;
					brushes[i].used[j] = true;
					break;
				case SplittingPlaneResult::POLYGON_FRONT:
					break;
				case SplittingPlaneResult::POLYGON_BACK:
					break;
				case SplittingPlaneResult::POLYGON_BOTH:
					break;
				}
			}
		}
	}

	return found;
}


// Page 175 IntersectSegmentPlane
bool IntersectEdgeAgainstPlane(glm::vec3 p0, glm::vec3 p1, Plane plane, glm::vec3& q)
{
	float t = 0;
	glm::vec3 vec1_0 = p1 - p0;

	t = (plane.distance - glm::dot(plane.normal, p0)) / glm::dot(plane.normal, vec1_0);

	if (0.0f <= t && t <= 1.0f)
	{
		q = p0 + t * vec1_0;
		return true;
	}

	return false;
}

void SplitPolygon(BspPolygon& polygon, Plane plane, BspPolygon*& frontPoly, BspPolygon*& backPoly)
{
	int numFront = 0, numBack = 0;
	const int MAX_POINTS = 1024;

	glm::vec3 frontVerts[MAX_POINTS], backVerts[MAX_POINTS];

	int numVerts = polygon.vertices.size();

	// vector from v0 -----> v1
	glm::vec3 v0 = polygon.vertices[numVerts - 1];
	SplittingPlaneResult v0Side = ClassifyPointToPlane(v0, plane);

	for (int i = 0; i < numVerts; i++)
	{
		glm::vec3 v1 = polygon.vertices[i];

		SplittingPlaneResult v1Side = ClassifyPointToPlane(v1, plane);

		if (v1Side == SplittingPlaneResult::POINT_FRONT)
		{
			if (v0Side == SplittingPlaneResult::POINT_BACK)
			{
				glm::vec3 intersectionPoint;
				IntersectEdgeAgainstPlane(v0, v1, plane, intersectionPoint);
				assert(ClassifyPointToPlane(intersectionPoint, plane) == SplittingPlaneResult::POINT_ON_PLANE);
				frontVerts[numFront++] = backVerts[numBack++] = intersectionPoint;
			}
			frontVerts[numFront++] = v1;
		}
		else if (v1Side == SplittingPlaneResult::POINT_BACK)
		{
			if (v0Side == SplittingPlaneResult::POINT_FRONT)
			{
				glm::vec3 intersectionPoint;
				IntersectEdgeAgainstPlane(v0, v1, plane, intersectionPoint);
				assert(ClassifyPointToPlane(intersectionPoint, plane) == SplittingPlaneResult::POINT_ON_PLANE);
				frontVerts[numFront++] = backVerts[numBack++] = intersectionPoint;
			}
			else if (v0Side == SplittingPlaneResult::POINT_ON_PLANE)
			{
				backVerts[numBack++] = v0;
			}

			backVerts[numBack++] = v1;
		}
		else
		{
			frontVerts[numFront++] = v1;
			if (v0Side == SplittingPlaneResult::POINT_BACK)
			{
				backVerts[numBack++] = v1;
			}
		}
		v0 = v1;
		v0Side = v1Side;
	}

	frontPoly = new BspPolygon(frontVerts, numFront);
	backPoly = new BspPolygon(backVerts, numBack);

}

std::vector<std::vector<bool>> GetPlaneUsedFlags(std::vector<Brush> brushes)
{
	std::vector<std::vector<bool>> flags;
	for (int i = 0; i < brushes.size(); i++)
	{
		int size = brushes[i].polygons.size();
		std::vector<bool> bools(size, false);
		flags.push_back(bools);
	}
	return flags;
}

void PrintBSPTree(BSPNode* node, int depth)
{
	if (node == NULL)
		return;

	printf(">>> print id: %d, depth %d\n", node->id, depth);
	if (node->IsLeafNode())
	{
		printf("	I am leaf node\n");
	
		if (node->IsSolid())
		{
			printf("	CONTENTS SOLID: ");
		}
		else
		{
			printf("	CONTENTS EMPTY: ");
		}
		

		printf("	count is %d\n", node->brushes.size());
	/*
		for (int i = 0; i < node->brushes.size(); i++)
		{
			printf();
		}
		*/

		printf("\n\n");
		return;
	}

	if (node->brushes.size() == 0)
	{
		printf("node has no brushes\n");
	}


	PrintBSPTree(node->children[0], depth + 1);
	PrintBSPTree(node->children[1], depth + 1);
}


void PrintBrushes(std::vector<Brush> brushes)
{
	printf("#### brushes size %d\n", brushes.size());
	for (int i = 0; i < brushes.size(); i++)
	{

	}
}





BSPNode* BuildBSPTree_r(std::vector<Brush> brushes, int depth)
{
	const int MAX_DEPTH = 10;
	const int MIN_LEAF_SIZE = 20;


	std::cout << ">>>>>>>>>>>>>>> Depth is " << depth << std::endl;

	BspPolygon splitPolygon;
	Plane splitPlane;

	bool succeess = PickSplittingPlane(brushes, splitPolygon, splitPlane);
	if (!succeess)
	{

		BSPNode* node = new BSPNode();
		node->brushes = brushes;
		node->children[0] = NULL;
		node->children[1] = NULL;

		printf("		Creating leafnode\n");
		printf("		count is %d\n\n", node->brushes.size());

		return node;
	}

	// std::cout << "	Split Plane " << std::endl;
	// splitPlane.PrintDebug();
	// splitPolygon.PrintDebug();

	std::vector<Brush> frontBrushes, backBrushes;
	for (int i = 0; i < brushes.size(); i++)
	{
		SplittingPlaneResult brushResult = ClassifyBrushToPlane(brushes[i], splitPlane);
		Brush frontBrush, backBrush;

		if (brushResult == SplittingPlaneResult::BRUSH_BACK)
		{
			backBrush = brushes[i];
			backBrushes.push_back(backBrush);
		}
		else if (brushResult == SplittingPlaneResult::BRUSH_FRONT)
		{
			frontBrush = brushes[i];
			frontBrushes.push_back(frontBrush);
		}
		else
		{
			for (int j = 0; j < brushes[i].polygons.size(); j++)
			{
				BspPolygon* polygon = &brushes[i].polygons[j];
				BspPolygon* frontPart = NULL;
				BspPolygon* backPart = NULL;

				SplittingPlaneResult result = ClassifyPolygonToPlane(polygon, splitPlane);

				switch (result)
				{
				case SplittingPlaneResult::POLYGON_FRONT:
					frontBrush.polygons.push_back(*polygon);
					frontBrush.used.push_back(brushes[i].used[j]);
					break;
				case SplittingPlaneResult::POLYGON_BACK:
					backBrush.polygons.push_back(*polygon);
					backBrush.used.push_back(brushes[i].used[j]);
					break;
				case SplittingPlaneResult::POLYGON_BOTH:
					SplitPolygon(*polygon, splitPlane, frontPart, backPart);
					frontBrush.polygons.push_back(*frontPart);
					backBrush.polygons.push_back(*backPart);

					frontBrush.used.push_back(brushes[i].used[j]);
					backBrush.used.push_back(brushes[i].used[j]);
					break;
				}
			}

			if (frontBrush.polygons.size() > 0)
			{
				frontBrushes.push_back(frontBrush);
			}
			if (backBrush.polygons.size() > 0)
			{
				backBrushes.push_back(backBrush);
			}
		}

	}

	PrintBrushes(frontBrushes);
	PrintBrushes(backBrushes);

	BSPNode* frontTree = BuildBSPTree_r(frontBrushes, depth + 1);
	BSPNode* backTree = BuildBSPTree_r(backBrushes, depth + 1);
	BSPNode* node = new BSPNode(frontTree, backTree);
	node->splitPlane = splitPlane;

//	std::cout << "split normal is " << node->splitPlane.normal << std::endl;

	if (abs(node->splitPlane.normal.x) != 1 && abs(node->splitPlane.normal.y) != 1 && abs(node->splitPlane.normal.z) != 1)
	{
		int a = 1;
	}

	node->debugSplitPolygon = splitPolygon;
	//	node->faces.push_back(splitPlaneBrush);
	return node;
}


// test case: https://www.bluesnews.com/abrash/chap64.shtml
BSPNode* BuildBSPTree(std::vector<Brush> brushes, int depth)
{
	// std::vector<std::vector<bool>> planeFlags = GetPlaneUsedFlags(brushes);
	return BuildBSPTree_r(brushes, depth);
}