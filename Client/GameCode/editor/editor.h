#pragma once
#include "../PlatformShared/platform_shared.h"

#include "../ui_util.h"
#include "../game_render.h"
#include "../game_state.h"
#include "editor_state.h"


namespace Editor
{
	glm::vec4 HIGHLIGHTED = glm::vec4(0, 0.5, 0.25, 0.5);
	glm::vec4 SELECTED = glm::vec4(0.5, 0, 0.5, 0.5);
	glm::vec4 REGULAR = glm::vec4(0, 0, 0.25, 0.25);

	/*
	
  -512,384			     511,384
	  ______________________
	 |						|
	 |						|
	 |			0,0			|
	 |						|
	 |						|
	 |______________________|
  -512,-384			     511,-384
	
	
	*/


	void InitEditorData(EditorState* editorState)
	{
		std::vector<glm::vec3> vertices;

		float scale = 5;

		editorState->options = new EntityOption[10];
		EntityOption option;
		int numOptions = 0;

		// because these are obstacles, they are in counter clockwise order
		//
		option.name = "Square";		
		vertices.clear();
		vertices.push_back(glm::vec3(-1.0, 1.0, 0.0));
		vertices.push_back(glm::vec3(1.0, 1.0, 0.0));
		vertices.push_back(glm::vec3(1.0, -1.0, 0.0));
		vertices.push_back(glm::vec3(-1.0, -1.0, 0.0));
		option.vertices = vertices;
		editorState->options[numOptions++] = option;


		option.name = "Concave";
		vertices.clear();
		vertices.push_back(glm::vec3(1.0, -3.0, 0.0));
		vertices.push_back(glm::vec3(2.0, -9.0, 0.0));
		vertices.push_back(glm::vec3(-2.0, -5.0, 0.0));
		vertices.push_back(glm::vec3(-5.0, 0.0, 0.0));
		vertices.push_back(glm::vec3(0.0, 6.0, 0.0));
		vertices.push_back(glm::vec3(2.0, 4.0, 0.0));
		vertices.push_back(glm::vec3(4.0, 0.0, 0.0));
		option.vertices = vertices;
		editorState->options[numOptions++] = option;


		option.name = "4 point star";
		vertices.clear();
		vertices.push_back(glm::vec3(1.0, -1.0, 0.0));
		vertices.push_back(glm::vec3(0.0, -5.0, 0.0));
		vertices.push_back(glm::vec3(-1.0, -1.0, 0.0));
		vertices.push_back(glm::vec3(-5.0, 0.0, 0.0));
		vertices.push_back(glm::vec3(-1.0, 1.0, 0.0));
		vertices.push_back(glm::vec3(0.0, 5.0, 0.0));
		vertices.push_back(glm::vec3(1.0, 1.0, 0.0));
		vertices.push_back(glm::vec3(5.0, 0.0, 0.0));
		option.vertices = vertices;
		editorState->options[numOptions++] = option;


		option.name = "Hexagon";
		vertices.clear();
		vertices.push_back(glm::vec3(0.0, -4.0, 0.0));
		vertices.push_back(glm::vec3(-8.0, 0.0, 0.0));
		vertices.push_back(glm::vec3(-7.0, 4.0, 0.0));
		vertices.push_back(glm::vec3(-4.0, 6.0, 0.0));
		vertices.push_back(glm::vec3(3.0, 4.0, 0.0));
		vertices.push_back(glm::vec3(5.0, 0.0, 0.0));
		option.vertices = vertices;
		editorState->options[numOptions++] = option;

		
		option.name = "Mineral";
		vertices.clear();
		vertices.push_back(glm::vec3(-1.0, 0.5, 0.0));
		vertices.push_back(glm::vec3(1.0, 0.5, 0.0));
		vertices.push_back(glm::vec3(1.0, -0.5, 0.0));
		vertices.push_back(glm::vec3(-1.0, -0.5, 0.0));
		option.vertices = vertices;
		editorState->options[numOptions++] = option;
		

		option.name = "long wall";
		vertices.clear();
		vertices.push_back(glm::vec3(-10.0, 1.0, 0.0));
		vertices.push_back(glm::vec3(10.0, 1.0, 0.0));
		vertices.push_back(glm::vec3(10.0, -1.0, 0.0));
		vertices.push_back(glm::vec3(-10.0, -1.0, 0.0));
		option.vertices = vertices;
		editorState->options[numOptions++] = option;


		option.name = "tall wall";
		vertices.clear();
		vertices.push_back(glm::vec3(-1.0, 10.0, 0.0));
		vertices.push_back(glm::vec3(1.0, 10.0, 0.0));
		vertices.push_back(glm::vec3(1.0, -10.0, 0.0));
		vertices.push_back(glm::vec3(-1.0, -10.0, 0.0));
		option.vertices = vertices;
		editorState->options[numOptions++] = option;

		editorState->numOptions = numOptions;

		for (int i = 0; i < editorState->numOptions; i++)
		{
			for (int j = 0; j < editorState->options[i].vertices.size(); j++)
			{
				editorState->options[i].vertices[j] *= scale;
			}
		}


		// do a special initalization, becuz im doing memset(0), so lots of inialization is messedup
		editorState->coreData = new EditorStateData();
	}


