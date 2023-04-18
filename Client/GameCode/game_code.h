#pragma once

#include "../PlatformShared/platform_shared.h"
#include "collision.h"

//#include "debug_interface.h"
#include "memory.h"
#include "world.h"
#include "game_state.h"
#include "../NavMesh/asset.h"
#include "debug.h"
#include "game_render.h"
#include "game_io.h"
#include <iostream>
#include "editor/editor.h"
#include "pathfinding.h"



#include "json_spirit\json_spirit.h"
#include "json_spirit\json_spirit_reader_template.h"
#include "json_spirit\json_spirit_writer_template.h"


// define memory for Push style stuff
#ifndef JSON_SPIRIT_MVALUE_ENABLED
#error Please define JSON_SPIRIT_MVALUE_ENABLED for the mValue type to be enabled 
#endif

using namespace std;
using namespace json_spirit;

// typedef void(*PlatformLoadTexture)(GameAssets* gameAssets, BitmapId bitmapId);



static PlatformAPI platformAPI;

//static FontId debugFontId;
//static LoadedFont* debugLoadedFont;
static glm::mat4 globalDebugCameraMat;



static DebugTable* GlobalDebugTable;
int MAX_DEBUG_EVENT_ARRAY_COUNT = 8;

float FIXED_UPDATE_TIME_S = 0.016f;

const float DEBUG_CHAR_BITMAP_SCALE = 1;

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
not entirely sure why this works... 
https://antongerdelan.net/opengl/raycasting.html

	mousePos

  0,768					1023,768
	  ______________________
	 |						|
	 |						|
	 |			  			|
	 |						|
	 |						|
	 |______________________|
   0,0						1023,0

*/
glm::vec3 MousePosToMousePickingRay(WorldCameraSetup worldCameraSetup, RenderSystem::GameRenderCommands* gameRenderCommands, glm::ivec2 mousePos)
{
	float halfWidth = gameRenderCommands->settings.dims.x / 2.0f;
	float halfHeight = gameRenderCommands->settings.dims.y / 2.0f;
	glm::vec3 deviceCoordinate = glm::vec3(mousePos.x - halfWidth, mousePos.y - halfHeight, 0);

	// noralized device coordinate
	glm::vec3 rayNoralizedCoordinate;
	rayNoralizedCoordinate.x = deviceCoordinate.x / halfWidth;
	rayNoralizedCoordinate.y = deviceCoordinate.y / halfHeight;
	rayNoralizedCoordinate.z = 1;

	// we have z = -1 because we want our ray to point into the screen
	glm::vec4 rayClip = glm::vec4(rayNoralizedCoordinate.x, rayNoralizedCoordinate.y, -1, 1);
	
	glm::vec4 rayEye = glm::inverse(worldCameraSetup.proj) * rayClip;
	rayEye = glm::vec4(rayEye.x, rayEye.y, 1, 0);

	glm::vec4 rayWorld4d = glm::inverse(worldCameraSetup.view) * rayEye;
	glm::vec3 rayWorld = glm::vec3(rayWorld4d.x, rayWorld4d.y, -rayWorld4d.z);
	rayWorld = glm::normalize(rayWorld);
	return rayWorld;

}



void RenderMiddle(RenderSystem::GameRenderCommands* gameRenderCommands,
	RenderSystem::RenderGroup* renderGroup, GameAssets* gameAssets, glm::ivec2 mousePos)
{

	BitmapId bitmapID = GetFirstBitmapIdFrom(gameAssets, AssetFamilyType::Default);
	LoadedBitmap* defaultBitmap = GetBitmap(gameAssets, bitmapID);

	glm::vec3 halfDim = glm::vec3(1, 1, 0);
	glm::vec3 rectMin = glm::vec3(-halfDim.x, -halfDim.y, 0);
	glm::vec3 rectMax = glm::vec3(halfDim.x, halfDim.y, 0);

	// background
	GameRender::PushBitmap(gameRenderCommands, renderGroup, defaultBitmap, glm::vec4(1, 1, 1, 1), rectMin,
		rectMax, GameRender::AlignmentMode::Left, GameRender::AlignmentMode::Bottom);

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
			glm::vec3 screenMousePos = UIUtil::PlatformMouseToScreenRenderPos(gameRenderCommands, mousePos);
			/*
			if (Collision::IsPointInsideRect({ rectMin, rectMax }, screenMousePos))
			{
				GameRender::DEBUGTextLine(node->element->GUID, gameRenderCommands, renderGroup, gameAssets, screenMousePos);
			}
			*/
		}
	}
}

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

void RenderAgentEntity(
	RenderSystem::GameRenderCommands* gameRenderCommands,
	RenderSystem::RenderGroup* renderGroup,
	GameAssets* gameAssets,
	Entity* entity)
{
	BitmapId bitmapID = GetFirstBitmapIdFrom(gameAssets, AssetFamilyType::Default);
	LoadedBitmap* bitmap = GetBitmap(gameAssets, bitmapID);

	GameRender::RenderCircle(
		gameRenderCommands, renderGroup, gameAssets, GameRender::COLOR_RED, entity->pos, entity->agentRadius, 0.5);
}

void RenderObstacleEntity(
	RenderSystem::GameRenderCommands* gameRenderCommands,
	RenderSystem::RenderGroup* renderGroup,
	GameAssets* gameAssets,
	Entity* entity)
{
	BitmapId bitmapID = GetFirstBitmapIdFrom(gameAssets, AssetFamilyType::Wall);
	LoadedBitmap* bitmap = GetBitmap(gameAssets, bitmapID);

	float lineThickness = 0.5;
	glm::vec3 translate = entity->pos;
	for (int i = 0; i < entity->vertices.size(); i++)
	{
		glm::vec3 pos0 = entity->vertices[i];
		glm::vec3 pos1;
		if (i == entity->vertices.size() - 1)
		{
			pos1 = entity->vertices[0];
		}
		else
		{
			pos1 = entity->vertices[i + 1];
		}

		pos0 += translate;
		pos1 += translate;

		GameRender::RenderLine(
			gameRenderCommands, renderGroup, gameAssets, GameRender::COLOR_RED, pos0, pos1, lineThickness);
	}


	/*
	for (int i = 0; i < entity->model.size(); i++)
	{
		GameRender::PushQuad(gameRenderCommands, renderGroup, bitmap,
			entity->model[i].vertices[0],
			entity->model[i].vertices[1],
			entity->model[i].vertices[2],
			entity->model[i].vertices[3], GameRender::COLOR_WHITE);
	}
	*/
}


