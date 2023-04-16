#pragma once

#include <assert.h> 

#include "../PlatformShared/platform_shared.h"
#include "../NavMesh/memory.h"
#include "geometry_core.h"
#include "nav_mesh.h"
#include "cd_triangulation.h"
#include "pathfinding_common.h"
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
	STATIC,
	GROUND,
	PLAYER,
	OBSTACLE
};

struct Entity
{
	EntityFlag flag;
	
	int id;
	glm::vec3 pos;
	glm::vec3 dim;
	glm::vec3 velocity;

	std::vector<glm::vec3> vertices;

	// For AABB physics, in object space
	glm::vec3 min;
	glm::vec3 max;

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

//	Entity* groundEntity;
	// For Rendering
	// TODO: change this model index
//	std::vector<Face> model;
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

	glm::ivec2 min;
	glm::ivec2 max;

	Entity entities[1024];
	int numEntities;
	int maxEntityCount;

//	WorldSafeData* data;

	int maxPlayerEntity;
	int numPlayerEntity;


	Triangulation::DebugState* triangulationDebug;
	Voronoi::DebugState* voronoiDebug;

	CDTriangulation::Graph* cdTriangulationGraph;
	PathFinding::DebugState* pathingDebug;

	MapGrid mapGrid;

	WorldCameraSetup cameraSetup;

	void Init()
	{
		// initlaize the game state  
		numEntities = 0;
		maxEntityCount = 1024;
		triangulationDebug = new Triangulation::DebugState();
		voronoiDebug = new Voronoi::DebugState();
		cdTriangulationGraph = new CDTriangulation::Graph();
		pathingDebug = new PathFinding::DebugState();

		xAxis = glm::vec3(1.0, 0.0, 0.0);
		yAxis = glm::vec3(0.0, 1.0, 0.0);
		zAxis = glm::vec3(0.0, 0.0, 1.0);


		min = glm::ivec2(0);
		max = glm::ivec2(256, 256);

		mapGrid.cellSize = 16;
		mapGrid.dimX = max.x / mapGrid.cellSize;
		mapGrid.dimY = max.y / mapGrid.cellSize;
		int numCells = mapGrid.cellSize * mapGrid.cellSize;
		mapGrid.grid = new MapCell[numCells];
	}

	void LoadSampleMap()
	{
		Init();
		CreateAreaA(this);
	}


	
	CDTriangulation::DelaunayTriangle* FindTriangleBySimPos(glm::vec3 pos)
	{
		int cx, cy = 0;
		SimPos2GridCoord(pos, cx, cy);

		MapCell* mapCell = mapGrid.GetCellByIndex(cx, cy);
		for (int i = 0; i < mapCell->triangles.size(); i++)
		{
			int trigId = mapCell->triangles[i];
			CDTriangulation::DelaunayTriangle* triangle = cdTriangulationGraph->trianglesById[trigId];

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
	

	void CreateAreaA(World* world)
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
		std::vector<GeoCore::Polygon> holes;

		bool testPathing = true;
		if (testPathing)
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
		else
		{
			vertices.push_back(glm::vec3(world->min.x, world->min.y, 0));
			vertices.push_back(glm::vec3(world->max.x, world->min.y, 0));
			vertices.push_back(glm::vec3(world->max.x, world->max.y, 0));
			vertices.push_back(glm::vec3(world->min.x, world->max.y, 0));


			GeoCore::Polygon hole;
			hole.vertices.push_back(glm::vec3(-5, 50, 0));
			hole.vertices.push_back(glm::vec3(5, 50, 0));
			hole.vertices.push_back(glm::vec3(5, -50, 0));
			hole.vertices.push_back(glm::vec3(-5, -50, 0));
			for (int i = 0; i < hole.vertices.size(); i++)
			{
				hole.vertices[i] += glm::vec3(76.743, 128.400, 0);
			}
			holes.push_back(hole);

			GeoCore::Polygon hole2;
			hole2.vertices.push_back(glm::vec3(-5, 50, 0));
			hole2.vertices.push_back(glm::vec3(5, 50, 0));
			hole2.vertices.push_back(glm::vec3(5, -50, 0));
			hole2.vertices.push_back(glm::vec3(-5, -50, 0));
			for (int i = 0; i < hole2.vertices.size(); i++)
			{
				hole2.vertices[i] += glm::vec3(158.268, 127.925, 0);
			}
			holes.push_back(hole2);
		}
		

		


		CDTriangulation::ConstrainedDelaunayTriangulation(vertices, holes, world->max, world->cdTriangulationGraph);
		CDTriangulation::MarkObstacles(world->cdTriangulationGraph, holes);

		if (testPathing)
		{
			world->cdTriangulationGraph->trianglesById[5]->isObstacle = true;
			world->cdTriangulationGraph->trianglesById[8]->isObstacle = true;
		}

		// spatial partioning, adding triangles to grid cell
		//
		for (int i = 0; i < world->cdTriangulationGraph->triangles.size(); i++)
		{
			CDTriangulation::DelaunayTriangle& triangle = world->cdTriangulationGraph->triangles[i];

			for (int y = 0, yi = 0; yi < world->max.y; y++, yi += world->mapGrid.cellSize)
			{
				for (int x = 0, xi = 0; xi < world->max.x; x++, xi += world->mapGrid.cellSize)
				{

					glm::vec3 min = glm::vec3(xi, yi, 0);
					glm::vec3 max = min + glm::vec3(world->mapGrid.cellSize, world->mapGrid.cellSize, 0);

					GeoCore::AABB aabb = { min, max };

					if (Collision::TestTriangleAABB2D(triangle.vertices[0].pos, triangle.vertices[1].pos, triangle.vertices[2].pos, aabb))
					{
						world->mapGrid.AddTriangle(triangle.id, x, y);
					}
				}
			}
		}
	}

	void InitEntity(Entity* entity, glm::vec3 pos, EntityFlag entityFlag, std::vector<glm::vec3> vertices)
	{
		entity->pos = pos;
		entity->vertices = vertices;
		entity->flag = entityFlag;
	}

	void AddObstacle(glm::vec3 pos, std::vector<glm::vec3> vertices)
	{
		int id = numEntities++;
		Entity* entity = &entities[id];
		entity->id = id;
		InitEntity(entity, pos, OBSTACLE, vertices);
	}

	bool IsValidSimPos(glm::vec3 pos)
	{
		return 0 <= pos.x && pos.x < max.x && 0 <= pos.y && pos.y < max.y;
	}

	void SimPos2GridCoord(glm::vec3 pos, int& cx, int& cy)
	{
		cx = pos.x / mapGrid.cellSize;
		cy = pos.y / mapGrid.cellSize;
	}
};




void initPlayerEntity(Entity* entity, glm::vec3 pos)
{
	entity->pos = pos;
	entity->flag = EntityFlag::PLAYER;
	entity->min = glm::vec3(-10, -10, -10);
	entity->max = glm::vec3(10, 10, 10);

	entity->xAxis = glm::vec3(1.0, 0.0, 0.0);
	entity->yAxis = glm::vec3(0.0, 1.0, 0.0);
	entity->zAxis = glm::vec3(0.0, 0.0, 1.0);
}

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

void ScalingTheHole(GeoCore::Polygon& hole, float scale, glm::vec3 min)
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