#pragma once

#include <assert.h> 

#include "../PlatformShared/platform_shared.h"
#include "../NavMesh/memory.h"
#include "geometry_core.h"
#include "nav_mesh.h"
#include "cd_triangulation.h"
#include "pathfinding_common.h"
#include "collision.h"
#include "voronoi.h"

#define	DIST_EPSILON	(0.03125)

// each face is a quad
struct Face
{
	// Assume this is a quad
	// p0 p1 p2 p3 in clock wise order
	std::vector<glm::vec3> vertices;
};

enum EntityFlag
{
	AGENT,
	OBSTACLE,
	REMOVED
};

struct CameraEntity
{
	// consider storing these 4 as a matrix?
	// then add accessors
	glm::vec3 position;
	// camera is viewing along -zAxis
	glm::vec3 xAxis;
	glm::vec3 yAxis;
	glm::vec3 zAxis;

	float pitch;
	void SetViewDirection(glm::vec3 viewDirection)
	{
		zAxis = -viewDirection;
	}

	glm::vec3 GetViewDirection()
	{
		return -zAxis;
	}

	// usually you just pass in the gluLookAtMatrix
	void SetOrientation(glm::mat4 cameraMatrix)
	{
		// Hack: Todo, get rid of this extra inverse
		xAxis = glm::vec3(cameraMatrix[0][0], cameraMatrix[0][1], cameraMatrix[0][2]);
		yAxis = glm::vec3(cameraMatrix[1][0], cameraMatrix[1][1], cameraMatrix[1][2]);
		zAxis = glm::vec3(cameraMatrix[2][0], cameraMatrix[2][1], cameraMatrix[2][2]);
	}
};

struct PathingState
{

	bool isNearTargetWaypoint;

	glm::vec3 destination;
	std::vector<glm::vec3> waypoints;
	int curTargetWaypointIndex;

	void BeginPathingToDestination(std::vector<glm::vec3> waypointsIn)
	{
		curTargetWaypointIndex = 0;
		waypoints = waypointsIn;
		destination = waypointsIn[waypointsIn.size() - 1];
	}
};

const int EMPTY_GROUP_ID = -1;

struct GroupHelper
{

	int groupId;

	void Init()
	{
		groupId = EMPTY_GROUP_ID;
	}

	bool IsInAGroup()
	{
		return groupId != EMPTY_GROUP_ID;
	}
};


struct Entity
{
	EntityFlag flag;

	bool isSelected;

	int id;
	glm::vec3 pos;
	glm::vec3 velocity;
	glm::vec3 dim;


	// Boid boid;

	glm::vec3 facingDirection;
	glm::vec3 targetFacingDirection;

	// if agent use agent radius
	float agentRadius;
	// if obstacle use vertices
	// std::vector<glm::vec3> vertices;
	Collision::PhysBody physBody;

	bool isPushed;
	PathingState pathingState;

	GroupHelper groupHelper;

	// For AABB physics, in object space
	glm::vec3 min;
	glm::vec3 max;

	void Init()
	{
		groupHelper.Init();
	}
};

struct PlayerEntity
{
	// consider storing these 4 as a matrix?
	glm::vec3 position;
	// camera is viewing along -zAxis
	glm::vec3 xAxis;
	glm::vec3 yAxis;
	glm::vec3 zAxis;
};

struct WorldCameraSetup
{
	glm::mat4 proj;
	glm::mat4 view;
	glm::mat4 translation;
};


struct MapCell
{
	std::vector<TriangleId> triangles;

	bool ContainsTriangle(TriangleId id)
	{
		for (int i = 0; i < triangles.size(); i++)
		{
			if (triangles[i] == id)
			{
				return true;
			}
		}
		return false;
	}

	void AddTriangle(TriangleId id)
	{
		if (!ContainsTriangle(id))
		{
			triangles.push_back(id);
		}
	}
};

struct MapGrid
{
	MapCell* grid;
	int cellSize;
	int dimX;
	int dimY;

	MapCell* GetCellByIndex(int cx, int cy)
	{
		return &grid[cy * dimX + cx];
	}