void RenderTriangulationDebug(RenderSystem::GameRenderCommands* gameRenderCommands,
	RenderSystem::RenderGroup* group,
	GameAssets* gameAssets,
	Triangulation::DebugState* triangulationDebug)
{
	float lineThickness = 0.5;

	BitmapId bitmapID = GetFirstBitmapIdFrom(gameAssets, AssetFamilyType::Default);
	LoadedBitmap* bitmap = GetBitmap(gameAssets, bitmapID);

	float thickness = 0.2f;
	for (int i = 0; i < triangulationDebug->triangles.size(); i++)
	{
		Triangulation::Triangle* triangle = &triangulationDebug->triangles[i];
		glm::vec3 liftedVertex[3];

		for (int j = 0; j < Triangulation::NUM_TRIANGLE_VERTEX; j++)
		{
			// lifting it slightly higher
			liftedVertex[j] = triangle->vertices[j] + GameRender::DEBUG_RENDER_OFFSET;
		}

		GameRender::PushTriangleOutline(gameRenderCommands, group, bitmap, GameRender::COLOR_WHITE, liftedVertex, thickness, false);
	}

	
	for (int i = 0; i < triangulationDebug->circles.size(); i++)
	{
		Triangulation::Circle* circle = &triangulationDebug->circles[i];

		GameRender::RenderCircle(
			gameRenderCommands, group, gameAssets, GameRender::COLOR_RED, circle->center, circle->radius, lineThickness);
	}
	
}



void RenderVoronoiDebug(RenderSystem::GameRenderCommands* gameRenderCommands,
	RenderSystem::RenderGroup* group,
	GameAssets* gameAssets,
	Voronoi::DebugState* voronoiDebug)
{
	BitmapId bitmapID = GetFirstBitmapIdFrom(gameAssets, AssetFamilyType::Default);
	LoadedBitmap* bitmap = GetBitmap(gameAssets, bitmapID);

	float pointThickness = 0.2f;
	float lineThickness = 0.1f;
	glm::vec4 color = GameRender::COLOR_BLUE;
	for (int i = 0; i < voronoiDebug->cells.size(); i++)
	{
		Voronoi::Cell cell = voronoiDebug->cells[i];
		GameRender::RenderPoint(gameRenderCommands, group, bitmap, color, cell.center.pos, 1);

		for (int j = 0; j < cell.neighbors.size(); j++)
		{
			Voronoi::Vertex neighbor = cell.neighbors[j];
			GameRender::PushLine(gameRenderCommands, group, bitmap, color, cell.center.pos, neighbor.pos, lineThickness);
		}
	}
}

void RenderGrid(EditorState* editorState, GameRender::GameRenderState* gameRenderState, World* world, glm::vec3 groundIntersectionPoint)
{
	RenderSystem::GameRenderCommands* gameRenderCommands = gameRenderState->gameRenderCommands;
	GameAssets* gameAssets = gameRenderState->gameAssets;
	RenderSystem::RenderGroup* group = gameRenderState->renderGroup;
	BitmapId bitmapID = GetFirstBitmapIdFrom(gameAssets, AssetFamilyType::Default);
	LoadedBitmap* bitmap = GetBitmap(gameAssets, bitmapID);

	Editor::GridDisplayConfig config = editorState->gridConfig;

	if (config.showGrid)
	{
		float lineThickness = 0.1;
		for (int xi = 0; xi < world->maxSimPos.x; xi += world->mapGrid.cellSize)
		{
			glm::vec3 pos0 = glm::vec3(xi, 0, 0);
			glm::vec3 pos1 = glm::vec3(xi, world->maxSimPos.y, 0);

			GameRender::RenderLine(
				gameRenderCommands, group, gameAssets, GameRender::HALF_TRANS_COLOR_WHITE, pos0, pos1, lineThickness);
		}

		for (int yi = 0; yi < world->maxSimPos.y; yi += world->mapGrid.cellSize)
		{
			glm::vec3 pos0 = glm::vec3(0, yi, 0);
			glm::vec3 pos1 = glm::vec3(world->maxSimPos.x, yi, 0);

			GameRender::RenderLine(
				gameRenderCommands, group, gameAssets, GameRender::HALF_TRANS_COLOR_WHITE, pos0, pos1, lineThickness);
		}

		if (config.showCellGridCoord)
		{
			for (int yi = 0, y = 0; yi < world->maxSimPos.y; yi += world->mapGrid.cellSize, y++)
			{
				for (int xi = 0, x = 0; xi < world->maxSimPos.x; xi += world->mapGrid.cellSize, x++)
				{
					std::string s = std::to_string(x) + " " + std::to_string(y);

					int xTextPos = xi;
					int yTextPos = yi - world->mapGrid.cellSize / 2;
					GameRender::DEBUGTextLine(s.c_str(), gameRenderState, glm::vec3(xTextPos, yTextPos, 0), 0.2);
				}
			}
		}
		else if (config.showCellSimCoord)
		{
			for (int yi = 0, y = 0; yi < world->maxSimPos.y; yi += world->mapGrid.cellSize, y++)
			{
				for (int xi = 0, x = 0; xi < world->maxSimPos.x; xi += world->mapGrid.cellSize, x++)
				{
					std::string s = std::to_string(xi) + " " + std::to_string(yi);

					int xTextPos = xi;
					int yTextPos = yi - world->mapGrid.cellSize / 2;
					GameRender::DEBUGTextLine(s.c_str(), gameRenderState, glm::vec3(xTextPos, yTextPos, 0), 0.2);
				}
			}
		}

	}

	if (editorState->highlightGrid)
	{
		editorState->hightlightMapCell.Reset();

		if (world->IsValidSimPos(groundIntersectionPoint))
		{
			int cx, cy = 0;
			world->SimPos2GridCoord(groundIntersectionPoint, cx, cy);

			editorState->hightlightMapCell.cellX = cx;
			editorState->hightlightMapCell.cellY = cy;
			editorState->hightlightMapCell.cell = world->mapGrid.GetCellByIndex(cx, cy);

			glm::vec3 min = glm::vec3(cx * world->mapGrid.cellSize, cy * world->mapGrid.cellSize, 0);
			glm::vec3 max = min + glm::vec3(world->mapGrid.cellSize, world->mapGrid.cellSize, 0);

			GameRender::PushCube(gameRenderCommands, group, bitmap, EditorMain::COLOR_HIGHLIGHT_GRID, min, max, false);
		}
	}
}