	void RenderSpecialButton(EditorState* editorState,
		GameInputState* gameInputState,
		GameRender::GameRenderState* gameRenderState,
		glm::vec3 screenMousePos,
		int x, int y,
		int width, int height,
		std::string s, EditorEvent editorEventToPublish)
	{
		RenderSystem::GameRenderCommands* gameRenderCommands = gameRenderState->gameRenderCommands;
		RenderSystem::RenderGroup* renderGroup = gameRenderState->renderGroup;
		GameAssets* gameAssets = gameRenderState->gameAssets;

		BitmapId bitmapID = GetFirstBitmapIdFrom(gameAssets, AssetFamilyType::Default);
		LoadedBitmap* defaultBitmap = GetBitmap(gameAssets, bitmapID);

		glm::vec3 profileRectMin = glm::vec3(x, y, 0);
		glm::vec3 profileRectMax = glm::vec3(x + width, y + height, 0);
		glm::vec3 halfDim = (profileRectMax - profileRectMin) / 2.0f;

		bool isHighlighted = false;
		bool isPressed = false;
		if (Collision::IsPointInsideRect({ profileRectMin, profileRectMax }, screenMousePos))
		{
			editorState->consumingMouse = true;
			isHighlighted = true;
			
			if (gameInputState->mouseButtons[(int)PlatformMouseButton_Left].endedDown)
			{
				isPressed = true;
			}

			if (gameInputState->DidMouseLeftButtonClicked())
			{
				editorState->coreData->editorEvents.push(editorEventToPublish);
			}			
		}

		if (isPressed)
		{
			GameRender::PushBitmap(gameRenderCommands, renderGroup, defaultBitmap, SELECTED, profileRectMin,
				halfDim, GameRender::AlignmentMode::Left, GameRender::AlignmentMode::Bottom);
		}
		else if (isHighlighted)
		{
			GameRender::PushBitmap(gameRenderCommands, renderGroup, defaultBitmap, HIGHLIGHTED, profileRectMin,
				halfDim, GameRender::AlignmentMode::Left, GameRender::AlignmentMode::Bottom);
		}
		else
		{
			GameRender::PushBitmap(gameRenderCommands, renderGroup, defaultBitmap, REGULAR, profileRectMin,
				halfDim, GameRender::AlignmentMode::Left, GameRender::AlignmentMode::Bottom);
		}

		glm::vec3 pos = profileRectMin;
		GameRender::DEBUGTextLine(s.c_str(), gameRenderState, pos, 1);
	}



	void RenderToggle(EditorState* editorState,
		GameInputState* gameInputState,
		GameRender::GameRenderState* gameRenderState,
		glm::vec3 screenMousePos,
		int x, int y,
		int width, int height,
		std::string s, EditorEvent editorEventToPublish, bool currentState)
	{
		RenderSystem::GameRenderCommands* gameRenderCommands = gameRenderState->gameRenderCommands;
		RenderSystem::RenderGroup* renderGroup = gameRenderState->renderGroup;
		GameAssets* gameAssets = gameRenderState->gameAssets;

		BitmapId bitmapID = GetFirstBitmapIdFrom(gameAssets, AssetFamilyType::Default);
		LoadedBitmap* defaultBitmap = GetBitmap(gameAssets, bitmapID);

		glm::vec3 profileRectMin = glm::vec3(x, y, 0);
		glm::vec3 profileRectMax = glm::vec3(x + width, y + height, 0);
		glm::vec3 halfDim = (profileRectMax - profileRectMin) / 2.0f;

		bool isHighlighted = false;
		bool isPressed = false;
		if (Collision::IsPointInsideRect({ profileRectMin, profileRectMax }, screenMousePos))
		{
			editorState->consumingMouse = true;
			isHighlighted = true;

			if (gameInputState->mouseButtons[(int)PlatformMouseButton_Left].endedDown)
			{
				isPressed = true;
			}

			if (gameInputState->DidMouseLeftButtonClicked())
			{
				editorState->coreData->editorEvents.push(editorEventToPublish);
			}
		}

		if (isPressed || currentState)
		{
			GameRender::PushBitmap(gameRenderCommands, renderGroup, defaultBitmap, SELECTED, profileRectMin,
				halfDim, GameRender::AlignmentMode::Left, GameRender::AlignmentMode::Bottom);
		}
		else if (isHighlighted)
		{
			GameRender::PushBitmap(gameRenderCommands, renderGroup, defaultBitmap, HIGHLIGHTED, profileRectMin,
				halfDim, GameRender::AlignmentMode::Left, GameRender::AlignmentMode::Bottom);
		}
		else
		{
			GameRender::PushBitmap(gameRenderCommands, renderGroup, defaultBitmap, REGULAR, profileRectMin,
				halfDim, GameRender::AlignmentMode::Left, GameRender::AlignmentMode::Bottom);
		}

		glm::vec3 pos = profileRectMin;
		GameRender::DEBUGTextLine(s.c_str(), gameRenderState, pos, 1);
	}