	void AddTriangle(TriangleId id, int cx, int cy)
	{
		MapCell* cell = GetCellByIndex(cx, cy);
		cell->AddTriangle(id);
	}
};

struct World
{
	MemoryArena memoryArena;

	glm::vec3 xAxis;
	glm::vec3 yAxis;
	glm::vec3 zAxis;

	glm::ivec2 minSimPos;
	glm::ivec2 maxSimPos;

	Entity entities[1024];
	int numEntities;
	int maxEntityCount;

//	WorldSafeData* data;

	int maxPlayerEntity;
	int numPlayerEntity;


	Triangulation::DebugState* triangulationDebug;
	Voronoi::DebugState* voronoiDebug;

	CDT::Graph* cdTriangulationGraph;
	PathFinding::DebugState* pathingDebug;

	MapGrid mapGrid;

	WorldCameraSetup cameraSetup;
	float agentRadius;

	void Init()
	{
		// initlaize the game state  
		numEntities = 0;
		maxEntityCount = 1024;
		triangulationDebug = new Triangulation::DebugState();
		voronoiDebug = new Voronoi::DebugState();
		cdTriangulationGraph = new CDT::Graph();
		pathingDebug = new PathFinding::DebugState();

		xAxis = glm::vec3(1.0, 0.0, 0.0);
		yAxis = glm::vec3(0.0, 1.0, 0.0);
		zAxis = glm::vec3(0.0, 0.0, 1.0);


		minSimPos = glm::ivec2(0);
		maxSimPos = glm::ivec2(256, 256);

		mapGrid.cellSize = 16;
		mapGrid.dimX = maxSimPos.x / mapGrid.cellSize;
		mapGrid.dimY = maxSimPos.y / mapGrid.cellSize;
		int numCells = mapGrid.cellSize * mapGrid.cellSize;
		mapGrid.grid = new MapCell[numCells];
	}

	void LoadSampleMap()
	{
		Init();
		CreateAreaA();
	}


	
	CDT::DelaunayTriangle* FindTriangleBySimPos(glm::vec3 pos)
	{
		int cx, cy = 0;
		SimPos2GridCoord(pos, cx, cy);

		MapCell* mapCell = mapGrid.GetCellByIndex(cx, cy);
		for (int i = 0; i < mapCell->triangles.size(); i++)
		{
			int trigId = mapCell->triangles[i];
			CDT::DelaunayTriangle* triangle = cdTriangulationGraph->trianglesById[trigId];

			if (triangle->isObstacle)
			{
				continue;
			}

			if (Collision::IsPointInsideTriangle_Barycentric(
				pos,
				triangle->vertices[0].pos,
				triangle->vertices[1].pos,
				triangle->vertices[2].pos))
			{
				return triangle;
			}
		}
		return NULL;
	}
	
	void AddWorldBoundsAsHoles(std::vector<gmt::Polygon>& holes)
	{
		gmt::Polygon hole;

		// a hole with 2 vertices is just a constrained edge
		// putting the world borders as hard constraints
		hole.vertices.clear();
		hole.vertices.push_back(glm::vec3(minSimPos.x, minSimPos.y, 0));
		hole.vertices.push_back(glm::vec3(minSimPos.x, maxSimPos.y, 0));
		holes.push_back(hole);

		hole.vertices.clear();
		hole.vertices.push_back(glm::vec3(minSimPos.x, maxSimPos.y, 0));
		hole.vertices.push_back(glm::vec3(maxSimPos.x, maxSimPos.y, 0));
		holes.push_back(hole);

		hole.vertices.clear();
		hole.vertices.push_back(glm::vec3(maxSimPos.x, maxSimPos.y, 0));
		hole.vertices.push_back(glm::vec3(maxSimPos.x, minSimPos.y, 0));
		holes.push_back(hole);

		hole.vertices.clear();
		hole.vertices.push_back(glm::vec3(maxSimPos.x, minSimPos.y, 0));
		hole.vertices.push_back(glm::vec3(minSimPos.x, minSimPos.y, 0));
		holes.push_back(hole);
	}

