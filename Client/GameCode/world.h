#pragma once

#include <assert.h> 

#include "../PlatformShared/platform_shared.h"
#include "../NavMesh/memory.h"
#include "nav_mesh.h"
#include "triangulation.h"
#include "cd_triangulation.h"
#include "voronoi.h"
#include "pathfinding.h"

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

	Entity* groundEntity;
	// For Rendering
	// TODO: change this model index
	std::vector<Face> model;
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

struct WorldSafeData
{
	std::vector<std::vector<glm::vec3>> obstacles;
};

struct World
{
	MemoryArena memoryArena;

	glm::vec3 min;
	glm::vec3 max;

	Entity entities[1024];
	int numEntities;
	int maxEntityCount;

//	std::vector<std::vector<glm::vec3>> obstacles;
	WorldSafeData* data;
//	std::vector<int> obstacles2;

	int startPlayerEntityId;
	int maxPlayerEntity;
	int numPlayerEntity;

	std::vector<NavMesh::NavMeshPolygon> navMeshPolygons;
	NavMesh::DualGraph* dualGraph;

	std::vector<glm::vec3> waypoints;
	std::vector<NavMesh::Edge> portals;
	glm::vec3 start;
	glm::vec3 destination;

	Triangulation::DebugState* triangulationDebug;
	Voronoi::DebugState* voronoiDebug;

	CDTriangulation::DebugState* cdTriangulationdebug;

	WorldCameraSetup cameraSetup;
};

void initEntity(Entity* entity, glm::vec3 pos, EntityFlag entityFlag, std::vector<glm::vec3> vertices)
{
	entity->pos = pos;
	entity->vertices = vertices;
	entity->flag = entityFlag;
}

/*
void initEntity(Entity* entity, glm::vec3 pos, EntityFlag entityFlag, std::vector<Face> faces)
{
	entity->pos = pos;
	entity->model = faces;
	entity->flag = entityFlag;
}
*/

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


/*
NavMesh::NavMeshPolygon CreateTestGround(World* world)
{
	Entity* entity = NULL;
	glm::vec3 pos;
	glm::vec3 min;
	glm::vec3 max;
	std::vector<Face> faces;

	// ground
	entity = &world->entities[world->numEntities++];
	min = glm::vec3(-100, -20, -100);
	max = glm::vec3(100, 0, 100);
	faces = CreateCubeFaceMinMax(min, max);

	NavMesh::NavMeshPolygon groundPolygon;
	
	std::vector<glm::vec3> vertices;
	
	vertices.push_back(glm::vec3(-40, 50, 0));
	vertices.push_back(glm::vec3(30, 90, 0));
	vertices.push_back(glm::vec3(90, 40, 0));
	vertices.push_back(glm::vec3(60, -30, 0));	
	vertices.push_back(glm::vec3(110, -80, 0));
	vertices.push_back(glm::vec3(70, -100, 0));
	vertices.push_back(glm::vec3(10, -60, 0));
	vertices.push_back(glm::vec3(30, 20, 0));

	groundPolygon.vertices = vertices;

	initEntity(entity, pos, GROUND, faces);
	return groundPolygon;
}
*/

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

	/*
	std::vector<NavMesh::NavMeshPolygon> holes;
	NavMesh::NavMeshPolygon polygon;
	NavMesh::NavMeshPolygon groundPolygon;


	
	// ground
	entity = &world->entities[world->numEntities++];
	min = glm::vec3(-100, -100, -20);
	max = glm::vec3(100, 100, 0);
	faces = CreateCubeFaceMinMax(min, max);

//	glm::vec3 polyMeshMin = glm::vec3(-100, -100, 0);
	glm::vec3 polyMeshMin = glm::vec3(-100, -100, 0);

// groundPolygon = NavMesh::CreatePolygonFromMinMax(polyMeshMin, max);
*/

	// https://technology.cpm.org/general/3dgraph/
	// https://oercommons.s3.amazonaws.com/media/courseware/relatedresource/file/imth-6-1-9-6-1-coordinate_plane_plotter/index.html
	// use this online plotter to as online visualization of your points before running the game
	// lines are plotted counter-clockswise so it's consistent with the right hand rule
	std::vector<glm::vec3> vertices;
	