	void RenderChoice(
		EditorState* editorState,
		GameInputState* gameInputState,
		GameRender::GameRenderState* gameRenderState,
		glm::vec3 screenMousePos,
		EntityOption* entityOption, int x, int y, int width, int height)
	{
		RenderSystem::GameRenderCommands* gameRenderCommands = gameRenderState->gameRenderCommands;
		RenderSystem::RenderGroup* renderGroup = gameRenderState->renderGroup;
		GameAssets* gameAssets = gameRenderState->gameAssets;

		BitmapId bitmapID = GetFirstBitmapIdFrom(gameAssets, AssetFamilyType::Default);
		LoadedBitmap* defaultBitmap = GetBitmap(gameAssets, bitmapID);

		glm::vec3 profileRectMin = glm::vec3(x, y, 0);
		glm::vec3 profileRectMax = glm::vec3(x + width, y + height, 0);
		glm::vec3 halfDim = (profileRectMax - profileRectMin) / 2.0f;

		if (Collision::IsPointInsideRect({ profileRectMin, profileRectMax }, screenMousePos))
		{
			editorState->consumingMouse = true;
			if (gameInputState->mouseButtons[(int)PlatformMouseButton_Left].endedDown && 
				gameInputState->mouseButtons[(int)PlatformMouseButton_Left].changed)
			{
				if (editorState->selected != NULL && entityOption == editorState->selected)
				{
					editorState->selected = NULL;
				}
				else
				{
					editorState->selected = entityOption;
				}
			}
			else
			{
				editorState->highlighted = entityOption;
			}			
		}
		
		if (entityOption == editorState->selected)
		{
			GameRender::PushBitmap(gameRenderCommands, renderGroup, defaultBitmap, SELECTED, profileRectMin,
				halfDim, GameRender::AlignmentMode::Left, GameRender::AlignmentMode::Bottom);
		}
		else if(entityOption == editorState->highlighted)
		{
			GameRender::PushBitmap(gameRenderCommands, renderGroup, defaultBitmap, HIGHLIGHTED, profileRectMin,
				halfDim, GameRender::AlignmentMode::Left, GameRender::AlignmentMode::Bottom);
		}
		else
		{
			GameRender::PushBitmap(gameRenderCommands, renderGroup, defaultBitmap, REGULAR, profileRectMin,
				halfDim, GameRender::AlignmentMode::Left, GameRender::AlignmentMode::Bottom);
		}


		glm::vec3 pos = profileRectMin;
		GameRender::DEBUGTextLine(entityOption->name.c_str(), gameRenderState, pos, 1);
	}

	void IncrementButtonIndex(int& indexX, int& indexY, int numCol)
	{
		indexX++;
		if (indexX == numCol)
		{
			indexY++;
			indexX = 0;
		}
	}