	void SpatialPartitionTriangles()
	{
		// spatial partioning, adding triangles to grid cell
//
		for (int i = 0; i < cdTriangulationGraph->triangles.size(); i++)
		{
			CDT::DelaunayTriangle& triangle = cdTriangulationGraph->triangles[i];

			for (int y = 0, yi = 0; yi < maxSimPos.y; y++, yi += mapGrid.cellSize)
			{
				for (int x = 0, xi = 0; xi < maxSimPos.x; x++, xi += mapGrid.cellSize)
				{

					glm::vec3 min = glm::vec3(xi, yi, 0);
					glm::vec3 max = min + glm::vec3(mapGrid.cellSize, mapGrid.cellSize, 0);

					gmt::AABB aabb = { min, max };

					if (Collision::TestTriangleAABB2D(triangle.vertices[0].pos, triangle.vertices[1].pos, triangle.vertices[2].pos, aabb))
					{
						mapGrid.AddTriangle(triangle.id, x, y);
					}
				}
			}
		}
	}


	void CreateAreaA()
	{
		Entity* entity = NULL;
		std::vector<Face> faces;


		// lower level
		// Box
		// entity = &world->entities[world->numEntities++];
		glm::vec3 pos;
		glm::vec3 dim;
		glm::vec3 min;
		glm::vec3 max;


		// https://technology.cpm.org/general/3dgraph/
		// https://oercommons.s3.amazonaws.com/media/courseware/relatedresource/file/imth-6-1-9-6-1-coordinate_plane_plotter/index.html
		// use this online plotter to as online visualization of your points before running the game
		// lines are plotted counter-clockswise so it's consistent with the right hand rule
		std::vector<glm::vec3> vertices;

		// clockwise
		std::vector<gmt::Polygon> holes;

		int testPathingCase = 0;
		if (testPathingCase == 1)
		{
			vertices.push_back(glm::vec3(130, 0, 0));
			vertices.push_back(glm::vec3(150, 50, 0));
			vertices.push_back(glm::vec3(120, 90, 0));
			vertices.push_back(glm::vec3(50, 70, 0));

			vertices.push_back(glm::vec3(30, 120, 0));
			vertices.push_back(glm::vec3(0, 90, 0));
			vertices.push_back(glm::vec3(20, 40, 0));
			vertices.push_back(glm::vec3(90, 40, 0));
		}
		else if (testPathingCase == 2)
		{
			vertices.push_back(glm::vec3(min.x, min.y, 0));
			vertices.push_back(glm::vec3(max.x, min.y, 0));
			vertices.push_back(glm::vec3(max.x, max.y, 0));
			vertices.push_back(glm::vec3(min.x, max.y, 0));


			gmt::Polygon hole;
			
			hole.vertices.clear();
			hole.vertices.push_back(glm::vec3(144, 176, 0));
			hole.vertices.push_back(glm::vec3(144, 208, 0));
			hole.vertices.push_back(glm::vec3(192, 208, 0));
			hole.vertices.push_back(glm::vec3(192, 256, 0));
			hole.vertices.push_back(glm::vec3(80, 256, 0));
			hole.vertices.push_back(glm::vec3(80, 176, 0));
			holes.push_back(hole);

			
			hole.vertices.clear();
			hole.vertices.push_back(glm::vec3(80, 80, 0));
			hole.vertices.push_back(glm::vec3(160, 80, 0));
			hole.vertices.push_back(glm::vec3(160, 128, 0));
			hole.vertices.push_back(glm::vec3(80, 128, 0));
			holes.push_back(hole);
			

			hole.vertices.clear();
			hole.vertices.push_back(glm::vec3(256, 128, 0));
			hole.vertices.push_back(glm::vec3(256, 176, 0));
			hole.vertices.push_back(glm::vec3(192, 176, 0));
			hole.vertices.push_back(glm::vec3(192, 128, 0));
			holes.push_back(hole);
			
			AddWorldBoundsAsHoles(holes);		
		}
		else if (testPathingCase == 3)
		{
			vertices.push_back(glm::vec3(min.x, min.y, 0));
			vertices.push_back(glm::vec3(max.x, min.y, 0));
			vertices.push_back(glm::vec3(max.x, max.y, 0));
			vertices.push_back(glm::vec3(min.x, max.y, 0));


			gmt::Polygon hole;

			hole.vertices.clear();
			hole.vertices.push_back(glm::vec3(-5, 50, 0));
			hole.vertices.push_back(glm::vec3(5, 50, 0));
			hole.vertices.push_back(glm::vec3(5, -50, 0));
			hole.vertices.push_back(glm::vec3(-5, -50, 0));
			for (int i = 0; i < hole.vertices.size(); i++)
			{
				hole.vertices[i] += glm::vec3(76.743, 128.400, 0);
			}
			holes.push_back(hole);


			hole.vertices.clear();
			hole.vertices.push_back(glm::vec3(-5, 50, 0));
			hole.vertices.push_back(glm::vec3(5, 50, 0));
			hole.vertices.push_back(glm::vec3(5, -50, 0));
			hole.vertices.push_back(glm::vec3(-5, -50, 0));
			for (int i = 0; i < hole.vertices.size(); i++)
			{
				hole.vertices[i] += glm::vec3(158.268, 127.925, 0);
			}
			holes.push_back(hole);

			AddWorldBoundsAsHoles(holes);
		}


		CDT::ConstrainedDelaunayTriangulation(vertices, holes, maxSimPos, cdTriangulationGraph);
		CDT::MarkObstacles(cdTriangulationGraph, holes);

		if (testPathingCase == 1)
		{
			cdTriangulationGraph->trianglesById[5]->isObstacle = true;
			cdTriangulationGraph->trianglesById[8]->isObstacle = true;
		}

		SpatialPartitionTriangles();
	}