void RenderWorldBorders(
	GameRender::GameRenderState* gameRenderState,
	World* world)
{
	RenderSystem::GameRenderCommands* gameRenderCommands = gameRenderState->gameRenderCommands;
	GameAssets* gameAssets = gameRenderState->gameAssets;
	RenderSystem::RenderGroup* group = gameRenderState->renderGroup;
	BitmapId bitmapID = GetFirstBitmapIdFrom(gameAssets, AssetFamilyType::Default);
	LoadedBitmap* bitmap = GetBitmap(gameAssets, bitmapID);

	std::vector<glm::vec3> worldBorders;

	worldBorders.push_back(glm::vec3(world->minSimPos.x, world->minSimPos.y, 0));
	worldBorders.push_back(glm::vec3(world->maxSimPos.x, world->minSimPos.y, 0));
	worldBorders.push_back(glm::vec3(world->maxSimPos.x, world->maxSimPos.y, 0));
	worldBorders.push_back(glm::vec3(world->minSimPos.x, world->maxSimPos.y, 0));

	float lineThickness = 0.5;
	for (int i = 0; i < worldBorders.size(); i++)
	{
		GameRender::RenderPoint(gameRenderCommands, group, bitmap, GameRender::COLOR_WHITE, worldBorders[i], lineThickness);

		glm::vec3 pos0 = worldBorders[i];
		glm::vec3 pos1;
		if (i == worldBorders.size() - 1)
		{
			pos1 = worldBorders[0];
		}
		else
		{
			pos1 = worldBorders[i + 1];
		}

		GameRender::RenderLine(
			gameRenderCommands, group, gameAssets, GameRender::COLOR_WHITE, pos0, pos1, lineThickness);
	}
}


void RenderSelectedEntityOption(
	EditorState* editor,
	GameRender::GameRenderState* gameRenderState,
	GameInputState* gameInputState,
	GameState* gameState,
	glm::vec3 groundIntersectionPoint)
{
	if (editor->consumingMouse)
	{
		return;
	}

	if (editor->selectedOption != NULL)
	{

		RenderSystem::GameRenderCommands* gameRenderCommands = gameRenderState->gameRenderCommands;
		GameAssets* gameAssets = gameRenderState->gameAssets;
		RenderSystem::RenderGroup* group = gameRenderState->renderGroup;


		BitmapId bitmapID = GetFirstBitmapIdFrom(gameAssets, AssetFamilyType::Default);
		LoadedBitmap* bitmap = GetBitmap(gameAssets, bitmapID);
		float lineThickness = 0.5;

		EntityOption* option = editor->selectedOption;

		if (option->isAgent)
		{
			BitmapId bitmapID = GetFirstBitmapIdFrom(gameAssets, AssetFamilyType::Default);
			LoadedBitmap* bitmap = GetBitmap(gameAssets, bitmapID);

			GameRender::RenderCircle(
				gameRenderCommands, group, gameAssets, GameRender::SELECTED_ENTITY_COLOR, groundIntersectionPoint, option->agentRadius, 0.5);
		
			if (gameInputState->DidMouseLeftButtonClicked())
			{
				gameState->world.AddAgent(groundIntersectionPoint, option->agentRadius);
			}		
		}
		else
		{
			for (int i = 0; i < option->vertices.size(); i++)
			{
				glm::vec3 pos0 = option->vertices[i];
				glm::vec3 pos1;
				if (i == option->vertices.size() - 1)
				{
					pos1 = option->vertices[0];
				}
				else
				{
					pos1 = option->vertices[i + 1];
				}

				pos0 += groundIntersectionPoint;
				pos1 += groundIntersectionPoint;

				GameRender::RenderLine(
					gameRenderCommands, group, gameAssets, GameRender::SELECTED_ENTITY_COLOR, pos0, pos1, lineThickness);
			}

			if (gameInputState->DidMouseLeftButtonClicked())
			{
				gameState->world.AddObstacle(groundIntersectionPoint, option->vertices);
			}
		}


	}

}

void RenderPathingData(
	EditorState* editor,
	World* world,
	GameRender::GameRenderState* gameRenderState,
	GameInputState* gameInputState,
	PathFinding::DebugState* debugState,
	glm::vec3 groundIntersectionPoint)
{
	RenderSystem::GameRenderCommands* gameRenderCommands = gameRenderState->gameRenderCommands;
	GameAssets* gameAssets = gameRenderState->gameAssets;
	RenderSystem::RenderGroup* group = gameRenderState->renderGroup;
	BitmapId bitmapID = GetFirstBitmapIdFrom(gameAssets, AssetFamilyType::Default);
	LoadedBitmap* bitmap = GetBitmap(gameAssets, bitmapID);

	const float markerSize = 3;

	if (!editor->consumingMouse)
	{
		if (editor->choosingPathingStart)
		{
			glm::vec3 min = glm::vec3(-markerSize, -markerSize, 0) + groundIntersectionPoint;
			glm::vec3 max = glm::vec3(markerSize, markerSize, 0) + groundIntersectionPoint;

			GameRender::PushCube(gameRenderCommands, group, bitmap, GameRender::POTENTIAL_PATHING_START_COLOR, min, max, false);

			if (gameInputState->DidMouseLeftButtonClicked())
			{
				debugState->hasSetStartPos = true;
				debugState->start = groundIntersectionPoint;
			}
		}

		if (editor->choosingPathingEnd)
		{
			glm::vec3 min = glm::vec3(-markerSize, -markerSize, 0) + groundIntersectionPoint;
			glm::vec3 max = glm::vec3(markerSize, markerSize, 0) + groundIntersectionPoint;

			GameRender::PushCube(gameRenderCommands, group, bitmap, GameRender::POTENTIAL_PATHING_END_COLOR, min, max, false);

			if (gameInputState->DidMouseLeftButtonClicked())
			{
				debugState->hasSetEndPos = true;
				debugState->end = groundIntersectionPoint;
			}
		}
	}

	if (debugState->hasSetStartPos)
	{
		glm::vec3 min = glm::vec3(-markerSize, -markerSize, 0) + debugState->start;
		glm::vec3 max = glm::vec3(markerSize, markerSize, 0) + debugState->start;

		GameRender::PushCube(gameRenderCommands, group, bitmap, GameRender::PATHING_START_COLOR, min, max, false);
	}

	if (debugState->hasSetEndPos)
	{
		glm::vec3 min = glm::vec3(-markerSize, -markerSize, 0) + debugState->end;
		glm::vec3 max = glm::vec3(markerSize, markerSize, 0) + debugState->end;

		GameRender::PushCube(gameRenderCommands, group, bitmap, GameRender::PATHING_END_COLOR, min, max, false);
	}


	for (int i = 1; i < debugState->waypoints.size(); i++)
	{
		glm::vec3 p0 = debugState->waypoints[i - 1];
		glm::vec3 p1 = debugState->waypoints[i];

		GameRender::RenderPoint(gameRenderCommands, group, bitmap, GameRender::COLOR_GREEN, p1, 0.5);
		GameRender::PushDashedLine(gameRenderCommands, group, gameAssets, GameRender::COLOR_GREEN, p0, p1, 0.5);
	}
	

	for (int i = 0; i < debugState->portals.size(); i++)
	{
		CDTriangulation::DelaunayTriangleEdge edge = debugState->portals[i];

		CDTriangulation::Vertex v0 = world->cdTriangulationGraph->GetVertexById(edge.vertices[0]);
		CDTriangulation::Vertex v1 = world->cdTriangulationGraph->GetVertexById(edge.vertices[1]);

		GameRender::PushLine(gameRenderCommands, group, gameAssets, GameRender::COLOR_TEAL, v0.pos, v1.pos, 0.5);

	}




	/*
	for (int i = 0; i < world->dualGraph->nodes.size(); i++)
	{
		NavMesh::DualGraphNode* node = &world->dualGraph->nodes[i];

		BitmapId bitmapID = GetFirstBitmapIdFrom(gameAssets, AssetFamilyType::Default);
		LoadedBitmap* bitmap = GetBitmap(gameAssets, bitmapID);

		// vertex
		glm::vec3 offset = glm::vec3(0.5, 0.5, 0.5);
		glm::vec3 min = node->center - offset;
		glm::vec3 max = node->center + offset;
		GameRender::PushCube(gameRenderCommands, &group, bitmap, GameRender::COLOR_GREEN, min, max, true);

		for (int j = 0; j < node->neighbors.size(); j++)
		{
			int neighborId = node->neighbors[j].id;
			NavMesh::DualGraphNode* neighbor = &world->dualGraph->nodes[neighborId];

			// so we dont over draw. We only draw from lowerId to larger Id
			if (neighborId > node->id)
			{
				GameRender::PushDashedLine(gameRenderCommands, &group, bitmap, GameRender::COLOR_GREEN, node->center, neighbor->center, 0.2);
			}
		}
	}
	*/



//	for (int i = 0; i < debugState->portals.size(); i++)
	{
		/*
		NavMesh::Edge edge = world->portals[i];

		if (i == 3)
		{
			GameRender::RenderPoint(gameRenderCommands, &group, gameAssets, GameRender::COLOR_YELLOW, edge.vertices[0], 1);
			GameRender::RenderPoint(gameRenderCommands, &group, gameAssets, GameRender::COLOR_GREEN, edge.vertices[1], 1);
			GameRender::PushLine(gameRenderCommands, &group, gameAssets, GameRender::COLOR_TEAL, edge.vertices[0], edge.vertices[1], 0.5);
		}
		*/


		/*
		if (i == 5)
		{
			NavMesh::Edge edge = world->portals[i];

			GameRender::PushLine(gameRenderCommands, &group, gameAssets, GameRender::COLOR_YELLOW, world->start, edge.vertices[0], 0.2);
			GameRender::RenderPoint(gameRenderCommands, &gr0000000000000000oup, gameAssets, GameRender::COLOR_YELLOW, edge.vertices[0], 1);

			GameRender::PushLine(gameRenderCommands, &group, gameAssets, GameRender::COLOR_TEAL, world->start, edge.vertices[1], 0.2);
			GameRender::RenderPoint(gameRenderCommands, &group, gameAssets, GameRender::COLOR_TEAL, edge.vertices[1], 1);
		}
		*/
	}

}