/*
	vertices.push_back(glm::vec3(5, -4, 0));
	vertices.push_back(glm::vec3(9, 3, 0));
	vertices.push_back(glm::vec3(4, 9, 0));
	vertices.push_back(glm::vec3(-3, 6, 0));
	vertices.push_back(glm::vec3(-8, 11, 0));
	vertices.push_back(glm::vec3(-10, 7, 0));
	vertices.push_back(glm::vec3(-6, 1, 0));
	vertices.push_back(glm::vec3(2, 3, 0));
	*/

	/*
	vertices.push_back(glm::vec3(5, -4, 0));
	vertices.push_back(glm::vec3(9, 3, 0));
	vertices.push_back(glm::vec3(4, 9, 0));
	*/
	
	
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

	

	
	// clockwise
	std::vector<glm::vec3> holesVertices;
	holesVertices.push_back(glm::vec3(2, 3, 0));
	holesVertices.push_back(glm::vec3(-8, 6, 0));
	holesVertices.push_back(glm::vec3(-3, 7, 0));
	holesVertices.push_back(glm::vec3(3, 4, 0));

	for (int i = 0; i < holesVertices.size(); i++)
	{
		holesVertices[i] = (holesVertices[i] - min) * scale;
		std::cout << holesVertices[i].x << " " << holesVertices[i].y << std::endl;
	}
	
	
	
	/*
	vertices.push_back(glm::vec3(0, -10, 0));
	vertices.push_back(glm::vec3(10, 0, 0));
	vertices.push_back(glm::vec3(-10, 0, 0));
	vertices.push_back(glm::vec3(0, 10, 0));
	*/


	// groundPolygon.vertices = vertices;