	void AddAgent(glm::vec3 pos, float radius)
	{
		int id = numEntities++;
		Entity* entity = &entities[id];
		entity->id = id;
		entity->pos = pos;
		entity->velocity = glm::vec3(0, 0, 0);

		entity->agentRadius = radius;
		entity->flag = EntityFlag::AGENT;
		entity->facingDirection = glm::vec3(1, 0, 0);
		entity->targetFacingDirection = glm::vec3(1, 0, 0);

		entity->Init();
	}

	void AddObstacle(glm::vec3 pos, std::vector<glm::vec3> vertices)
	{
		int id = numEntities++;
		Entity* entity = &entities[id];
		entity->id = id;
		entity->pos = pos;
		entity->velocity = glm::vec3(0, 0, 0);

		entity->physBody.SetData(vertices);
		entity->flag = EntityFlag::OBSTACLE;
		entity->facingDirection = glm::vec3(1, 0, 0);
		entity->targetFacingDirection = glm::vec3(1, 0, 0);

		entity->Init();
	}

	void RemoveEntity(Entity* entity)
	{
		entity->flag = EntityFlag::REMOVED;
	}

	bool IsValidSimPos(glm::vec3 pos)
	{
		return 0 <= pos.x && pos.x < maxSimPos.x && 0 <= pos.y && pos.y < maxSimPos.y;
	}

	void SimPos2GridCoord(glm::vec3 pos, int& cx, int& cy)
	{
		cx = pos.x / mapGrid.cellSize;
		cy = pos.y / mapGrid.cellSize;
	}
};