void RenderCDTriangulationDebug(
	EditorState* editorState,
	GameRender::GameRenderState* gameRenderState,
	CDTriangulation::Graph* triangulationDebug)
{
	RenderSystem::GameRenderCommands* gameRenderCommands = gameRenderState->gameRenderCommands;
	GameAssets* gameAssets = gameRenderState->gameAssets;
	RenderSystem::RenderGroup* group = gameRenderState->renderGroup;


	float lineThickness = 0.5;
	BitmapId bitmapID = GetFirstBitmapIdFrom(gameAssets, AssetFamilyType::Default);
	LoadedBitmap* bitmap = GetBitmap(gameAssets, bitmapID);

	float thickness = 0.2f;
	for (int i = 0; i < triangulationDebug->vertices.size(); i++)
	{
		GameRender::RenderPoint(gameRenderCommands, group, bitmap, GameRender::COLOR_GREEN, triangulationDebug->vertices[i], 1);
	}

	for (int j = 0; j < triangulationDebug->holes.size(); j++)
	{
		GeoCore::Polygon polygon = triangulationDebug->holes[j];
		for (int i = 0; i < polygon.vertices.size(); i++)
		{
			GameRender::RenderPoint(gameRenderCommands, group, bitmap, GameRender::COLOR_RED, polygon.vertices[i], 1);
		}
	}

	/*
	std::vector<glm::vec4> colors = {
		GameRender::COLOR_WHITE, 
		GameRender::COLOR_YELLOW,
		GameRender::COLOR_BLUE,
		GameRender::COLOR_GREEN,
		GameRender::COLOR_RED,
		GameRender::COLOR_TEAL,
		GameRender::COLOR_ORANGE };
		*/

	for (int i = 0; i < triangulationDebug->triangles.size(); i++)
	{
		CDTriangulation::DelaunayTriangle* triangle = &triangulationDebug->triangles[i];
		glm::vec3 liftedVertex[3];


		if (triangle->isObstacle)
		{
			for (int j = 0; j < CDTriangulation::NUM_TRIANGLE_VERTEX; j++)
			{
				// lifting it slightly higher
				liftedVertex[j] = triangle->vertices[j].pos + GameRender::DEBUG_RENDER_OFFSET;
			}

			GameRender::PushTriangleOutline(gameRenderCommands, group, bitmap, GameRender::COLOR_RED, liftedVertex, 2*thickness, false);
		}
		else
		{
			for (int j = 0; j < CDTriangulation::NUM_TRIANGLE_VERTEX; j++)
			{
				// lifting it slightly higher
				liftedVertex[j] = triangle->vertices[j].pos; // +GameRender::DEBUG_RENDER_OFFSET;
			}

			GameRender::PushTriangleOutline(gameRenderCommands, group, bitmap, GameRender::COLOR_WHITE, liftedVertex, thickness, false);
		}

		std::string s = std::to_string(triangle->id);
		GameRender::DEBUGTextLine(s.c_str(), gameRenderState, triangle->GetCenter(), 0.5);


		if (editorState->highlightTriangle)
		{
			if (triangulationDebug->highlightedTriangle != NULL && triangle->id == triangulationDebug->highlightedTriangle->id)
			{
				GameRender::PushTriangle(gameRenderCommands, group, bitmap, GameRender::COLOR_YELLOW, liftedVertex, false);

				Triangulation::Circle circle = CDTriangulation::FindCircumCircle(*triangle);
				GameRender::RenderCircle(
					gameRenderCommands, group, gameAssets, GameRender::COLOR_RED, circle.center, circle.radius, lineThickness);

				float thickness = 3;
				GameRender::RenderPoint(gameRenderCommands, group, gameAssets, GameRender::COLOR_GREEN, liftedVertex[0], thickness);
				GameRender::RenderPoint(gameRenderCommands, group, gameAssets, GameRender::COLOR_BLUE, liftedVertex[1], thickness);
				GameRender::RenderPoint(gameRenderCommands, group, gameAssets, GameRender::COLOR_ORANGE, liftedVertex[2], thickness);
			}
		}
	}


	/*
	for (int i = 0; i < triangulationDebug->intersectingEdges.size(); i++)
	{		
		std::vector<CDTriangulation::Vertex> edge = triangulationDebug->intersectingEdges[i];

		GameRender::RenderLine(
			gameRenderCommands, group, gameAssets, GameRender::COLOR_YELLOW, edge[0].pos, edge[1].pos, lineThickness);
	}
	*/

	/*
	for (int j = 0; j < triangulationDebug->debugAllConstrainedEdges.size(); j++)
	{
		CDTriangulation::DelaunayTriangleEdge edge = triangulationDebug->debugAllConstrainedEdges[j];

		CDTriangulation::Vertex v0 = triangulationDebug->GetVertexById(edge.vertices[0]);
		CDTriangulation::Vertex v1 = triangulationDebug->GetVertexById(edge.vertices[1]);

		GameRender::RenderLine(
			gameRenderCommands, group, gameAssets, GameRender::COLOR_TEAL, v0.pos, v1.pos, 0.70);
	}
	

	for (int j = 0; j < triangulationDebug->constrainedEdges.size(); j++)
	{
		CDTriangulation::DelaunayTriangleEdge edge = triangulationDebug->constrainedEdges[j];

		CDTriangulation::Vertex v0 = triangulationDebug->GetVertexById(edge.vertices[0]);
		CDTriangulation::Vertex v1 = triangulationDebug->GetVertexById(edge.vertices[1]);

		GameRender::RenderLine(
			gameRenderCommands, group, gameAssets, GameRender::COLOR_TEAL, v0.pos, v1.pos, 0.70);
	}
	*/
}