	void TickAndRenderEditorMenu(GameMemory* gameMemory,
		GameInputState* gameInputState,
		GameRender::GameRenderState* gameRenderSetup,
		glm::ivec2 windowDimensions, DebugModeState* debugModeState)
	{
		GameAssets* gameAssets = gameRenderSetup->gameAssets;
		RenderSystem::RenderGroup* group = gameRenderSetup->renderGroup;
		RenderSystem::GameRenderCommands* gameRenderCommands = gameRenderSetup->gameRenderCommands;

		GameState* gameState = (GameState*)gameMemory->permenentStorage;
		TransientState* transientState = (TransientState*)gameMemory->transientStorage;

		EditorState* editor = &gameState->editorState;

		float halfWidth = gameRenderCommands->settings.dims.x / 2.0f;
		float halfHeight = gameRenderCommands->settings.dims.y / 2.0f;
		

		glm::vec3 screenMousePos = UIUtil::PlatformMouseToScreenRenderPos(gameRenderCommands, gameInputState->mousePos);



		int numCol = 2;

		int btnWidth = 200;
		int btnHeight = 20;

		int startX = halfWidth - numCol * btnWidth;
		int startY = halfHeight;

		int curX = startX;
		int curY = startY;

		editor->highlighted = NULL;

		int entityOptionIndex = 0;
		editor->consumingMouse = false;


		int indexX = 0, indexY = 0;
		for (int i = 0; i < editor->numOptions; i++)
		{
			curX = startX + btnWidth * indexX;
			curY = startY - btnHeight * (indexY + 1);

			EntityOption* entityOption = &editor->options[i];
			RenderChoice(editor, gameInputState, gameRenderSetup, screenMousePos, entityOption, curX, curY, btnWidth, btnHeight);

			IncrementButtonIndex(indexX, indexY, numCol);
		}

		curX = startX + btnWidth * indexX;
		curY = startY - btnHeight * (indexY + 1);
		RenderSpecialButton(editor, gameInputState, gameRenderSetup, screenMousePos, 
			curX, curY, btnWidth, btnHeight, "Save", EditorEvent::SAVE);
		IncrementButtonIndex(indexX, indexY, numCol);

		curX = startX + btnWidth * indexX;
		curY = startY - btnHeight * (indexY + 1);
		RenderSpecialButton(editor, gameInputState, gameRenderSetup, screenMousePos, 
			curX, curY, btnWidth, btnHeight, "Triangulate", EditorEvent::TRIANGULATE);
		IncrementButtonIndex(indexX, indexY, numCol);


		curX = startX + btnWidth * indexX;
		curY = startY - btnHeight * (indexY + 1);
		RenderToggle(editor, gameInputState, gameRenderSetup, screenMousePos,
			curX, curY, btnWidth, btnHeight, "Show Grid", EditorEvent::SHOW_GRID, editor->gridConfig.showGrid);
		IncrementButtonIndex(indexX, indexY, numCol);

		if (editor->gridConfig.showGrid)
		{
			curX = startX + btnWidth * indexX;
			curY = startY - btnHeight * (indexY + 1);
			RenderToggle(editor, gameInputState, gameRenderSetup, screenMousePos,
				curX, curY, btnWidth, btnHeight, "Show Grid Coord", EditorEvent::SHOW_GRID_COORD, editor->gridConfig.showCellGridCoord);
			IncrementButtonIndex(indexX, indexY, numCol);

			curX = startX + btnWidth * indexX;
			curY = startY - btnHeight * (indexY + 1);
			RenderToggle(editor, gameInputState, gameRenderSetup, screenMousePos,
				curX, curY, btnWidth, btnHeight, "Show Grid Sim Pos", EditorEvent::SHOW_GRID_SIM_COORD, editor->gridConfig.showCellSimCoord);
			IncrementButtonIndex(indexX, indexY, numCol);
		}

		curX = startX + btnWidth * indexX;
		curY = startY - btnHeight * (indexY + 1);
		RenderToggle(editor, gameInputState, gameRenderSetup, screenMousePos,
			curX, curY, btnWidth, btnHeight, "Debug Triangle", EditorEvent::DEBUG_TRIANGLE, editor->highlightTriangle);
		IncrementButtonIndex(indexX, indexY, numCol);


		curX = startX + btnWidth * indexX;
		curY = startY - btnHeight * (indexY + 1);
		RenderToggle(editor, gameInputState, gameRenderSetup, screenMousePos,
			curX, curY, btnWidth, btnHeight, "Hide Obstacles", EditorEvent::HIDE_OBSTACLES, editor->hideObstacles);
		IncrementButtonIndex(indexX, indexY, numCol);



		curX = startX + btnWidth * indexX;
		curY = startY - btnHeight * (indexY + 1);
		RenderToggle(editor, gameInputState, gameRenderSetup, screenMousePos,
			curX, curY, btnWidth, btnHeight, "Debug Grid", EditorEvent::DEBUG_GRID, editor->highlightGrid);
		IncrementButtonIndex(indexX, indexY, numCol);
		

		int numSpecialButtons = indexY * numCol + indexX;
		int numRow = (numSpecialButtons + (numCol - 1)) / numCol;


		float thickness = 0.5;
		curX = startX;
		curY = startY;
		float lineHeight = numRow * btnHeight;

		for (int x = 0; x < numCol; x++)
		{
			curX = startX + btnWidth * x;

			glm::vec3 pos = glm::vec3(curX, curY, 0);
			glm::vec3 end = pos;
			end.y -= lineHeight;

			GameRender::RenderLine(gameRenderCommands, group, gameAssets, GameRender::COLOR_BLUE, pos, end, thickness);
		}

		
		curX = startX;
		curY = startY;
		float lineWidth = numCol * btnWidth;
		for (int y = 0; y < numRow; y++)
		{
			curY = startY - btnHeight * y;

			glm::vec3 pos = glm::vec3(curX, curY, 0);
			glm::vec3 end = pos;
			end.x += lineWidth;

			GameRender::RenderLine(gameRenderCommands, group, gameAssets, GameRender::COLOR_BLUE, pos, end, thickness);
		}

	}






};