// the list of vertices are in no particular order.
std::vector<glm::vec3> GetCubeVertices(glm::vec3 min, glm::vec3 max)
{
	/*
		y
		^
	  (-x,y,-z) p4 ------------ p5 (x,y,-z)
		|		|\              |\
		|		| \             | \
		|		| (-x,y,z)      |  \
		|		|	p0 ------------ p1 (x,y,z)
		|	    p6 -|----------	p7	|
	   (-x,-y,-z)\  |	(x,-y,-z)\	|
		|		  \	|		      \ |
		|		   \|			   \|
		|			p2 ------------ p3 (x,-y,z)
		|         (-x,-y,z)
		|
		------------------------------------------> x
		\
		 \
		  \
		   V z
	*/

	// 4 points on front face 
	glm::vec3 p0 = glm::vec3(min.x, max.y, max.z);
	glm::vec3 p1 = glm::vec3(max.x, max.y, max.z);
	glm::vec3 p2 = glm::vec3(min.x, min.y, max.z);
	glm::vec3 p3 = glm::vec3(max.x, min.y, max.z);

	// 4 points on back face 
	glm::vec3 p4 = glm::vec3(min.x, max.y, min.z);
	glm::vec3 p5 = glm::vec3(max.x, max.y, min.z);
	glm::vec3 p6 = glm::vec3(min.x, min.y, min.z);
	glm::vec3 p7 = glm::vec3(max.x, min.y, min.z);

	return {p0, p1, p2, p3, p4, p5, p6, p7};
}

std::vector<glm::vec3> ToVector(glm::vec3* data)
{
	std::vector<glm::vec3> result;
	for (int i = 0; i < ArrayCount(data); i++)
	{
		result.push_back(data[i]);
	}
	return result;
}

enum RampRiseDirection
{
	POS_X,
	NEG_X,
	POS_Z,
	NEG_Z,
};

// min max as a volume
std::vector<Face> CreateRampMinMax(glm::vec3 min, glm::vec3 max, RampRiseDirection rampRiseDirection)
{
	std::vector<Face> result;
	std::vector<std::vector<glm::vec3>> temp;

	std::vector<glm::vec3> vertices = GetCubeVertices(min, max);

	// 4 points on front face 
	glm::vec3 p0 = vertices[0];
	glm::vec3 p1 = vertices[1];
	glm::vec3 p2 = vertices[2];
	glm::vec3 p3 = vertices[3];

	// 4 points on back face 
	glm::vec3 p4 = vertices[4];
	glm::vec3 p5 = vertices[5];
	glm::vec3 p6 = vertices[6];
	glm::vec3 p7 = vertices[7];

	if (rampRiseDirection == POS_Z)
	{
		p4 = p6;
		p5 = p7;
	}
	else if (rampRiseDirection == NEG_Z)
	{
		p0 = p2;
		p1 = p3;
	}
	else if (rampRiseDirection == POS_X)
	{
		p0 = p2;
		p4 = p6;
	}
	else if (rampRiseDirection == NEG_X)
	{
		p5 = p7;
		p1 = p3;
	}

	temp.push_back({ p0, p2, p3, p1 });		// front
	temp.push_back({ p4, p0, p1, p5 });		// top
	temp.push_back({ p4, p6, p2, p0 });		// left 
	temp.push_back({ p2, p6, p7, p3 });		// bottom
	temp.push_back({ p1, p3, p7, p5 });		// right 
	temp.push_back({ p5, p7, p6, p4 });		// back 
	

	for (int i = 0; i < temp.size(); i++)
	{
		Face face = { temp[i] };
		result.push_back(face);
	}

	return result;
}





std::vector<Face> CreatePlaneMinMax(glm::vec3 min, glm::vec3 max)
{
	std::vector<Face> result;

	std::vector<glm::vec3> vertices = GetCubeVertices(min, max);

	// 4 points on front face 
	glm::vec3 p0 = vertices[0];
	glm::vec3 p1 = vertices[1];
	glm::vec3 p2 = vertices[2];
	glm::vec3 p3 = vertices[3];

	// 4 points on back face 
	glm::vec3 p4 = vertices[4];
	glm::vec3 p5 = vertices[5];
	glm::vec3 p6 = vertices[6];
	glm::vec3 p7 = vertices[7];


	std::vector<std::vector<glm::vec3>> temp;
	temp.push_back({ p0, p2, p3, p1 });		// front
	temp.push_back({ p4, p0, p1, p5 });		// top
	temp.push_back({ p4, p6, p2, p0 });		// left 
	temp.push_back({ p2, p6, p7, p3 });		// bottom
	temp.push_back({ p1, p3, p7, p5 });		// right 
	temp.push_back({ p5, p7, p6, p4 });		// back 

	for (int i = 0; i < temp.size(); i++)
	{
		Face face = { temp[i] };
		result.push_back(face);
	}

	return result;
}