struct PlayerMoveData
{
	glm::vec3 velocity;
};


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

void InteractWithWorldEntities(GameState* gameState, 
	GameInputState* gameInputState, 
	RenderSystem::GameRenderCommands* gameRenderCommands, 
	World* world, 
	glm::vec3 groundIntersectionPoint)
{
	glm::vec3 rayOrigin = gameState->debugCameraEntity.pos;
	glm::vec3 rayDir = MousePosToMousePickingRay(gameState->world.cameraSetup, gameRenderCommands, gameInputState->mousePos);

	EditorState* editorState = &gameState->editorState;

	if (editorState->IsInSelectionMode())
	{
		return;
	}

//	std::cout << "intersectionPoint " << intersectionPoint.x << " " << intersectionPoint.y << " " << intersectionPoint.z << std::endl;

	for (int i = 0; i < world->numEntities; i++)
	{
		Entity* entity = &world->entities[i];
		switch (entity->flag)
		{
			case EntityFlag::OBSTACLE:
	
				std::vector<glm::vec3> absolutePos;

				for (int j = 0; j < entity->vertices.size(); j++)
				{
					absolutePos.push_back(entity->pos + entity->vertices[j]);
				}

				if (Collision::IsPointInsidePolygon2D(groundIntersectionPoint, absolutePos))
				{
				//	std::cout << "entity id " << entity->id << std::endl;

					if (gameInputState->DidMouseLeftButtonClicked())
					{
						if (editorState->draggedEntity == entity)
						{
							editorState->draggedEntity = NULL;
						}
						else
						{
							editorState->draggedEntity = entity;
							editorState->draggedPivot = groundIntersectionPoint - editorState->draggedEntity->pos;
						}

					}					
				}

				break;
		}
	}

	if (editorState->draggedEntity != NULL)
	{
		editorState->draggedEntity->pos = groundIntersectionPoint - editorState->draggedPivot;
	}


}


void WorldTickAndRender(GameState* gameState, TransientState* transientState, GameAssets* gameAssets,
	GameInputState* gameInputState, RenderSystem::GameRenderCommands* gameRenderCommands, glm::ivec2 windowDimensions, DebugModeState* debugModeState)
{
	Entity* controlledEntity = &gameState->debugCameraEntity;

	World* world = &gameState->world;
	
	controlledEntity = &gameState->debugCameraEntity;


	glm::vec3 newViewDir = controlledEntity->GetViewDirection();

	PlayerMoveData pmove = {};

	// process input
	float stepSize = 40.0f;
	{
		if (gameInputState->moveUp.endedDown) {
			pmove.velocity += stepSize * glm::vec3(0, 1, 0);
		}
		if (gameInputState->moveLeft.endedDown) {
			pmove.velocity += stepSize * glm::vec3(-1, 0, 0);
		}
		if (gameInputState->moveRight.endedDown) {
			pmove.velocity += stepSize * glm::vec3(1, 0, 0);
		}
		if (gameInputState->moveDown.endedDown) {
			pmove.velocity += stepSize * glm::vec3(0, -1, 0);
		}
		// make the zoom reverse by distance like what Casey did
		//
		if (gameInputState->zoomIn.endedDown) {
			pmove.velocity += stepSize * newViewDir * 5.0f;
		}
		if (gameInputState->zoomOut.endedDown) {
			pmove.velocity += stepSize * -newViewDir * 5.0f;
		}
	}


	float dt = 0.1f;// FIXED_UPDATE_TIME_S;
	controlledEntity->pos += pmove.velocity * dt;


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
	glm::mat4 cameraProj = glm::perspective(45.0f, windowDimensions.x / (float)windowDimensions.y, 0.5f, 5000.0f);

	world->cameraSetup.proj = cameraProj;
	world->cameraSetup.view = cameraMatrix;
	world->cameraSetup.translation = cameraTransform;


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

	EditorState* editor = &gameState->editorState;

	for (int i = 0; i < world->numEntities; i++)
	{
		Entity* entity = &world->entities[i];
		switch (entity->flag)
		{
			case EntityFlag::OBSTACLE:

				if (!editor->hideObstacles)
				{
					RenderObstacleEntity(gameRenderCommands, &group, gameAssets, entity);
				}
				break;

			case EntityFlag::AGENT:
				RenderAgentEntity(gameRenderCommands, &group, gameAssets, entity);
				break;
		}	
	}





	


	RenderTriangulationDebug(gameRenderCommands, &group, gameAssets, world->triangulationDebug);
	RenderVoronoiDebug(gameRenderCommands, &group, gameAssets, world->voronoiDebug);
	
	

	
	glm::vec3 rayOrigin = gameState->debugCameraEntity.pos;
	glm::vec3 rayDir = MousePosToMousePickingRay(gameState->world.cameraSetup, gameRenderCommands, gameInputState->mousePos);

	glm::vec3 groundIntersectionPoint;
	
	Collision::Plane plane = { world->zAxis, 0 };
	Collision::Ray ray = { rayOrigin, rayDir };
	bool intersects = Collision::RayPlaneIntersection3D(plane, ray, groundIntersectionPoint);

	world->cdTriangulationGraph->highlightedTriangle = NULL;
	if (editor->highlightTriangle)
	{
		glm::vec3 triangleInterspectionPoint;
		for (int i = 0; i < world->cdTriangulationGraph->triangles.size(); i++)
		{
			CDTriangulation::DelaunayTriangle triangle = world->cdTriangulationGraph->triangles[i];

			if (Collision::IsPointInsideTriangle_Barycentric(
				groundIntersectionPoint,
				triangle.vertices[0].pos,
				triangle.vertices[1].pos,
				triangle.vertices[2].pos))
			{
				world->cdTriangulationGraph->highlightedTriangle = &world->cdTriangulationGraph->triangles[i];
				break;
				//			std::cout << "intersecting with " << triangle.id << std::endl;
			}
		}
	}
	
	
	GameRender::GameRenderState gameRenderState = {};
	gameRenderState.gameRenderCommands = gameRenderCommands;
	gameRenderState.renderGroup = &group;
	gameRenderState.debugLoadedFont = transientState->debugLoadedFont;
	gameRenderState.debugFontId = transientState->debugFontId;
	gameRenderState.gameAssets = transientState->assets;


	RenderCDTriangulationDebug(editor, &gameRenderState, world->cdTriangulationGraph);
	RenderPathingData(editor, world, &gameRenderState, gameInputState, world->pathingDebug, groundIntersectionPoint);

	RenderSelectedEntityOption(editor, &gameRenderState, gameInputState, gameState, groundIntersectionPoint);

	// render world borders


	InteractWithWorldEntities(gameState, gameInputState, gameRenderCommands, world, groundIntersectionPoint);


	RenderGrid(editor, &gameRenderState, world, groundIntersectionPoint);
	RenderWorldBorders(&gameRenderState, world);

	GameRender::RenderCoordinateSystem(gameRenderCommands, &group, gameAssets);
}

