#pragma once

#include "../PlatformShared/platform_shared.h"
#include "collision.h"

//#include "debug_interface.h"
#include "memory.h"
#include "world.h"
#include "../NavMesh/asset.h"
#include "debug.h"
#include "game_render.h"

#include <iostream>



// define memory for Push style stuff

using namespace std;


// typedef void(*PlatformLoadTexture)(GameAssets* gameAssets, BitmapId bitmapId);



static PlatformAPI platformAPI;

static FontId debugFontId;
static LoadedFont* debugLoadedFont;
static glm::mat4 globalDebugCameraMat;



static DebugTable* GlobalDebugTable;
int MAX_DEBUG_EVENT_ARRAY_COUNT = 8;

float FIXED_UPDATE_TIME_S = 0.016f;

/*
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
*/




// This is mirroring the sim_region struct in handmade_sim_region.h
struct GameState
{
	bool isInitalized;

	World world;

	Entity debugCameraEntity;

	bool mouseIsDebugMode;

	MemoryArena memoryArena;
};

struct TransientState
{
	bool isInitalized;
	MemoryArena memoryArena;
	GameAssets* assets;

};




#define PushRenderElement(gameRenderCommands, type) (RenderSystem::RenderGroupEntry##type*)PushRenderElement_(\
gameRenderCommands, \
RenderSystem::RenderGroupEntryType_##type, \
sizeof(RenderSystem::RenderGroupEntry##type))

void* PushRenderElement_(RenderSystem::GameRenderCommands* commands, RenderSystem::RenderGroupEntryType type, uint32 size)
{
	void* result = 0;

	size += sizeof(RenderSystem::RenderEntryHeader);
	if (commands->HasSpaceFor(size))
	{
		RenderSystem::RenderEntryHeader* header = (RenderSystem::RenderEntryHeader*)commands->CurrentPushBufferAt();
		header->type = type;
		result = (uint8*)header + sizeof(*header);

		commands->pushBufferSize += size;
		commands->numRenderGroups++;
	}
	else
	{

	}


	return result;
}






/*
// this assumes BspPolygon is a plane
void PushPlane(GameRenderCommands* gameRenderCommands, RenderGroup* renderGroup, LoadedBitmap* bitmap, glm::vec4 color, 
				BspPolygon polygon, bool fakeLighting = false)
{
	// polygon.PrintDebug();

	// 4 points on front face 
	glm::vec3 p0 = polygon.vertices[0];
	glm::vec3 p1 = polygon.vertices[1];
	glm::vec3 p2 = polygon.vertices[2];
	glm::vec3 p3 = polygon.vertices[3];

	glm::vec2 t0 = glm::vec2(0, 0);
	glm::vec2 t1 = glm::vec2(1, 0);
	glm::vec2 t2 = glm::vec2(0, 1);
	glm::vec2 t3 = glm::vec2(1, 1);

	glm::vec4 topColor = color;
	glm::vec4 bottomColor = color;

	if (fakeLighting)
	{
		bottomColor *= 0.1;
		bottomColor.a = 1;
	}

	GameRender::PushQuad_Core(gameRenderCommands, renderGroup, bitmap, p0, t0, topColor,
		p1, t1, topColor,
		p2, t2, bottomColor,
		p3, t3, bottomColor);
}

void PushPlaneOutline(GameRenderCommands* gameRenderCommands, RenderGroup* renderGroup, LoadedBitmap* bitmap, glm::vec4 color, BspPolygon polygon)
{

	// 4 points on front face
	glm::vec3 p0 = polygon.vertices[0];
	glm::vec3 p1 = polygon.vertices[1];
	glm::vec3 p2 = polygon.vertices[2];
	glm::vec3 p3 = polygon.vertices[3];

	float cubeThickness = 0.5f;


	GameRender::PushLine(gameRenderCommands, renderGroup, bitmap, color, p0, p1, cubeThickness);
	GameRender::PushLine(gameRenderCommands, renderGroup, bitmap, color, p1, p2, cubeThickness);
	GameRender::PushLine(gameRenderCommands, renderGroup, bitmap, color, p2, p3, cubeThickness);
	GameRender::PushLine(gameRenderCommands, renderGroup, bitmap, color, p3, p0, cubeThickness);

}

void PushTreeRecursive(GameRenderCommands* gameRenderCommands, RenderGroup* group, LoadedBitmap* bitmap, BSPNode* node, bool isFront, int depth)
{
	static glm::vec4 colorList[5] = { 
		GameRender::COLOR_RED, 
		GameRender::COLOR_GREEN, 
		GameRender::COLOR_BLUE, 
		GameRender::COLOR_YELLOW, 
		GameRender::COLOR_TEAL };

	bool renderFlag = true;// depth == 0;
//	bool renderFlag = depth == 1;


	if (node == NULL)
	{
		return;
	}

	// cout << "node " << node->brushes.size() << endl;
	if (node->brushes.size() > 0)
	{
		//	std::cout << "brush size" << node->brushes.size() << std::endl;

		for (int i = 0; i < node->brushes.size(); i++)
		{
			//	std::cout << "	polygon size" << node->brushes[i].polygons.size() << std::endl;

			for (int j = 0; j < node->brushes[i].polygons.size(); j++)
			{
				if (renderFlag)
				{
					if (isFront)
					{
						//	cout << "rendering front" << endl;
				//		PushPlaneOutline(gameRenderCommands, group, bitmap, COLOR_RED, node->brushes[i].polygons[j]);
					}
					else
					{
						//	cout << "rendering back" << endl;

				//		PushPlaneOutline(gameRenderCommands, group, bitmap, COLOR_BLUE, node->brushes[i].polygons[j]);
					}
				}
			}
		}
		return;
	}

	if (renderFlag)
	{
		if (!node->IsLeafNode())
		{
			glm::vec4 color = GameRender::COLOR_GREEN;
			color.a = 0.01;

			//	if (!IsAxialPlane(node->debugSplitPolygon.plane))
			{
				PushPlane(gameRenderCommands, group, bitmap, color, node->debugSplitPolygon, true);
				PushPlaneOutline(gameRenderCommands, group, bitmap, GameRender::COLOR_GREEN, node->debugSplitPolygon);
			}
		}


	}

	// render the splitting plane
	PushTreeRecursive(gameRenderCommands, group, bitmap, node->children[0], true, depth + 1);
	PushTreeRecursive(gameRenderCommands, group, bitmap, node->children[1], false, depth + 1);
}
*/

glm::vec3 GetHorizontalVector(glm::vec3 dir, bool left)
{
	glm::vec3 supportingVector = glm::vec3(0, 1, 0);

	if (glm::dot(dir, supportingVector) == 1)
	{
		supportingVector = glm::vec3(0, 0, 1);
	}

	glm::vec3 result = -glm::cross(dir, supportingVector);
	if (!left)
	{
		result = -result;
	}

	return result;
}

glm::mat4 LookAtMatrix(glm::vec3 eye, glm::vec3 direction)
{
	glm::vec3 up = glm::vec3(0, 1, 0);

	glm::mat4 result = glm::mat4(1.0);

	glm::vec3 xaxis = glm::cross(up, direction);
	xaxis = glm::normalize(xaxis);

	glm::vec3 yaxis = glm::cross(direction, xaxis);
	yaxis = glm::normalize(xaxis);

	result[0][0] = xaxis.x;
	result[1][0] = yaxis.x;
	result[2][0] = direction.x;

	result[0][1] = xaxis.y;
	result[1][1] = yaxis.y;
	result[2][1] = direction.y;

	result[0][2] = xaxis.z;
	result[1][2] = yaxis.z;
	result[2][2] = direction.z;
	//	result = glm::mat4(1.0);
	return result;
}


// this is copying glm::lookat implementation, only that we are doing the inverse ourselves
glm::mat4 GetCameraMatrix(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up)
{
	glm::vec3 f = glm::normalize(center - eye);
	glm::vec3 u = glm::normalize(up);
	glm::vec3 s = glm::normalize(glm::cross(f, u));
	u = glm::cross(s, f);

	glm::mat4 result = glm::mat4(1.0);

	result[0][0] = s.x;
	result[0][1] = s.y;
	result[0][2] = s.z;

	result[1][0] = u.x;
	result[1][1] = u.y;
	result[1][2] = u.z;

	result[2][0] = -f.x;
	result[2][1] = -f.y;
	result[2][2] = -f.z;

	result[3][0] = eye.x;
	result[3][1] = eye.y;
	result[3][2] = eye.z;

	/*
	Result[0][0] = s.x;
	Result[1][0] = s.y;
	Result[2][0] = s.z;

	Result[0][1] = u.x;
	Result[1][1] = u.y;
	Result[2][1] = u.z;

	Result[0][2] = -f.x;
	Result[1][2] = -f.y;
	Result[2][2] = -f.z;

	Result[3][0] = -dot(s, eye);
	Result[3][1] = -dot(u, eye);
	Result[3][2] = dot(f, eye);
	*/
	return result;
}

void RenderEntityPlayerModel(
	RenderSystem::GameRenderCommands* gameRenderCommands,
	RenderSystem::RenderGroup* renderGroup,
	GameAssets* gameAssets,
	Entity* entity)
{
	BitmapId bitmapID = GetFirstBitmapIdFrom(gameAssets, AssetFamilyType::Default);
	LoadedBitmap* bitmap = GetBitmap(gameAssets, bitmapID);

	glm::vec3 offset = glm::vec3(1, 1, 1);

	glm::vec3 min = entity->pos + entity->min - offset;
	glm::vec3 max = entity->pos + entity->max + offset;

	GameRender::PushCube(gameRenderCommands, renderGroup, bitmap, GameRender::COLOR_RED, min, max, true);
}

void RenderEntityStaticModel(
	RenderSystem::GameRenderCommands* gameRenderCommands,
	RenderSystem::RenderGroup* renderGroup,
	GameAssets* gameAssets,
	Entity* entity)
{
	BitmapId bitmapID = GetFirstBitmapIdFrom(gameAssets, AssetFamilyType::Wall);
	LoadedBitmap* bitmap = GetBitmap(gameAssets, bitmapID);

	for (int i = 0; i < entity->model.size(); i++)
	{
		GameRender::PushQuad(gameRenderCommands, renderGroup, bitmap,
			entity->model[i].vertices[0],
			entity->model[i].vertices[1],
			entity->model[i].vertices[2],
			entity->model[i].vertices[3], GameRender::COLOR_WHITE);
	}
}

void RenderEntityGroundModel(
	RenderSystem::GameRenderCommands* gameRenderCommands,
	RenderSystem::RenderGroup* renderGroup,
	GameAssets* gameAssets,
	Entity* entity)
{
	BitmapId bitmapID = GetFirstBitmapIdFrom(gameAssets, AssetFamilyType::Chessboard);
	LoadedBitmap* bitmap = GetBitmap(gameAssets, bitmapID);

	for (int i = 0; i < entity->model.size(); i++)
	{
		GameRender::PushQuad(gameRenderCommands, renderGroup, bitmap,
			entity->model[i].vertices[0],
			entity->model[i].vertices[1],
			entity->model[i].vertices[2],
			entity->model[i].vertices[3], GameRender::COLOR_WHITE);
	}
}

void RenderNavMeshPolygon(
	RenderSystem::GameRenderCommands* gameRenderCommands,
	RenderSystem::RenderGroup* renderGroup,
	GameAssets* gameAssets,
	NavMesh::NavMeshPolygon* polygon)
{
	BitmapId bitmapID = GetFirstBitmapIdFrom(gameAssets, AssetFamilyType::Default);
	LoadedBitmap* bitmap = GetBitmap(gameAssets, bitmapID);

	float lineThickness = 0.2f;

	for (int i = 0; i < polygon->vertices.size(); i++)
	{
		glm::vec3 p0 = polygon->vertices[i];
		glm::vec3 p1;

		if (i == polygon->vertices.size() - 1)
		{
			p1 = polygon->vertices[0];
		}
		else
		{
			p1 = polygon->vertices[i + 1];
		}
		GameRender::PushLine(gameRenderCommands, renderGroup, bitmap, GameRender::COLOR_RED, p0, p1, lineThickness);

		// vertex
		glm::vec3 offset = glm::vec3(0.5, 0.5, 0.5);
		glm::vec3 min = p0 - offset;
		glm::vec3 max = p0 + offset;
		

		GameRender::PushCube(gameRenderCommands, renderGroup, bitmap, GameRender::COLOR_BLUE, min, max, true);
	}

	if (polygon->vertices.size() == 3)
	{
		glm::vec4 shadedRed = glm::vec4(0.1, 0, 0, 0.4);
		std::vector<glm::vec3> liftedVertex;
		for (int j = 0; j < polygon->vertices.size(); j++)
		{
			// lifting it slightly higher
			liftedVertex.push_back(polygon->vertices[j] + glm::vec3(0,0.1,0));
		}
		GameRender::PushTriangle(gameRenderCommands, renderGroup, bitmap, shadedRed, liftedVertex, false);
	}
}


void CatagorizePosition(World* world, Entity* entity)
{
	glm::vec3 end = entity->pos;
	end[1] -= 0.25;

	TraceResult result = BoxTrace(entity->pos, end, entity->min, entity->max, world->tree);
//	cout << "result.timeFraction " << result.timeFraction << endl;

	if(result.plane == NULL_PLANE && result.outputStartsOut)
	{
//		cout << "	has no ground entity ";

		entity->groundEntity = NULL;
		entity->groundPlane = NULL_PLANE;
	}
	else 
	{
//		cout << "	has ground entity ";
		entity->groundEntity = (Entity*)1;
		entity->groundPlane = result.plane;
	}
}


struct PlayerMoveData
{
//	glm::vec3 position;
	glm::vec3 velocity;
};

void PerformMove(World* world, Entity* entity, PlayerMoveData* move)
{
	glm::vec3 origin = entity->pos;
	glm::vec3 velocity = move->velocity;

	int numClippingPlaces = 0;
	float timeLeft = 0.1f;// FIXED_UPDATE_TIME_S;
	for (int i = 0; i < 1; i++)
	{
		glm::vec3 end = origin + timeLeft * velocity;

		cout << "origin " << origin << endl;
		cout << "end " << end << endl;

		TraceResult result = BoxTrace(origin, end, entity->min, entity->max, world->tree);

		if (result.outputAllSolid)
		{
			move->velocity = glm::vec3(0);
			return;
		}
	
		if (result.timeFraction > 0)
		{
			entity->pos = result.endPos;
			numClippingPlaces = 0;
		}
		else if (result.timeFraction == 0)
		{

		}
		else if (result.timeFraction == 1)
		{
			break;
		}

		timeLeft -= timeLeft * result.timeFraction;


		numClippingPlaces++;
	}



}



void PerformMove2(World* world, Entity* entity, PlayerMoveData* move)
{
	glm::vec3 origin = entity->pos;
	glm::vec3 velocity = move->velocity;

	int numClippingPlaces = 0;
	float timeLeft = 0.1f;// FIXED_UPDATE_TIME_S;
	for (int i = 0; i < 1; i++)
	{
		glm::vec3 end = origin + timeLeft * velocity;

		if (origin.y == 10.5)
		{
			int a = 1;
		}

		cout << "origin " << origin << endl;
		cout << "end " << end << endl;

		TraceResult result = BoxTrace(origin, end, entity->min, entity->max, world->tree, true);

		cout << "result time fraction " << result.timeFraction << endl;

		if (result.outputAllSolid)
		{
			move->velocity = glm::vec3(0);
			return;
		}

		if (result.timeFraction > 0)
		{
			entity->pos = result.endPos;
			numClippingPlaces = 0;
		}
		else if (result.timeFraction == 0)
		{

		}
		else if (result.timeFraction == 1)
		{
			break;
		}

		timeLeft -= timeLeft * result.timeFraction;


		numClippingPlaces++;
	}



}



void EntityMoveTick(World* world, Entity* entity, PlayerMoveData* move, bool applyGravity)
{
	if (entity->groundEntity != NULL)
	{

		if (move->velocity.x != 0 || move->velocity.z != 0)
		{
			std::cout << "ground entity" << std::endl;

			PerformMove2(world, entity, move);
		}
	}
	else
	{
	//	std::cout << "in air" << std::endl;
		if (applyGravity)
		{
			static glm::vec3 GRAVITY = glm::vec3(0, -5, 0);
			move->velocity += GRAVITY;
		}

		if (move->velocity.x != 0 || move->velocity.y != 0 || move->velocity.z != 0)
		{
			PerformMove2(world, entity, move);
		}
	}
}


void PlayerMove(World* world, Entity* entity, PlayerMoveData* move, bool applyGravity)
{
	// categorize current position
	CatagorizePosition(world, entity);

	// slide move
	EntityMoveTick(world, entity, move, applyGravity);

	// categoize current position 2
	CatagorizePosition(world, entity);
}


glm::vec3 UpdateEntityViewDirection(Entity* entity, GameInputState* gameInputState, glm::ivec2 windowDimensions)
{
	float angleXInDeg = 0;
	float angleYInDeg = 0;

	float dx = gameInputState->mousePos.x - windowDimensions.x / 2;
	float dy = gameInputState->mousePos.y - windowDimensions.y / 2;


	angleXInDeg = dx * 0.05f;
	angleYInDeg = dy * 0.05f;

	if (entity->pitch + angleYInDeg >= 179)
	{
		angleYInDeg = 179 - entity->pitch;
	}

	if (entity->pitch + angleYInDeg <= -179)
	{
		angleYInDeg = -179 - entity->pitch;
	}

	entity->pitch += angleYInDeg;

	// rotate around x with dy then rotate around Y with dx
	glm::vec3 newViewDir = glm::vec3(glm::rotate(angleYInDeg, entity->xAxis) *
		glm::rotate(-angleXInDeg, entity->yAxis) * glm::vec4(entity->GetViewDirection(), 1));

	newViewDir = glm::normalize(newViewDir);
	return newViewDir;
}


void WorldTickAndRender(GameState* gameState, GameAssets* gameAssets,
	GameInputState* gameInputState, RenderSystem::GameRenderCommands* gameRenderCommands, glm::ivec2 windowDimensions, DebugModeState* debugModeState)
{
	Entity* controlledEntity = &gameState->debugCameraEntity;

	World* world = &gameState->world;
	glm::vec3 newViewDir;
	if (debugModeState->cameraDebugMode)
	{
		controlledEntity = &gameState->debugCameraEntity;
	}
	else
	{
		controlledEntity = &world->entities[world->startPlayerEntityId];
	}

	if (!debugModeState->mouseDebugMode)
	{
		newViewDir = UpdateEntityViewDirection(controlledEntity, gameInputState, windowDimensions);
	}
	else
	{
		newViewDir = controlledEntity->GetViewDirection();
	}

	glm::vec3 newWalkDir;
	if (debugModeState->cameraDebugMode)
	{
		newWalkDir = newViewDir;
	}
	else
	{
		newWalkDir = newViewDir;
		newWalkDir.y = 0;
		newWalkDir = glm::normalize(newWalkDir);
	}

	//	cam->SetViewDirection(newViewDir);

	PlayerMoveData pmove = {};

	// process input
	float stepSize = 40.0f;
//	if (debugModeState->cameraDebugMode)
	{
		if (gameInputState->moveForward.endedDown) {
//			newWalkDir.y = 0;
			pmove.velocity += stepSize * newWalkDir;
		}
		if (gameInputState->moveLeft.endedDown) {
			newWalkDir = GetHorizontalVector(newWalkDir, true);
//			newWalkDir.y = 0;
			pmove.velocity += stepSize * newWalkDir;
		}
		if (gameInputState->moveRight.endedDown) {
			newWalkDir = GetHorizontalVector(newWalkDir, false);
//			newWalkDir.y = 0;
			pmove.velocity += stepSize * newWalkDir;
		}
		if (gameInputState->moveBack.endedDown) {
//			newWalkDir.y = 0;
			pmove.velocity += -stepSize * newWalkDir;
		}
	}
	/*
	else
	{
		if (gameInputState->moveForward.endedDown) {
			newWalkDir = glm::vec3(1, 0, 0);
			pmove.velocity += stepSize * newWalkDir;
		}
		if (gameInputState->moveLeft.endedDown) {
			newWalkDir = glm::vec3(0, 0, 1);
			pmove.velocity += stepSize * newWalkDir;
		}
		if (gameInputState->moveRight.endedDown) {
			newWalkDir = glm::vec3(0, 0, 1);
			pmove.velocity += -stepSize * newWalkDir;
		}
		if (gameInputState->moveBack.endedDown) {
			newWalkDir = glm::vec3(1, 0, 0);
			pmove.velocity += -stepSize * newWalkDir;
		}
	}
	*/

	stepSize = 0.1f;

	if (gameInputState->moveForward2.endedDown){world->entities[world->startPlayerEntityId].pos.x += stepSize;}
	if (gameInputState->moveBack2.endedDown){world->entities[world->startPlayerEntityId].pos.x += -stepSize;}

	if (gameInputState->moveLeft2.endedDown){world->entities[world->startPlayerEntityId].pos.z += stepSize;}
	if (gameInputState->moveRight2.endedDown){world->entities[world->startPlayerEntityId].pos.z += -stepSize;}

	if (gameInputState->moveUp2.endedDown)	{world->entities[world->startPlayerEntityId].pos.y += stepSize;}
	if (gameInputState->moveDown2.endedDown){world->entities[world->startPlayerEntityId].pos.y += -stepSize;}

//	if (!debugModeState->cameraDebugMode)
	{
		// Update Player movement
	//	cout << "cur position " << world->entities[world->startPlayerEntityId].pos << endl;
		PlayerMove(world, controlledEntity, &pmove, !debugModeState->cameraDebugMode);
	}

	// world->entities[world->startPlayerEntityId].pos = pmove.position;
	world->entities[world->startPlayerEntityId].velocity = pmove.velocity;


	// Update camera matrix
	glm::vec3 center = controlledEntity->pos + newViewDir;
	glm::vec3 supportUpVector = glm::vec3(0, 1, 0);
	if (glm::dot(newViewDir, supportUpVector) == 1)
	{
		supportUpVector = controlledEntity->yAxis;
	}


	glm::mat4 cameraMatrix = GetCameraMatrix(controlledEntity->pos, center, supportUpVector);
	controlledEntity->SetOrientation(cameraMatrix);

	globalDebugCameraMat = cameraMatrix;

	glm::mat4 cameraTransform = glm::translate(controlledEntity->pos);// *cameraRot;
	float dim = 20;
	glm::mat4 cameraProj = glm::perspective(45.0f, windowDimensions.x / (float)windowDimensions.y, 0.5f, 5000.0f);





	// We start a render setup
	RenderSystem::RenderGroup group = {};

	RenderSystem::RenderSetup renderSetup = {};
	//	renderSetup.cameraProjectionMatrix = cameraProj;
	//	renderSetup.cameraTransformMatrix = cameraTransform;
	renderSetup.transformMatrix = cameraProj * glm::inverse(cameraMatrix);

	group.quads = PushRenderElement(gameRenderCommands, TexturedQuads);
	*group.quads = {};
	group.quads->masterVertexArrayOffset = gameRenderCommands->numVertex;
	group.quads->masterBitmapArrayOffset = gameRenderCommands->numBitmaps;
	group.quads->renderSetup = renderSetup;


	for (int i = 0; i < world->numEntities; i++)
	{
		Entity* entity = &world->entities[i];
		switch (entity->flag)
		{
			case EntityFlag::STATIC:
				// RenderEntityStaticModel(gameRenderCommands, &group, gameAssets, entity);
				break;

			case EntityFlag::GROUND:
				RenderEntityGroundModel(gameRenderCommands, &group, gameAssets, entity);
				break;

			case EntityFlag::PLAYER:
				/*
				if (debugModeState->cameraDebugMode)
				{
					RenderEntityPlayerModel(gameRenderCommands, &group, gameAssets, entity);
				}
				*/
				break;
		}	
	}

	for (int i = 0; i < world->navMeshPolygons.size(); i++)
	{
		NavMesh::NavMeshPolygon* navMeshPolygon = &world->navMeshPolygons[i];
		RenderNavMeshPolygon(gameRenderCommands, &group, gameAssets, navMeshPolygon);
	}


	GameRender::RenderCoordinateSystem(gameRenderCommands, &group, gameAssets);
}


extern DebugTable* globalDebugTable;


extern "C" __declspec(dllexport) void GameUpdateAndRender(GameMemory * gameMemory, GameInputState * gameInputState, RenderSystem::GameRenderCommands * gameRenderCommands,
	glm::ivec2 windowDimensions, DebugModeState* debugModeState)
{
	//	cout << "Update And Render-2" << endl;

		// so all my game state resides in memory

		// initalize, just init 20 randome entities

		// allocate memroy for entities

	if (gameInputState->moveForward.endedDown)
	{
		//	cout << "move forward" << endl;
	}
	if (gameInputState->moveLeft.endedDown)
	{
		//	cout << "move left" << endl;
	}
	if (gameInputState->moveRight.endedDown)
	{
		//	cout << "move right" << endl;
	}
	if (gameInputState->moveBack.endedDown)
	{
		//	cout << "move back" << endl;
	}

	globalDebugTable = gameMemory->debugTable;


	GameState* gameState = (GameState*)gameMemory->permenentStorage;
	if (!gameState->isInitalized)
	{
		// intialize memory arena
		platformAPI = gameMemory->platformAPI;


		initWorld(&gameState->world);

		gameState->debugCameraEntity = {};
		gameState->debugCameraEntity.pos = glm::vec3(-262, 500, -75);
		gameState->debugCameraEntity.xAxis = glm::vec3(0.0, 0.0, 1.0);
		gameState->debugCameraEntity.yAxis = glm::vec3(1.0, 0.0, 0.0);
		gameState->debugCameraEntity.zAxis = glm::vec3(0.0, 1.0, 0.0);
		gameState->debugCameraEntity.min = glm::vec3(-10, -10, -10);
		gameState->debugCameraEntity.max = glm::vec3(10, 10, 10);

		gameState->mouseIsDebugMode = false;


		uint8* base = (uint8*)gameMemory->permenentStorage + sizeof(GameState);
		MemoryIndex size = gameMemory->permenentStorageSize - sizeof(GameState);
		gameState->memoryArena.Init(base, size);

		gameState->isInitalized = true;


		// CallOpenGL To Load Textures
	}

	TransientState* transientState = (TransientState*)gameMemory->transientStorage;
	if (!transientState->isInitalized)
	{
		uint8* base = (uint8*)gameMemory->transientStorage + sizeof(TransientState);
		MemoryIndex size = gameMemory->transientStorageSize - sizeof(TransientState);
		transientState->memoryArena.Init(base, size);

		transientState->assets = PushStruct(&transientState->memoryArena, GameAssets);
		AllocateGameAssets(&transientState->memoryArena, transientState->assets);

		debugFontId = { GetFirstAssetIdFrom(transientState->assets, AssetFamilyType::Enum::Font) };
		debugLoadedFont = GetFont(transientState->assets, debugFontId);


		transientState->isInitalized = true;
	}

	/*
	for (int i = 0; i < 5000; i++)
	{
		cout << "nice";
	}
	*/

	WorldTickAndRender(gameState, transientState->assets, gameInputState, gameRenderCommands, windowDimensions, debugModeState);
}



void AllocateDebugFrame(DebugFrame* debugFrame, MemoryArena* memoryArena)
{
	debugFrame->beginClock = 0;
	debugFrame->endClock = 0;
	debugFrame->wallSecondsElapsed = 0.0f;
	debugFrame->rootProfileNode = new ProfileNode(-1);

	std::cout << "debugFrame->rootProfileNode " << debugFrame->rootProfileNode << std::endl;
}


void AllocateDebugFrames(DebugState* debugState)
{
	debugState->frames = PushArray(&debugState->collationArena, MAX_DEBUG_EVENT_ARRAY_COUNT, DebugFrame);
	debugState->numFrames = 0;
	debugState->maxFrames = MAX_DEBUG_EVENT_ARRAY_COUNT;
	debugState->collationFrame = 0;

	// allocating DebugFrame
	for (int i = 0; i < MAX_DEBUG_EVENT_ARRAY_COUNT; i++)
	{
		AllocateDebugFrame(&debugState->frames[i], &debugState->collationArena);
	}
}


void RestartCollation(DebugState* debugState)
{
	debugState->numFrames = 0;
	debugState->collationFrame = 0;
}


glm::vec3 PlatformMouseToScreenRenderPos(RenderSystem::GameRenderCommands* gameRenderCommands, glm::ivec2 mousePos)
{
	float halfWidth = gameRenderCommands->settings.dims.x / 2.0f;
	float halfHeight = gameRenderCommands->settings.dims.y / 2.0f;
	return glm::vec3(mousePos.x - halfWidth, mousePos.y - halfHeight, 0);
}



void DEBUGTextLine(char* s, RenderSystem::GameRenderCommands* gameRenderCommands, RenderSystem::RenderGroup* group, GameAssets* gameAssets, glm::vec3 position)
{
	// how big do we want char to be displayed
	const float DEBUG_CHAR_BITMAP_SCALE = 1;

	int ascent = 0;
	int descent = 0;
	int lineGap = 0;
	stbtt_GetFontVMetrics(&debugLoadedFont->fontInfo, &ascent, &descent, &lineGap);
	float scale = stbtt_ScaleForPixelHeight(&debugLoadedFont->fontInfo, FONT_SCALE);

	int lineGapBetweenNextBaseline = (ascent - descent + lineGap);
	int scaledLineGap = (int)(lineGapBetweenNextBaseline * scale);

	float xPos = position.x;
	int yBaselinePos = position.y;

	// This is essentially following the example from stb library
//	for (int i = 0; i < size; i++)

	int i = 0;
	while (s[i] != '\0')
	{
		int advance, leftSideBearing;
		stbtt_GetCodepointHMetrics(&debugLoadedFont->fontInfo, s[i], &advance, &leftSideBearing);

		GlyphId glyphID = GetGlyph(gameAssets, debugLoadedFont, s[i]);
		LoadedGlyph* glyphBitmap = GetGlyph(gameAssets, glyphID);

		if (s[i] == '\n')
		{
			xPos = position.x;
			yBaselinePos -= scaledLineGap;
		}
		else
		{
			float height = DEBUG_CHAR_BITMAP_SCALE * glyphBitmap->bitmap.height;
			float width = glyphBitmap->bitmap.width / (float)glyphBitmap->bitmap.height * height;

			int x = xPos + glyphBitmap->bitmapXYOffsets.x;
			int y = yBaselinePos - glyphBitmap->bitmapXYOffsets.y;

			glm::vec3 leftTopPos = glm::vec3(x, y, 0.2);

			GameRender::PushBitmap(
				gameRenderCommands, 
				group, 
				&glyphBitmap->bitmap, 
				GameRender::COLOR_WHITE, 
				leftTopPos, 
				glm::vec3(width / 2.0, height / 2.0, 0), 
				GameRender::AlignmentMode::Left, GameRender::AlignmentMode::Top);


			xPos += (advance * scale);
			xPos += scale * stbtt_GetCodepointKernAdvance(&debugLoadedFont->fontInfo, s[i], s[i + 1]);
		}
		i++;
	}

}

void RenderProfileBars(DebugState* debugState, RenderSystem::GameRenderCommands* gameRenderCommands,
	RenderSystem::RenderGroup* renderGroup, GameAssets* gameAssets, glm::ivec2 mousePos)
{

	// we have lanes for our threads 
	// handmade hero day254 24:52
	float halfWidth = gameRenderCommands->settings.dims.x / 2.0f;
	float halfHeight = gameRenderCommands->settings.dims.y / 2.0f;

	BitmapId bitmapID = GetFirstBitmapIdFrom(gameAssets, AssetFamilyType::Default);
	LoadedBitmap* defaultBitmap = GetBitmap(gameAssets, bitmapID);

	glm::vec3 profileRectDim = glm::vec3(200, 100, 0);
	glm::vec3 profileRectMin = glm::vec3(-halfWidth, halfHeight - profileRectDim.y, 0);
	glm::vec3 profileRectMax = profileRectMin + profileRectDim;
	
	// background
	GameRender::PushBitmap(gameRenderCommands, renderGroup, defaultBitmap, glm::vec4(0, 0, 0.25, 0.25), profileRectMin,
		glm::vec3(profileRectDim.x / 2.0, profileRectDim.y / 2.0, 0), GameRender::AlignmentMode::Left, GameRender::AlignmentMode::Bottom);

	//cout << "RenderProfileBars " << endl;
	if (debugState->mostRecentFrame != NULL)
	{
		// debugState->mostRecentFrame->PrintDebug();


		ProfileNode* root = debugState->mostRecentFrame->rootProfileNode;

		float frameSpan = root->duration;
		float pixelSpan = profileRectDim.x;

		float scale = 0.0f;
		if (frameSpan > 0)
		{
			scale = pixelSpan / frameSpan;
		}


		uint32 numLanes = debugState->threads.size();
		float laneHeight = 0.0f;
		if (numLanes > 0)
		{
			laneHeight = profileRectDim.y / numLanes;
		}



		glm::vec4 colors[] = {
			glm::vec4(1,0,0,1),
			glm::vec4(0,1,0,1),
			glm::vec4(0,0,1,1),

			glm::vec4(1,1,0,1),
			glm::vec4(0,1,1,1),
			glm::vec4(1,0,1,1)
		};


		// cout << "root->children " << root->children.size() << endl;
		// the more recent ones are at the top
		for (uint32 i = 0; i < root->children.size(); i++)
		{
			ProfileNode* node = root->children[i];
			glm::vec4 color = colors[i % ArrayCount(colors)];

			glm::vec3 rectMin, rectMax;

			rectMin.x = profileRectMin.x + scale * node->parentRelativeClock;
			rectMax.x = rectMin.x + scale * node->duration;

			uint32 laneIndex = 0;
			rectMin.y = profileRectMax.y - (laneIndex + 1) * laneHeight;
			rectMax.y = profileRectMax.y - laneIndex * laneHeight;

			glm::vec3 dim = rectMax - rectMin;

			float zOffset = 0.1;

			GameRender::PushBitmap(gameRenderCommands, renderGroup, defaultBitmap, color, glm::vec3(rectMin.x, rectMin.y, zOffset),
				glm::vec3(dim.x / 2.0, dim.y / 2.0, 0), GameRender::AlignmentMode::Left, GameRender::AlignmentMode::Bottom);

			// if mouse in region
			glm::vec3 screenMousePos = PlatformMouseToScreenRenderPos(gameRenderCommands, mousePos);
			if (Collision::IsPointInsideRect({ rectMin, rectMax }, screenMousePos))
			{
				DEBUGTextLine(node->element->GUID, gameRenderCommands, renderGroup, gameAssets, screenMousePos);
			}
		}
	}
}

extern "C" __declspec(dllexport) void DebugSystemUpdateAndRender(GameMemory * gameMemory,
	GameInputState * gameInputState,
	RenderSystem::GameRenderCommands * gameRenderCommands,
	glm::ivec2 windowDimensions, DebugModeState* debugModeState)
{
	DebugState* debugState = (DebugState*)gameMemory->debugStorage;
	if (!debugState->isInitalized)
	{
		uint64 collationArenaSize = Megabytes(32);

		uint8* debugArenaBase = (uint8*)gameMemory->debugStorage + sizeof(DebugState);
		MemoryIndex debugArenaSize = gameMemory->debugStorageSize - sizeof(DebugState) - collationArenaSize;
		debugState->debugArena.Init(debugArenaBase, debugArenaSize);

		debugState->renderGroup = PushStruct(&debugState->debugArena, RenderSystem::RenderGroup);

		uint8* collationArenaBase = (uint8*)gameMemory->debugStorage + sizeof(DebugState) + debugArenaSize;
		debugState->collationArena.Init(collationArenaBase, collationArenaSize);

		AllocateDebugFrames(debugState);
		debugState->threads = std::vector<DebugThread>();
		debugState->debugElements = std::vector<DebugElement>();

		debugState->isInitalized = true;
	}

	GameState* gameState = (GameState*)gameMemory->permenentStorage;
	TransientState* transientState = (TransientState*)gameMemory->transientStorage;


	float halfWidth = gameRenderCommands->settings.dims.x / 2.0f;
	float halfheight = gameRenderCommands->settings.dims.y / 2.0f;

	glm::mat4 cameraProj = glm::ortho(-halfWidth, halfWidth, -halfheight, halfheight);


	// We start a render setup
	RenderSystem::RenderGroup group = {};

	RenderSystem::RenderSetup renderSetup = {};
	renderSetup.transformMatrix = cameraProj;// *glm::inverse(globalDebugCameraMat);



	group.quads = PushRenderElement(gameRenderCommands, TexturedQuads);
	*group.quads = {};
	group.quads->masterVertexArrayOffset = gameRenderCommands->numVertex;
	group.quads->masterBitmapArrayOffset = gameRenderCommands->numBitmaps;
	group.quads->renderSetup = renderSetup;


	uint64 arrayIndex_eventIndex = globalDebugTable->eventArrayIndex_EventIndex;

	// get the top 32 bit
	uint32 eventArrayIndex = arrayIndex_eventIndex >> 32;
	// we want the ladder 32 bit
	uint32 numEvents = arrayIndex_eventIndex & 0xFFFFFFFF;
	// cout << "		before eventArrayIndex " << eventArrayIndex << ", numEvents " << numEvents << endl;

	// we change the array we want to write to
	uint64 newEventArrayIndex = !eventArrayIndex;
	globalDebugTable->eventArrayIndex_EventIndex = (uint64)(newEventArrayIndex << 32);

	uint32 eventArrayIndex2 = globalDebugTable->eventArrayIndex_EventIndex >> 32;
	uint32 numEvents2 = globalDebugTable->eventArrayIndex_EventIndex & 0xFFFFFFFF;
	// cout << "		after new eventArrayIndex " << eventArrayIndex2 << ", numEvents " << numEvents2 << endl;


	// one debug event array is almost a frame worth of stuff

	if (debugState->numFrames >= MAX_DEBUG_EVENT_ARRAY_COUNT)
	{
		RestartCollation(debugState);
	}

	// Assuming we aren't recording debugEvents multithreadedly
	// cout << "		before eventArrayIndex " << eventArrayIndex << ", numEvents " << numEvents << endl;
	ProcessDebugEvents(debugState, globalDebugTable->events[eventArrayIndex], numEvents);

	// Render Debug stuff

	RenderProfileBars(debugState, gameRenderCommands, &group, transientState->assets, gameInputState->mousePos);

	glm::vec3 startPos = glm::vec3(-halfWidth, halfheight - 120, 0.2);
	static char buffer[512];
	char* ptr = buffer;
	int size = 0;

	if (debugState->mostRecentFrame)
	{
		size = sprintf(ptr, "Last frame time: %.02fms", debugState->mostRecentFrame->wallSecondsElapsed * 1000.0f);
		ptr += size;
	}

	size = sprintf(ptr, "\n");
	ptr += size;

	size = sprintf(ptr, "CameraPos is %f %f %f\n", gameState->debugCameraEntity.pos.x, gameState->debugCameraEntity.pos.y, gameState->debugCameraEntity.pos.z);
	ptr += size;

	size = sprintf(ptr, "xDir is %f %f %f\n", gameState->debugCameraEntity.xAxis.x, gameState->debugCameraEntity.xAxis.y, gameState->debugCameraEntity.xAxis.z);
	ptr += size;

	size = sprintf(ptr, "yDir is %f %f %f\n", gameState->debugCameraEntity.yAxis.x, gameState->debugCameraEntity.yAxis.y, gameState->debugCameraEntity.yAxis.z);
	ptr += size;

	size = sprintf(ptr, "zDir is %f %f %f\n", gameState->debugCameraEntity.zAxis.x, gameState->debugCameraEntity.zAxis.y, gameState->debugCameraEntity.zAxis.z);
	ptr += size;

	DEBUGTextLine(buffer, gameRenderCommands, &group, transientState->assets, startPos);



}