std::vector<Face> CreateCubeFaceMinMax(glm::vec3 min, glm::vec3 max)
{
	std::vector<Face> result;

	std::vector<glm::vec3> vertices = GetCubeVertices(min, max);

	// 4 points on front face 
	glm::vec3 p0 = vertices[0];
	glm::vec3 p1 = vertices[1];
	glm::vec3 p2 = vertices[2];
	glm::vec3 p3 = vertices[3];

	// 4 points on back face 
	glm::vec3 p4 = vertices[4];
	glm::vec3 p5 = vertices[5];
	glm::vec3 p6 = vertices[6];
	glm::vec3 p7 = vertices[7];


	std::vector<std::vector<glm::vec3>> temp;
	temp.push_back({ p0, p2, p3, p1 });		// front
	temp.push_back({ p4, p0, p1, p5 });		// top
	temp.push_back({ p4, p6, p2, p0 });		// left 
	temp.push_back({ p2, p6, p7, p3 });		// bottom
	temp.push_back({ p1, p3, p7, p5 });		// right 
	temp.push_back({ p5, p7, p6, p4 });		// back 

	for (int i = 0; i < temp.size(); i++)
	{
		Face face = { temp[i] };
		result.push_back(face);
	}

	return result;
}


std::vector<Face> CreateCubeFaceCentered(glm::vec3 pos, glm::vec3 dim)
{
	glm::vec3 min = pos - dim;
	glm::vec3 max = pos + dim;
	return CreateCubeFaceMinMax(min, max);
}

void ScalingTheHole(gmt::Polygon& hole, float scale, glm::vec3 min)
{
	for (int i = 0; i < hole.vertices.size(); i++)
	{
		hole.vertices[i] = (hole.vertices[i] - min) * scale;
		std::cout << hole.vertices[i].x << " " << hole.vertices[i].y << std::endl;
	}
}




		/*
		float scale = 10;
		vertices.push_back(glm::vec3(2, 4, 0));
		vertices.push_back(glm::vec3(-4, 6, 0));
		vertices.push_back(glm::vec3(-9, 8, 0));

		vertices.push_back(glm::vec3(7, 2, 0));
		vertices.push_back(glm::vec3(6, -3, 0));
		vertices.push_back(glm::vec3(5, -9, 0));

		vertices.push_back(glm::vec3(3, -5, 0));
		vertices.push_back(glm::vec3(1, -9, 0));
		vertices.push_back(glm::vec3(-3, -8, 0));

		vertices.push_back(glm::vec3(-12, 3, 0));
		vertices.push_back(glm::vec3(-8, 0, 0));
		vertices.push_back(glm::vec3(-9, 2, 0));

		vertices.push_back(glm::vec3(-1, -5, 0));
		vertices.push_back(glm::vec3(-1, 10, 0));

		min = glm::vec3(FLT_MAX, FLT_MAX, 0);
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
			vertices[i] = (vertices[i] - min) * scale;
			std::cout << vertices[i].x << " " << vertices[i].y << std::endl;
		}




		GeoCore::Polygon hole;
		hole.vertices.push_back(glm::vec3(2, 3, 0));
		hole.vertices.push_back(glm::vec3(-8, 6, 0));
		hole.vertices.push_back(glm::vec3(-3, 7, 0));
		hole.vertices.push_back(glm::vec3(3, 4, 0));
		ScalingTheHole(hole, scale, min);
		holes.push_back(hole);

		GeoCore::Polygon hole2;
		hole2.vertices.push_back(glm::vec3(4, -3, 0));
		hole2.vertices.push_back(glm::vec3(-5, 0, 0));
		hole2.vertices.push_back(glm::vec3(0, 1, 0));
		hole2.vertices.push_back(glm::vec3(5, -2, 0));
		ScalingTheHole(hole2, scale, min);

		holes.push_back(hole2);
		*/