void TestRealTimeTick(
	GameState* gameState,
	GameInputState* gameInputState,
	RenderSystem::GameRenderCommands* gameRenderCommands,
	glm::vec3 groundIntersectionPoint)
{
	EditorState* editorState = &gameState->editorState;
	World* world = &gameState->world;

	if (editorState->testRealTime)
	{
		if (gameInputState->DidMouseRightButtonClicked())
		{

		}
	}
}


Object SerializeEntity(Entity* entity)
{
	Object entityObj;
	entityObj.push_back(Pair("Id", entity->id));
	entityObj.push_back(Pair("Flag", (int)entity->flag));
	entityObj.push_back(Pair("x", entity->pos.x));
	entityObj.push_back(Pair("y", entity->pos.y));

	if (entity->flag == EntityFlag::OBSTACLE)
	{
		Array verticesArray;
		for (int i = 0; i < entity->vertices.size(); i++)
		{
			glm::vec3 pos = entity->vertices[i];
			Object pointObj;

			pointObj.push_back(Pair("x", pos.x));
			pointObj.push_back(Pair("y", pos.y));
			pointObj.push_back(Pair("z", pos.z));

			verticesArray.push_back(pointObj);
		}
		entityObj.push_back(Pair("vertices", verticesArray));
	}
	else if (entity->flag == EntityFlag::AGENT)
	{
		entityObj.push_back(Pair("agentRadius", entity->agentRadius));
	}


	return entityObj;
}

void DeserializeEntity(Entity* entity, const mObject& obj)
{
	int id = GameIO::FindValue(obj, "Id").get_int();

	EntityFlag entityFlag = (EntityFlag)GameIO::FindValue(obj, "Flag").get_int();

	{
		float x = GameIO::FindValue(obj, "x").get_real();
		float y = GameIO::FindValue(obj, "y").get_real();

		entity->id = id;
		entity->flag = entityFlag;
		entity->pos = glm::vec3(x, y, 0);
	}

	if (entityFlag == EntityFlag::OBSTACLE)
	{
		const mArray& vertices = GameIO::FindValue(obj, "vertices").get_array();

		for (int i = 0; i < vertices.size(); i++)
		{
			const mObject pointObj = vertices[i].get_obj();

			float x = GameIO::FindValue(pointObj, "x").get_real();
			float y = GameIO::FindValue(pointObj, "y").get_real();
			float z = GameIO::FindValue(pointObj, "z").get_real();

			glm::vec3 pos = glm::vec3(x, y, z);

			entity->vertices.push_back(pos);
		}
	}
	else if (entityFlag == EntityFlag::AGENT)
	{
		float agentRadius = GameIO::FindValue(obj, "agentRadius").get_real();
		entity->agentRadius = agentRadius;
	}
}

void LoadMap(World* world, string fileName)
{
	std::cout << "Loading the map " << std::endl;

	world->Init();

	mValue content = GameIO::ReadJsonFileToMap(fileName.c_str());
	const mObject& obj = content.get_obj();

	const mArray& addr_array = GameIO::FindValue(obj, "entities").get_array();

	for (int i = 0; i < addr_array.size(); i++)
	{
		const mObject entityObj = addr_array[i].get_obj();
		
		Entity* entity = &world->entities[i];
		DeserializeEntity(entity, entityObj);
		assert(world->numEntities == entity->id);
		world->numEntities++;
	}

	world->pathingDebug->hasSetStartPos = GameIO::FindValue(obj, "hasPathingStart").get_bool();
	float pathingStartX = GameIO::FindValue(obj, "pathingStartX").get_real();
	float pathingStartY = GameIO::FindValue(obj, "pathingStartY").get_real();
	world->pathingDebug->start = glm::vec3(pathingStartX, pathingStartY, 0);


	world->pathingDebug->hasSetEndPos = GameIO::FindValue(obj, "hasPathingEnd").get_bool();
	float pathingEndX = GameIO::FindValue(obj, "pathingEndX").get_real();
	float pathingEndY = GameIO::FindValue(obj, "pathingEndY").get_real();
	world->pathingDebug->end = glm::vec3(pathingEndX, pathingEndY, 0);
}

