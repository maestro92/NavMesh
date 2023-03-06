#pragma once

#include <assert.h> 

#include "../PlatformShared/platform_shared.h"
#include "../NavMesh/memory.h"
#include "nav_mesh.h"
#include "triangulation.h"
#include "cd_triangulation.h"
#include "voronoi.h"
#include "pathfinding.h"
#include "bsp_tree.h"

#define	DIST_EPSILON	(0.03125)

// each face is a quad
struct Face
{
	// Assume this is a quad
	// p0 p1 p2 p3 in clock wise order
	std::vector<glm::vec3> vertices;
};



Plane NULL_PLANE;

enum EntityFlag
{
	STATIC,
	GROUND,
	PLAYER
};

struct Entity
{
	EntityFlag flag;

	glm::vec3 pos;
	glm::vec3 dim;
	glm::vec3 velocity;

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
	Plane groundPlane;
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


struct World
{
	MemoryArena memoryArena;

	BSPNode* tree;

	Entity entities[1024];
	int numEntities;
	int maxEntityCount;


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


void initEntity(Entity* entity, glm::vec3 pos, EntityFlag entityFlag, std::vector<Face> faces)
{
	entity->pos = pos;
	entity->model = faces;
	entity->flag = entityFlag;
}

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


void AddPolygonToBrush(Brush* brush, std::vector<glm::vec3> verticesIn)
{
	const int arraySize = verticesIn.size();
	glm::vec3* vertices = new glm::vec3[arraySize];

	for (int i = 0; i < arraySize; i++)
	{
		vertices[i] = verticesIn[i];
	}

	assert(arraySize == 4);
	// attemp to build normal in all dimensions
	glm::vec3 normal;
	for (int i = 0; i < 2; i++)
	{
		glm::vec3 v0 = vertices[i+1] - vertices[i];
		glm::vec3 v1 = vertices[i+2] - vertices[i+1];
		normal = glm::normalize(glm::cross(v0, v1));

		if (!isnan(normal.x) && !isnan(normal.y) && !isnan(normal.z))
		{
			break;
		}
	}


	if (!isnan(normal.x) && !isnan(normal.y) && !isnan(normal.z))
	{
		std::cout << "normal " << normal << std::endl;
		BspPolygon polygon(vertices, 4);
		float dist = glm::dot(normal, vertices[0]);
		polygon.plane = { normal, dist };
		std::cout << "	dist " << dist << std::endl;
		brush->polygons.push_back(polygon);
		brush->used.push_back(false);
	}


}


Brush ConvertFaceToBrush(std::vector<Face> faces)
{
	Brush brush;

	for (int i = 0; i < faces.size(); i++)
	{
		AddPolygonToBrush(&brush, faces[i].vertices);
	}
	std::cout << std::endl;
	return brush;
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

void CreateAreaA(World* world, std::vector<Brush>& brushes)
{
	Entity* entity = NULL;
	std::vector<Face> faces;

	// lower level
	// Box
	entity = &world->entities[world->numEntities++];
	glm::vec3 pos;
	glm::vec3 dim;
	glm::vec3 min;
	glm::vec3 max;


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


	float scale = 20;
	/*
	vertices.push_back(glm::vec3(5, -4, 0));
	vertices.push_back(glm::vec3(9, 3, 0));
	vertices.push_back(glm::vec3(4, 9, 0));
	*/
	
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
//	holesVertices.push_back(glm::vec3(-3, 7, 0));
//	holesVertices.push_back(glm::vec3(3, 4, 0));

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


	groundPolygon.vertices = vertices;


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
	CDTriangulation::ConstrainedDelaunayTriangulation(groundPolygon.vertices, holesVertices, mapSize, world->cdTriangulationdebug);
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






// this contains the result when a box is swept through the world
struct TraceResult
{
	float timeFraction; // 0 ~ 1;
	glm::vec3 endPos;
	bool outputStartsOut;	// True if the line segment starts outside of a solid volume.
	bool outputAllSolid;	// True if the line segment is completely enclosed in a solid volume.
							// meaning both start and end are inside a brush

	Plane plane;			// surface normal at impact;
	Entity* entity;			// ground entity
};


struct TraceSetupInfo
{
	glm::vec3 mins;
	glm::vec3 maxs;
	glm::vec3 traceExtends;
	bool isTraceBoxAPoint;	// does min == max;
};


void CheckBrush(Brush* brush, glm::vec3 start, glm::vec3 end, TraceResult* result, TraceSetupInfo* setupInfo)
{
	if (brush->polygons.size() == 0)
	{
		return;
	}

	float startFraction = -1;
	float endFraction = 1;
	bool startsOut = false;
	bool endsOut = false;
	Plane* clipPlane = NULL;

	glm::vec3 offsets;
//	std::cout << ">>>>>>> CheckBrush" << std::endl;
	for (int i = 0; i < brush->polygons.size(); i++)
	{
		BspPolygon polygon = brush->polygons[i];
		Plane plane = polygon.plane;

		float startToPlaneDist = 0;
		float endToPlaneDist = 0;

		//std::cout << "plane normal " << plane.normal << " dist " << plane.distance << std::endl;
		if (plane.normal.x == 0 && plane.normal.y == 0 && plane.normal.z == -1)
		{
	//		std::cout << "plane dist " << plane.distance << std::endl;
		}
		
		for (int j = 0; j < 3; j++)
		{
			if (plane.normal[j] < 0)
			{
				offsets[j] = setupInfo->maxs[j];
			}
			else
			{
				offsets[j] = setupInfo->mins[j];
			}
		}

		startToPlaneDist = (start[0] + offsets[0]) * plane.normal[0] +
					(start[1] + offsets[1]) * plane.normal[1] +
					(start[2] + offsets[2]) * plane.normal[2] - plane.distance;

		endToPlaneDist = (end[0] + offsets[0]) * plane.normal[0] +
					(end[1] + offsets[1]) * plane.normal[1] +
					(end[2] + offsets[2]) * plane.normal[2] - plane.distance;

	//	std::cout << "	startToPlaneDist " << startToPlaneDist << std::endl;
	//	std::cout << "	endToPlaneDist " << endToPlaneDist << std::endl;


		if (startToPlaneDist > 0)
		{
			startsOut = true;
		}
		if (endToPlaneDist > 0)
		{
			endsOut = true;
		}

		/*
		if its completely in front, we return
				 _______
				|		|
				|		|
 ---->		<---|		|
				|		|
				|_______|
		
		*/

		// makesure the trace isn't completely 
		if (startToPlaneDist > 0 && endToPlaneDist > 0)
		{
			return;
		}


		/*
		both are behind this plane, it will get clipped by another one
				 _______
				|		|
				|		|
			<---|  ------->
				|		|
				|_______|
		*/
		if (startToPlaneDist <= 0 && endToPlaneDist <= 0)
		{
			continue;
		}

		// crosses face

		/*
		startDist > endDist means line is entering into the brush. See graph below
		startDist is > endDist cuz its further along the plane normal direction
				 _______
				|		|
	 	----------->	|
      		<---|		|
				|		|
				|_______|


		startDist > endDist means line is entering into the brush. See graph below
		startDist is > endDist cuz its further along the plane normal direction
				 _______
				|		|
	 			|	----------->	
      			|		|--->
				|		|
				|_______|
		*/
		if (startToPlaneDist > endToPlaneDist)
		{
			// line is entering into the brush
			float fraction = (startToPlaneDist - DIST_EPSILON) / (startToPlaneDist - endToPlaneDist);
			if (fraction > startFraction)
			{
				startFraction = fraction;
				clipPlane = &plane;
			}
		}
		else
		{
			// line is leaving the brush
			float fraction = (startToPlaneDist + DIST_EPSILON) / (startToPlaneDist - endToPlaneDist);
			if (fraction < endFraction)
			{
				endFraction = fraction;
			}
		}
	}

	if (startsOut == false)
	{
		result->outputStartsOut = false;
		if (endsOut == false)
		{
			result->outputAllSolid = true;
		}
		return;
	}

	if (startFraction < endFraction)
	{
		// -1 is just the default value. we just want to check if startFraction
		// has been set a proper value.
		if (startFraction > -1 && startFraction < result->timeFraction)
		{
			if (startFraction < 0)
				startFraction = 0;
			result->timeFraction = startFraction;
			result->plane = *clipPlane;
		}
	}


}

void TraceToLeafNode(BSPNode* node, glm::vec3 start, glm::vec3 end, TraceResult* result, TraceSetupInfo* setupInfo)
{
	if (node->brushes.size() == 0)
	{
		return;
	}

	if (node->IsLeafNode())
	{
		for (int i = 0; i < node->brushes.size(); i++)
		{
			CheckBrush(&node->brushes[i], start, end, result, setupInfo);
		
			if (result->timeFraction == 0)
				return;
		}
	}
}





void RecursiveHullCheck(BSPNode* node, float startFraction, float endFraction,
	glm::vec3 start, glm::vec3 end,
	glm::vec3 traceStart, glm::vec3 traceEnd,
	TraceResult* result, TraceSetupInfo* setupInfo, bool print = false)
{

	/*
	if (print)
	{
		std::cout << "visiting node " << node->id << std::endl;
	}
	*/

	// already hit something nearer
	if (result->timeFraction <= startFraction)
	{
		return;
	}

//	std::cout << "visiting node " << node->id << std::endl;
//	std::cout << "startFraction " << startFraction << ", endFraction " << endFraction << std::endl;

	if (node->IsLeafNode())
	{
		TraceToLeafNode(node, traceStart, traceEnd, result, setupInfo);
		return;
	}

	Plane plane = node->splitPlane;
//	std::cout << "		plane " << plane.normal << std::endl;

	float startDist, endDist, offset;
	if (IsAxialPlane(plane))
	{
		// optimize this
		startDist = glm::dot(start, plane.normal) - plane.distance;
		endDist = glm::dot(end, plane.normal) - plane.distance;
		offset = fabs(setupInfo->traceExtends[0] * plane.normal[0]) +
			fabs(setupInfo->traceExtends[1] * plane.normal[1]) +
			fabs(setupInfo->traceExtends[2] * plane.normal[2]);
	}
	else
	{
		// optimize this
		startDist = glm::dot(start, plane.normal) - plane.distance;
		endDist = glm::dot(end, plane.normal) - plane.distance;
		offset = glm::dot(plane.normal, setupInfo->traceExtends);
		
		if (setupInfo->isTraceBoxAPoint)
		{
			offset = 0;
		}
		else
		{
			// similar to 5.2.3 Testing Box Against Plane
			offset = fabs(setupInfo->traceExtends[0] * plane.normal[0]) +
				fabs(setupInfo->traceExtends[1] * plane.normal[1]) +
				fabs(setupInfo->traceExtends[2] * plane.normal[2]);
		}
	}

	if (print && node->id == 7)
	{
		std::cout << "visiting node " << node->id << std::endl;
	}

	if (startDist >= offset && endDist >= offset)
	{
		RecursiveHullCheck(node->children[0], startFraction, endFraction, start, end, 
			traceStart, traceEnd, result, setupInfo, print);
		return;
	}
	if (startDist < -offset && endDist < -offset)
	{
		RecursiveHullCheck(node->children[1], startFraction, endFraction, start, end, 
			traceStart, traceEnd, result, setupInfo, print);
		return;
	}

	// the side that the start is on. 
	int side;
	float fraction1, fraction2, middleFraction;
	glm::vec3 middlePoint;
	// 1/32 epsilon to keep floating point happy


	/*
	the case where endDist > startDist

				      plane
						
			front side	|  back side
						|
				end		|    start
			<-----------|
						|
						|


	the case where startDist > endDist

					  plane

			front side	|  back side
						|
				start	|    end
			<-----------|
						|
						|

	*/
	if (startDist < endDist)
	{
		side = 1;	// start is on the back of the plane
		float inverseDistance = 1.0f / (startDist - endDist);
		fraction1 = (startDist - offset + DIST_EPSILON) * inverseDistance;
		fraction2 = (startDist + offset + DIST_EPSILON) * inverseDistance;
	}
	else if (startDist > endDist)
	{
		side = 0;	// start is on the front side of the plane
		float inverseDistance = 1.0f / (startDist - endDist);
		fraction1 = (startDist + offset + DIST_EPSILON) * inverseDistance;
		fraction2 = (startDist - offset - DIST_EPSILON) * inverseDistance;

	//	std::cout << "fraction1 " << fraction1 << std::endl;
	//	std::cout << "fraction2 " << fraction2 << std::endl;
	}
	else
	{
		side = 0;
		fraction1 = 1.0f;
		fraction2 = 0.0f;
	}

	// examine [start  middle]
	if (fraction1 < 0) { fraction1 = 0;	}
	else if (fraction1 > 1) { fraction1 = 1; }

	middleFraction = startFraction + (endFraction - startFraction) * fraction1;
	middlePoint = start + fraction1 * (end - start);

	RecursiveHullCheck(node->children[side], startFraction, middleFraction, 
												start, middlePoint, traceStart, traceEnd, 
												result, setupInfo, print);

	// examine [middle	end]
	if (fraction2 < 0) { fraction2 = 0; }
	else if (fraction2 > 1) { fraction2 = 1; }

	middleFraction = startFraction + (endFraction - startFraction) * fraction2;
	middlePoint = start + fraction2 * (end - start);

	RecursiveHullCheck(node->children[!side], middleFraction, endFraction, 
												middlePoint, end, traceStart, 
												traceEnd, result, setupInfo, print);
}




// Cloning cmodel.c
TraceResult BoxTrace(glm::vec3 start, glm::vec3 end, glm::vec3 mins, glm::vec3 maxs, BSPNode* tree, bool print = false)
{
	TraceResult result = {};
	result.outputStartsOut = true;
	result.outputAllSolid = false;
	result.plane = NULL_PLANE;

	TraceSetupInfo setup = {};

	result.timeFraction = 1;

	setup.mins = mins;
	setup.maxs = maxs;

	if (start == end)
	{

	}

	if (mins == maxs)
	{
		setup.isTraceBoxAPoint = true;
	}
	else
	{
		setup.isTraceBoxAPoint = false;

		// getting the largest dimension in of min or max
		// essentially we are comparing -min[i] and max[i]
		setup.traceExtends[0] = -mins[0] > maxs[0] ? -mins[0] : maxs[0];
		setup.traceExtends[1] = -mins[1] > maxs[1] ? -mins[1] : maxs[1];
		setup.traceExtends[2] = -mins[2] > maxs[2] ? -mins[2] : maxs[2];
	}

	RecursiveHullCheck(tree, 0, 1, start, end, start, end, &result, &setup, print);

	if (result.timeFraction == 1)
	{
		result.endPos = end;
	}
	else
	{
		for (int i = 0; i < 3; i++)
		{
			result.endPos[i] = start[i] + result.timeFraction * (end[i] - start[i]);
		}
	}

	return result;
}










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

	NULL_PLANE = Plane();
	NULL_PLANE.normal = glm::vec3(0);

	std::vector<Brush> brushes;


	float wallHeight = 50;

	CreateAreaA(world, brushes);
	// CreateAreaC(world, brushes);

	// glm::vec3 siteBSize = glm::vec3(200, wallHeight, 200);
	//CreateAreaB(world, glm::vec3(500, 0, 0), siteBSize, brushes);
	


	std::cout << "############# BuildBSPTree" << std::endl;
	world->tree = BuildBSPTree(brushes, 0);

	std::cout << "############# PrintBSPTree" << std::endl;
	PrintBSPTree(world->tree, 0);




	world->startPlayerEntityId = world->numEntities;
	Entity* entity = &world->entities[world->numEntities++];
	glm::vec3 pos = glm::vec3(-50, 11, -12);
//	glm::vec3 pos = glm::vec3(-50, 1, -50);
	initPlayerEntity(entity, pos);
}