//	initEntity(entity, pos, GROUND, faces);
	

	// groundPolygon = CreateTestGround(world);

	/*
	// obstacle 0
	entity = &world->entities[world->numEntities++];
	min = glm::vec3(-80, 0, -80); 
	max = glm::vec3(80, 50, -70);
	faces = CreateCubeFaceMinMax(min, max);
	polygon = NavMesh::CreatePolygonFromMinMax(min, max);

	initEntity(entity, pos, STATIC, faces);
	holes.push_back(polygon);

	// obstacle 1
	entity = &world->entities[world->numEntities++];
	min = glm::vec3(-30, 0, -90);
	max = glm::vec3(10, 50, 60);
	faces = CreateCubeFaceMinMax(min, max);
	polygon = NavMesh::CreatePolygonFromMinMax(min, max);

	initEntity(entity, pos, STATIC, faces);
	holes.push_back(polygon);


	// obstacle 2
	entity = &world->entities[world->numEntities++];
	min = glm::vec3(-50, 0, -20);
	max = glm::vec3(20, 50, 30);
	faces = CreateCubeFaceMinMax(min, max);
	polygon = NavMesh::CreatePolygonFromMinMax(min, max);

	initEntity(entity, pos, STATIC, faces);
	holes.push_back(polygon);

	// obstacle 3
	entity = &world->entities[world->numEntities++];
	min = glm::vec3(30, 0, 20);
	max = glm::vec3(70, 50, 40);
	faces = CreateCubeFaceMinMax(min, max);
	polygon = NavMesh::CreatePolygonFromMinMax(min, max);

	initEntity(entity, pos, STATIC, faces);
	holes.push_back(polygon);
	*/

	/* 
	// step 3, we unionize the obstacle polygons (holes)
	TryUnionizePolygons(holes);
	
	// step 4, combine the boundary polgyon with ostacle polygons (holes),
	// so we are representing the world as a single polygon
	ConnectHoles(groundPolygon, holes);
	*/


	// Triangulation::DelaunayTraingulation_bowyer_watson(groundPolygon.vertices, world->triangulationDebug);
	// Voronoi::GenerateVoronoiGraph(world->triangulationDebug->triangles, world->voronoiDebug);


	glm::ivec2 mapSize = glm::ivec2(256, 256);
	CDTriangulation::ConstrainedDelaunayTriangulation(vertices, holesVertices, mapSize, world->cdTriangulationdebug);

	world->min = glm::vec3(0);
	world->max = glm::vec3(mapSize.x, mapSize.y, 0);

	// 
	/*
	// setp 5, triangulate the whole thing
	std::vector<Triangulation::Triangle> triangles = Triangulation::EarClippingTriangulation(groundPolygon.vertices);
	std::vector<NavMesh::NavMeshPolygon> polygons;

	for (int i = 0; i < triangles.size(); i++)
	{
		NavMesh::NavMeshPolygon polygon;
		polygon.vertices.push_back(triangles[i].vertices[0]);
		polygon.vertices.push_back(triangles[i].vertices[1]);
		polygon.vertices.push_back(triangles[i].vertices[2]);
		polygons.push_back(polygon);
	}
	

	// world->navMeshPolygons = holes;
	world->navMeshPolygons = polygons;

	world->dualGraph = new NavMesh::DualGraph(polygons);

//	world->start = glm::vec3(90, 0, -80);
//	world->destination = glm::vec3(-30, 0, 50);


	world->start = glm::vec3(90, 0, 90);
	world->destination = glm::vec3(-90, 0, -90);

	PathFinding::PathfindingResult pathingResult = PathFinding::FindPath(world->dualGraph, world->start, world->destination);
	world->portals = pathingResult.portals;
	world->waypoints = pathingResult.waypoints;
	
	world->navMeshPolygons = polygons;
	*/

	// world->navMeshPolygons = { groundPolygon };
}

namespace WorldManager
{
	void AddObstacle(World* world, glm::vec3 pos, std::vector<glm::vec3> vertices)
	{
		
		// std::cout << "adding obstacle" << std::endl;
		int id = world->numEntities++;
		// std::cout << "id " << id << std::endl;
		Entity* entity = &world->entities[id];
		entity->id = id;
		initEntity(entity, pos, OBSTACLE, vertices);
		world->data->obstacles.push_back(vertices);



		// std::cout << "made it to the end" << std::endl;
		
	}
};




// Essentially recreating a simplified version of dust2
void initWorld(World* world)
{
	// initlaize the game state  
	world->numEntities = 0;
	world->maxEntityCount = 1024;
	/*
	for (int i = 0; i < world->entityCount; i++)
	{
		world->entities[i].pos = glm::vec3(5 - i, 5 - i, 5 - i);
		world->entities[i].dim = glm::vec3(1);
		//			cout << i << ": " << gameState->entities[i].pos << std::endl;
	}
	*/
	world->triangulationDebug = new Triangulation::DebugState();
	world->voronoiDebug = new Voronoi::DebugState();
	world->cdTriangulationdebug = new CDTriangulation::DebugState();


	world->data = new WorldSafeData();


	float wallHeight = 50;

	CreateAreaA(world);
	// CreateAreaC(world, brushes);

	// glm::vec3 siteBSize = glm::vec3(200, wallHeight, 200);
	//CreateAreaB(world, glm::vec3(500, 0, 0), siteBSize, brushes);
	



	world->startPlayerEntityId = world->numEntities;
	int index = world->numEntities++;
	Entity* entity = &world->entities[index];
	entity->id = index;
	glm::vec3 pos = glm::vec3(-50, 11, -12);
//	glm::vec3 pos = glm::vec3(-50, 1, -50);
	initPlayerEntity(entity, pos);
}