void SaveMap(World* world, string savedFileName)
{
	std::cout << "saving the map " << std::endl;
	
	ofstream myfile;
	myfile.open(savedFileName);

	Object worldObj;

	Array entityArray;
	for (int i = 0; i < world->numEntities; i++)
	{
		Entity* entity = &world->entities[i];

		Object entityObj = SerializeEntity(entity);
		entityArray.push_back(entityObj);
	}

	worldObj.push_back(Pair("entities", entityArray));

	worldObj.push_back(Pair("hasPathingStart", world->pathingDebug->hasSetStartPos));
	worldObj.push_back(Pair("pathingStartX", world->pathingDebug->start.x));
	worldObj.push_back(Pair("pathingStartY", world->pathingDebug->start.y));

	worldObj.push_back(Pair("hasPathingEnd", world->pathingDebug->hasSetEndPos));
	worldObj.push_back(Pair("pathingEndX", world->pathingDebug->end.x));
	worldObj.push_back(Pair("pathingEndY", world->pathingDebug->end.y));

	write(worldObj, myfile, pretty_print);
	myfile.close();
	
}

void TriangulateMap(World* world)
{
	

	// https://technology.cpm.org/general/3dgraph/
	// https://oercommons.s3.amazonaws.com/media/courseware/relatedresource/file/imth-6-1-9-6-1-coordinate_plane_plotter/index.html
	// use this online plotter to as online visualization of your points before running the game
	// lines are plotted counter-clockswise so it's consistent with the right hand rule
	std::vector<glm::vec3> vertices;


	float scale = 10;

	vertices.push_back(glm::vec3(world->minSimPos.x, world->minSimPos.y, 0));
	vertices.push_back(glm::vec3(world->maxSimPos.x, world->minSimPos.y, 0));
	vertices.push_back(glm::vec3(world->maxSimPos.x, world->maxSimPos.y, 0));
	vertices.push_back(glm::vec3(world->minSimPos.x, world->maxSimPos.y, 0));

	// clockwise
	std::vector<GeoCore::Polygon> holes;

	for (int i = 0; i < world->numEntities; i++)
	{
		Entity* entity = &world->entities[i];
		if (entity->flag == OBSTACLE)
		{
			GeoCore::Polygon hole;
			for (int j = 0; j < entity->vertices.size(); j++)
			{
				hole.vertices.push_back(entity->vertices[j] + entity->pos);
			}
			holes.push_back(hole);
		}
	}
	
	world->AddWorldBoundsAsHoles(holes);

	CDTriangulation::ConstrainedDelaunayTriangulation(vertices, holes, world->maxSimPos, world->cdTriangulationGraph);
	CDTriangulation::MarkObstacles(world->cdTriangulationGraph, holes);
	world->SpatialPartitionTriangles();
}

void ExecutePathingLogic(World* world, glm::vec3 start, glm::vec3 end)
{
	if (world->cdTriangulationGraph->triangulated)
	{
		world->pathingDebug->dualGraph = new NavMesh::DualGraph(world->cdTriangulationGraph->triangles);
		PathFinding::PathfindingResult pathingResult = PathFinding::FindPath(world->pathingDebug, world, start, end);
		world->pathingDebug->waypoints = pathingResult.waypoints;
	}
	else
	{
		std::cout << "U have triangulated the map" << std::endl;
	}
}

void SamplePathingLogic(World* world)
{
	glm::vec3 start = glm::vec3(20, 48, 0);
	glm::vec3 end = glm::vec3(233, 212, 0);

	ExecutePathingLogic(world, start, end);
}


extern DebugTable* globalDebugTable;

extern "C" __declspec(dllexport) void GameUpdateAndRender(GameMemory * gameMemory, GameInputState * gameInputState, RenderSystem::GameRenderCommands * gameRenderCommands,
	glm::ivec2 windowDimensions, DebugModeState* debugModeState)
{
	//	cout << "Update And Render-2" << endl;

		// so all my game state resides in memory

		// initalize, just init 20 randome entities

		// allocate memroy for entities

	if (gameInputState->moveUp.endedDown)
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
	if (gameInputState->moveDown.endedDown)
	{
		//	cout << "move back" << endl;
	}


	globalDebugTable = gameMemory->debugTable;


	GameState* gameState = (GameState*)gameMemory->permenentStorage;
	if (!gameState->isInitalized)
	{
		// intialize memory arena
		platformAPI = gameMemory->platformAPI;

		int testCase = 2;

		if (testCase == 0)
		{
			gameState->world.Init();
		}
		else if (testCase == 1)
		{
			gameState->world.LoadSampleMap();
			SamplePathingLogic(&gameState->world);
		}
		else if (testCase == 2)
		{
			LoadMap(&gameState->world, "TestData/data2.txt");
		}


		gameState->debugCameraEntity = {};
		gameState->debugCameraEntity.pos = glm::vec3(0, -90, 400);

		gameState->debugCameraEntity.xAxis = glm::normalize(glm::vec3(1.0, 0.0, 0.0));
		gameState->debugCameraEntity.yAxis = glm::normalize(glm::vec3(0.0, 0.96, 0.25));
		gameState->debugCameraEntity.zAxis = glm::normalize(glm::vec3(0.0, -0.25, 0.96));
		

		gameState->debugCameraEntity.min = glm::vec3(-10, -10, -10);
		gameState->debugCameraEntity.max = glm::vec3(10, 10, 10);

		gameState->mouseIsDebugMode = false;


		uint8* base = (uint8*)gameMemory->permenentStorage + sizeof(GameState);
		MemoryIndex size = gameMemory->permenentStorageSize - sizeof(GameState);
		gameState->memoryArena.Init(base, size);


		Editor::InitEditorData(&gameState->editorState);


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

		transientState->debugFontId = { GetFirstAssetIdFrom(transientState->assets, AssetFamilyType::Enum::Font) };
		transientState->debugLoadedFont = GetFont(transientState->assets, transientState->debugFontId);


		transientState->isInitalized = true;
	}

	/*
	for (int i = 0; i < 5000; i++)
	{
		cout << "nice";
	}
	*/


	EditorState* editor = &gameState->editorState;

	while (!editor->coreData->editorEvents.empty())
	{
		EditorEvent editorEvent = editor->coreData->editorEvents.front();
		editor->coreData->editorEvents.pop();
		if (editorEvent == EditorEvent::SAVE)
		{
			SaveMap(&gameState->world, "TestData/Last Data.txt");
		}
		else if (editorEvent == EditorEvent::TRIANGULATE)
		{
			std::cout << "Triangulate" << std::endl;

			SaveMap(&gameState->world, "TestData/Last Data.txt");
			TriangulateMap(&gameState->world);
		}
		else if (editorEvent == EditorEvent::CLEAR_PATHING_START)
		{
			gameState->world.pathingDebug->hasSetStartPos = false;	
		}
		else if (editorEvent == EditorEvent::CLEAR_PATHING_END)
		{
			gameState->world.pathingDebug->hasSetEndPos = false;
		}
		else if (editorEvent == EditorEvent::PATH)
		{
			if (gameState->world.pathingDebug->hasSetStartPos &&
				gameState->world.pathingDebug->hasSetEndPos)
			{
				SaveMap(&gameState->world, "TestData/Last Data.txt");
				ExecutePathingLogic(&gameState->world, gameState->world.pathingDebug->start, gameState->world.pathingDebug->end);
			}
			else
			{
				std::cout << "You havet set the start ane end destination" << std::endl;
			}
		}
		else if (editorEvent == EditorEvent::CLEAR_PATHING_END)
		{
			gameState->world.pathingDebug->hasSetEndPos = false;
		}
	}


	WorldTickAndRender(gameState, transientState, transientState->assets, gameInputState, gameRenderCommands, windowDimensions, debugModeState);







	float halfWidth = gameRenderCommands->settings.dims.x / 2.0f;
	float halfHeight = gameRenderCommands->settings.dims.y / 2.0f;

	glm::mat4 cameraProj = glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight);


	// We start a render setup
	RenderSystem::RenderGroup group = {};

	RenderSystem::RenderSetup renderSetup = {};
	renderSetup.transformMatrix = cameraProj;

	group.quads = PushRenderElement(gameRenderCommands, TexturedQuads);
	*group.quads = {};
	group.quads->masterVertexArrayOffset = gameRenderCommands->numVertex;
	group.quads->masterBitmapArrayOffset = gameRenderCommands->numBitmaps;
	group.quads->renderSetup = renderSetup;

	/*
	if (gameInputState->mouseButtons[(int)PlatformMouseButton_Left].changed)
	{
		std::cout << "changed " << std::endl;
	}
	*/	


	GameRender::GameRenderState gameRenderState = {};
	gameRenderState.gameRenderCommands = gameRenderCommands;
	gameRenderState.renderGroup = &group;
	gameRenderState.debugLoadedFont = transientState->debugLoadedFont;
	gameRenderState.debugFontId = transientState->debugFontId;
	gameRenderState.gameAssets = transientState->assets;
	Editor::TickAndRenderEditorMenu(gameMemory, gameInputState, &gameRenderState, windowDimensions, debugModeState);
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
	float halfHeight = gameRenderCommands->settings.dims.y / 2.0f;

	glm::mat4 cameraProj = glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight);


	// We start a render setup
	RenderSystem::RenderGroup group = {};

	RenderSystem::RenderSetup renderSetup = {};
	renderSetup.transformMatrix = cameraProj;// *glm::inverse(globalDebugCameraMat);



	group.quads = PushRenderElement(gameRenderCommands, TexturedQuads);
	*group.quads = {};
	group.quads->masterVertexArrayOffset = gameRenderCommands->numVertex;
	group.quads->masterBitmapArrayOffset = gameRenderCommands->numBitmaps;
	group.quads->renderSetup = renderSetup;


	GameRender::GameRenderState gameRenderState = {};
	gameRenderState.gameRenderCommands = gameRenderCommands;
	gameRenderState.renderGroup = &group;
	gameRenderState.debugLoadedFont = transientState->debugLoadedFont;
	gameRenderState.debugFontId = transientState->debugFontId;
	gameRenderState.gameAssets = transientState->assets;

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

	// RenderProfileBars(debugState, gameRenderCommands, &group, transientState->assets, gameInputState->mousePos);

	// RenderMiddle(gameRenderCommands, &group, transientState->assets, gameInputState->mousePos);


	glm::vec3 startPos = glm::vec3(-halfWidth, halfHeight - 120, 0.2);
	static char buffer[1024];
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

	size = sprintf(ptr, "viewDir is %f %f %f\n", gameState->debugCameraEntity.GetViewDirection().x, 
		gameState->debugCameraEntity.GetViewDirection().y, 
		gameState->debugCameraEntity.GetViewDirection().z);
	ptr += size;



	glm::vec3 rayDir = MousePosToMousePickingRay(gameState->world.cameraSetup, gameRenderCommands, glm::ivec2(halfWidth, halfHeight));
	size = sprintf(ptr, "rayDir %f %f %f\n", rayDir.x, rayDir.y, rayDir.z);
	ptr += size;

	EditorState* editorState = &gameState->editorState;

	if (editorState->highlightTriangle)
	{
		if (gameState->world.cdTriangulationGraph->highlightedTriangle != NULL)
		{
			CDTriangulation::DelaunayTriangle* trig = gameState->world.cdTriangulationGraph->highlightedTriangle;

			size = sprintf(ptr, "trig %d neighbors %d %d %d\n", trig->id, trig->neighbors[0], trig->neighbors[1], trig->neighbors[2]);
			ptr += size;
		}
	}

	if (editorState->highlightGrid)
	{
		if (!editorState->hightlightMapCell.IsEmpty())
		{
			MapCell* mapCell = editorState->hightlightMapCell.cell;
			
			size = sprintf(ptr, "mapCell %d %d \n", editorState->hightlightMapCell.cellX, editorState->hightlightMapCell.cellY);
			ptr += size;

			editorState->mapCellDebugString.clear();

			for (int i = 0; i < mapCell->triangles.size(); i++)
			{
				editorState->mapCellDebugString += Math::IntToStr(mapCell->triangles[i]) + " ";
			}
		
			if (mapCell->triangles.size() > 0)
			{
				size = sprintf(ptr, "mapCell triangles %s \n", editorState->mapCellDebugString.c_str());
				ptr += size;
			}
			else
			{
				size = sprintf(ptr, "mapCell has no triangles \n");
				ptr += size;
			}
		}
	}


	if (editorState->draggedEntity != NULL)
	{
		size = sprintf(ptr, "dragging entity %d\n", gameState->editorState.draggedEntity->id);
	}
	else
	{
		size = sprintf(ptr, "Not dragging entity\n");
	}
	ptr += size;


	if (gameState->world.pathingDebug->hasSetStartPos)
	{
		glm::vec3 pos = gameState->world.pathingDebug->start;
		size = sprintf(ptr, "start pos: %f %f\n", pos.x, pos.y);
		ptr += size;
	}

	if (gameState->world.pathingDebug->hasSetEndPos)
	{
		glm::vec3 pos = gameState->world.pathingDebug->end;
		size = sprintf(ptr, "end pos: %f %f\n", pos.x, pos.y);
		ptr += size;
	}

	GameRender::DEBUGTextLine(buffer, &gameRenderState, startPos, 1);


	/*
	// render the points 
	World* world = &gameState->world;
	for (int i = 0; i < world->cdTriangulationdebug->vertices.size(); i++)
	{
		glm::vec3 point = 
	}
	*/
}
