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

	const int BTN_WIDTH = 200;
	const int BTN_HEIGHT = 20;
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

		option.clear();
		option.name = "Agent 2";
		option.isAgent = true;
		option.agentRadius = 2;
		editorState->options[numOptions++] = option;

		option.clear();
		option.name = "Agent 5";
		option.isAgent = true;
		option.agentRadius = 5;
		editorState->options[numOptions++] = option;

		option.clear();
		option.name = "Agent 10";
		option.isAgent = true;
		option.agentRadius = 10;
		editorState->options[numOptions++] = option;


		// because these are obstacles, they are in counter clockwise order
		//
		option.clear();
		option.name = "Square";		
		vertices.clear();
		vertices.push_back(glm::vec3(-1.0, 1.0, 0.0));
		vertices.push_back(glm::vec3(1.0, 1.0, 0.0));
		vertices.push_back(glm::vec3(1.0, -1.0, 0.0));
		vertices.push_back(glm::vec3(-1.0, -1.0, 0.0));
		option.vertices = vertices;
		editorState->options[numOptions++] = option;

		option.clear();
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


		option.clear();
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


		option.clear();
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


		option.clear();
		option.name = "Mineral";
		vertices.clear();
		vertices.push_back(glm::vec3(-1.0, 0.5, 0.0));
		vertices.push_back(glm::vec3(1.0, 0.5, 0.0));
		vertices.push_back(glm::vec3(1.0, -0.5, 0.0));
		vertices.push_back(glm::vec3(-1.0, -0.5, 0.0));
		option.vertices = vertices;
		editorState->options[numOptions++] = option;
		

		option.clear();
		option.name = "long wall";
		vertices.clear();
		vertices.push_back(glm::vec3(-10.0, 1.0, 0.0));
		vertices.push_back(glm::vec3(10.0, 1.0, 0.0));
		vertices.push_back(glm::vec3(10.0, -1.0, 0.0));
		vertices.push_back(glm::vec3(-10.0, -1.0, 0.0));
		option.vertices = vertices;
		editorState->options[numOptions++] = option;


		option.clear();
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


	bool RenderSpecialButton(EditorState* editorState,
		GameInputState* gameInputState,
		GameRender::GameRenderState* gameRenderState,
		glm::vec3 screenMousePos,
		int x, int y,
		int width, int height,
		std::string s)
	{
		bool trigger = false;
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

			if (gameInputState->DidMouseLeftButtonClickedUp())
			{
				trigger = true;
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
		return trigger;
	}



	bool RenderToggle(EditorState* editorState,
		GameInputState* gameInputState,
		GameRender::GameRenderState* gameRenderState,
		glm::vec3 screenMousePos,
		int x, int y,
		int width, int height,
		std::string s, bool currentState)
	{
		bool trigger = false;
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

			if (gameInputState->DidMouseLeftButtonClickedUp())
			{
				trigger = true;
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
		return trigger;
	}



	bool RenderChoice(
		EditorState* editorState,
		GameInputState* gameInputState,
		GameRender::GameRenderState* gameRenderState,
		glm::vec3 screenMousePos,
		EntityOption* entityOption, int x, int y, int width, int height)
	{
		bool trigger = false;
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
			if (gameInputState->DidMouseLeftButtonClickedUp())
			{
				if (editorState->selectedOption != NULL && entityOption == editorState->selectedOption)
				{
					editorState->selectedOption = NULL;
				}
				else
				{
					editorState->selectedOption = entityOption;
				}
				trigger = true;
			}
			else
			{
				editorState->highlightedOption = entityOption;
			}			
		}
		
		if (entityOption == editorState->selectedOption)
		{
			GameRender::PushBitmap(gameRenderCommands, renderGroup, defaultBitmap, SELECTED, profileRectMin,
				halfDim, GameRender::AlignmentMode::Left, GameRender::AlignmentMode::Bottom);
		}
		else if(entityOption == editorState->highlightedOption)
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
		return trigger;
	}



	struct GridLayoutHelper
	{
		int startX;
		int startY;
		int indexX;
		int indexY;
		int btnWidth;
		int btnHeight;
		int numCol;

		void GetElementPosition(int& curX, int& curY)
		{
			curX = startX + btnWidth * indexX;
			curY = startY - btnHeight * (indexY + 1);
		}

		void IncrementElementCount()
		{
			indexX++;
			if (indexX == numCol)
			{
				indexY++;
				indexX = 0;
			}
		}

		int GetNumElements()
		{
			return indexY * numCol + indexX;
		}

		int GetNumRow()
		{
			return indexY + (indexX != 0);
		} 
	};

	void RenderEntityOptionsPanel(
		GameInputState* gameInputState,
		GameRender::GameRenderState* gameRenderSetup,
		glm::vec3 screenMousePos,
		EditorState* editor)
	{
		RenderSystem::GameRenderCommands* gameRenderCommands = gameRenderSetup->gameRenderCommands;
		RenderSystem::RenderGroup* group = gameRenderSetup->renderGroup;
		GameAssets* gameAssets = gameRenderSetup->gameAssets;

		editor->highlightedOption = NULL;

		int btnWidth = 150;
		int btnHeight = 20;

		float halfWidth = gameRenderCommands->settings.dims.x / 2.0f;
		float halfHeight = gameRenderCommands->settings.dims.y / 2.0f;


		int startX = -halfWidth;
		int startY = halfHeight;
		int indexX = 0, indexY = 0;
		GridLayoutHelper gridLayoutHelper =
		{
			startX, startY,
			indexX,	indexY,
			btnWidth, btnHeight,
			2
		};

		int curX = startX;
		int curY = startY;

		for (int i = 0; i < editor->numOptions; i++)
		{
			gridLayoutHelper.GetElementPosition(curX, curY);

			EntityOption* entityOption = &editor->options[i];
			if (RenderChoice(editor, gameInputState, gameRenderSetup, screenMousePos, entityOption, curX, curY, btnWidth, btnHeight))
			{
				editor->isEditingEntities = false;
			}

			gridLayoutHelper.IncrementElementCount();
		}


		float thickness = 0.5;
		curX = startX;
		curY = startY;
		float lineHeight = gridLayoutHelper.GetNumRow() * btnHeight;

		for (int x = 0; x <= gridLayoutHelper.numCol; x++)
		{
			curX = startX + btnWidth * x;

			glm::vec3 pos = glm::vec3(curX, curY, 0);
			glm::vec3 end = pos;
			end.y -= lineHeight;

			GameRender::RenderLine(gameRenderCommands, group, gameAssets, GameRender::COLOR_BLUE, pos, end, thickness);
		}


		curX = startX;
		curY = startY;
		float lineWidth = gridLayoutHelper.numCol * btnWidth;
		for (int y = 0; y <= gridLayoutHelper.GetNumRow(); y++)
		{
			curY = startY - BTN_HEIGHT * y;

			glm::vec3 pos = glm::vec3(curX, curY, 0);
			glm::vec3 end = pos;
			end.x += lineWidth;

			GameRender::RenderLine(gameRenderCommands, group, gameAssets, GameRender::COLOR_BLUE, pos, end, thickness);
		}
	}


	void RenderPathingDebugTab(
		GameInputState* gameInputState,
		GameRender::GameRenderState* gameRenderSetup,
		glm::vec3 screenMousePos,
		EditorState* editor)
	{
		RenderSystem::GameRenderCommands* gameRenderCommands = gameRenderSetup->gameRenderCommands;
		RenderSystem::RenderGroup* group = gameRenderSetup->renderGroup;
		GameAssets* gameAssets = gameRenderSetup->gameAssets;

		editor->highlightedOption = NULL;

		int btnWidth = 200;
		int btnHeight = 20;

		float halfWidth = gameRenderCommands->settings.dims.x / 2.0f;
		float halfHeight = gameRenderCommands->settings.dims.y / 2.0f;


		int startX = -halfWidth;
		int startY = 0;
		int indexX = 0, indexY = 0;
		GridLayoutHelper gridLayoutHelper =
		{
			startX, startY,
			indexX,	indexY,
			btnWidth, btnHeight,
			2
		};

		int curX = startX;
		int curY = startY;


		gridLayoutHelper.GetElementPosition(curX, curY);
		if (RenderToggle(editor, gameInputState, gameRenderSetup, screenMousePos,
			curX, curY, btnWidth, btnHeight, "Show AStar Waypoints", editor->pathingDebugConfig.showAStarWaypoints))
		{
			editor->pathingDebugConfig.showAStarWaypoints = !editor->pathingDebugConfig.showAStarWaypoints;
		}
		gridLayoutHelper.IncrementElementCount();


		gridLayoutHelper.GetElementPosition(curX, curY);
		if (RenderToggle(editor, gameInputState, gameRenderSetup, screenMousePos,
			curX, curY, btnWidth, btnHeight, "Show Tunnel", editor->pathingDebugConfig.showTunnel))
		{
			editor->pathingDebugConfig.showTunnel = !editor->pathingDebugConfig.showTunnel;
		}
		gridLayoutHelper.IncrementElementCount();


		gridLayoutHelper.GetElementPosition(curX, curY);
		if (RenderToggle(editor, gameInputState, gameRenderSetup, screenMousePos,
			curX, curY, btnWidth, btnHeight, "Show Portals", editor->pathingDebugConfig.showPortals))
		{
			editor->pathingDebugConfig.showPortals = !editor->pathingDebugConfig.showPortals;
		}
		gridLayoutHelper.IncrementElementCount();


		gridLayoutHelper.GetElementPosition(curX, curY);
		if (RenderToggle(editor, gameInputState, gameRenderSetup, screenMousePos,
			curX, curY, btnWidth, btnHeight, "show Modified Tunnel", editor->pathingDebugConfig.showModifiedTunnel))
		{
			editor->pathingDebugConfig.showModifiedTunnel = !editor->pathingDebugConfig.showModifiedTunnel;
		}
		gridLayoutHelper.IncrementElementCount();

		gridLayoutHelper.GetElementPosition(curX, curY);
		if (RenderToggle(editor, gameInputState, gameRenderSetup, screenMousePos,
			curX, curY, btnWidth, btnHeight, "Show Modified Portals", editor->pathingDebugConfig.showModifiedPortals))
		{
			editor->pathingDebugConfig.showModifiedPortals = !editor->pathingDebugConfig.showModifiedPortals;
		}
		gridLayoutHelper.IncrementElementCount();

		gridLayoutHelper.GetElementPosition(curX, curY);
		if (RenderToggle(editor, gameInputState, gameRenderSetup, screenMousePos,
			curX, curY, btnWidth, btnHeight, "Show ANE Perps", editor->pathingDebugConfig.showAnePerps))
		{
			editor->pathingDebugConfig.showAnePerps = !editor->pathingDebugConfig.showAnePerps;
		}
		gridLayoutHelper.IncrementElementCount();



		float thickness = 0.5;
		curX = startX;
		curY = startY;
		float lineHeight = gridLayoutHelper.GetNumRow() * btnHeight;

		for (int x = 0; x <= gridLayoutHelper.numCol; x++)
		{
			curX = startX + btnWidth * x;

			glm::vec3 pos = glm::vec3(curX, curY, 0);
			glm::vec3 end = pos;
			end.y -= lineHeight;

			GameRender::RenderLine(gameRenderCommands, group, gameAssets, GameRender::COLOR_BLUE, pos, end, thickness);
		}

		curX = startX;
		curY = startY;
		float lineWidth = gridLayoutHelper.numCol * btnWidth;
		for (int y = 0; y <= gridLayoutHelper.GetNumRow(); y++)
		{
			curY = startY - BTN_HEIGHT * y;

			glm::vec3 pos = glm::vec3(curX, curY, 0);
			glm::vec3 end = pos;
			end.x += lineWidth;

			GameRender::RenderLine(gameRenderCommands, group, gameAssets, GameRender::COLOR_BLUE, pos, end, thickness);
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

		editor->consumingMouse = false;

		RenderEntityOptionsPanel(gameInputState, gameRenderSetup, screenMousePos, editor);



		int numCol = 2;
		int btnWidth = 200;
		int btnHeight = 20;
		int indexX = 0, indexY = 0;
		int startX = halfWidth - numCol * btnWidth;
		int startY = halfHeight;
		GridLayoutHelper gridLayoutHelper =
		{
			startX,	startY,
			indexX, indexY,
			btnWidth, btnHeight,
			numCol
		};

		int curX = startX;
		int curY = startY;

		gridLayoutHelper.GetElementPosition(curX, curY);
		if (RenderSpecialButton(editor, gameInputState, gameRenderSetup, screenMousePos,
			curX, curY, btnWidth, btnHeight, "Save"))
		{
			editor->coreData->editorEvents.push(EditorEvent::SAVE);
		}
		gridLayoutHelper.IncrementElementCount();

		
		gridLayoutHelper.GetElementPosition(curX, curY);
		if (RenderToggle(editor, gameInputState, gameRenderSetup, screenMousePos,
			curX, curY, btnWidth, btnHeight, "Edit Entities", editor->isEditingEntities))
		{
			editor->isEditingEntities = !editor->isEditingEntities;
		}
		gridLayoutHelper.IncrementElementCount();


		gridLayoutHelper.GetElementPosition(curX, curY);
		if (RenderSpecialButton(editor, gameInputState, gameRenderSetup, screenMousePos,
			curX, curY, btnWidth, btnHeight, "Triangulate"))
		{
			editor->coreData->editorEvents.push(EditorEvent::TRIANGULATE);
		}
		gridLayoutHelper.IncrementElementCount();


		gridLayoutHelper.GetElementPosition(curX, curY);
		if (RenderToggle(editor, gameInputState, gameRenderSetup, screenMousePos,
			curX, curY, btnWidth, btnHeight, "Show Half Width Lines", editor->triangulationDebugConfig.showHalfWidthLines))
		{
			editor->triangulationDebugConfig.showHalfWidthLines = !editor->triangulationDebugConfig.showHalfWidthLines;
		}
		gridLayoutHelper.IncrementElementCount();


		gridLayoutHelper.GetElementPosition(curX, curY);
		if (RenderToggle(editor, gameInputState, gameRenderSetup, screenMousePos,
			curX, curY, btnWidth, btnHeight, "Show Triangles", editor->triangulationDebugConfig.showTriangles))
		{
			editor->triangulationDebugConfig.showTriangles = !editor->triangulationDebugConfig.showTriangles;
		}
		gridLayoutHelper.IncrementElementCount();

		

		gridLayoutHelper.GetElementPosition(curX, curY);
		if (RenderToggle(editor, gameInputState, gameRenderSetup, screenMousePos,
			curX, curY, btnWidth, btnHeight, "Debug Camera", editor->debugConfig.debugCamera))
		{
			editor->debugConfig.debugCamera = !editor->debugConfig.debugCamera;
		}
		gridLayoutHelper.IncrementElementCount();


		gridLayoutHelper.GetElementPosition(curX, curY);
		if (RenderToggle(editor, gameInputState, gameRenderSetup, screenMousePos,
			curX, curY, btnWidth, btnHeight, "Debug Cursor Point", editor->debugConfig.debugCursorSimPos))
		{
			editor->debugConfig.debugCursorSimPos = !editor->debugConfig.debugCursorSimPos;
		}
		gridLayoutHelper.IncrementElementCount();




		gridLayoutHelper.GetElementPosition(curX, curY);
		if (RenderToggle(editor, gameInputState, gameRenderSetup, screenMousePos,
			curX, curY, btnWidth, btnHeight, "Debug Cursor Point", editor->debugConfig.debugCursorSimPos))
		{
			editor->debugConfig.debugCursorSimPos = !editor->debugConfig.debugCursorSimPos;
		}
		gridLayoutHelper.IncrementElementCount();


		gridLayoutHelper.GetElementPosition(curX, curY);
		if (RenderToggle(editor, gameInputState, gameRenderSetup, screenMousePos,
			curX, curY, btnWidth, btnHeight, "Debug Pathing Window", editor->tabConfig.showPathingDebug))
		{
			editor->tabConfig.showPathingDebug = !editor->tabConfig.showPathingDebug;
		}
		gridLayoutHelper.IncrementElementCount();

		if (editor->tabConfig.showPathingDebug)
		{
			RenderPathingDebugTab(gameInputState, gameRenderSetup, screenMousePos, editor);
		}


		gridLayoutHelper.GetElementPosition(curX, curY);
		if (RenderToggle(editor, gameInputState, gameRenderSetup, screenMousePos,
			curX, curY, btnWidth, btnHeight, "Show Grid", editor->gridConfig.showGrid))
		{
			editor->gridConfig.showGrid = !editor->gridConfig.showGrid;
		}
		gridLayoutHelper.IncrementElementCount();

		if (editor->gridConfig.showGrid)
		{
			gridLayoutHelper.GetElementPosition(curX, curY);
			if (RenderToggle(editor, gameInputState, gameRenderSetup, screenMousePos,
				curX, curY, btnWidth, btnHeight, "Show Grid Coord", editor->gridConfig.showCellGridCoord))
			{
				editor->gridConfig.showCellGridCoord = !editor->gridConfig.showCellGridCoord;
				editor->gridConfig.showCellSimCoord = false;
			}
			gridLayoutHelper.IncrementElementCount();


			gridLayoutHelper.GetElementPosition(curX, curY);
			if (RenderToggle(editor, gameInputState, gameRenderSetup, screenMousePos,
				curX, curY, btnWidth, btnHeight, "Show Grid Sim Pos", editor->gridConfig.showCellSimCoord))
			{
				editor->gridConfig.showCellSimCoord = !editor->gridConfig.showCellSimCoord;
				editor->gridConfig.showCellGridCoord = false;
			}
			gridLayoutHelper.IncrementElementCount();
		}

		gridLayoutHelper.GetElementPosition(curX, curY);
		if (RenderToggle(editor, gameInputState, gameRenderSetup, screenMousePos,
			curX, curY, btnWidth, btnHeight, "Debug Triangle", editor->highlightTriangle))
		{
			editor->highlightTriangle = !editor->highlightTriangle;
		}
		gridLayoutHelper.IncrementElementCount();


		gridLayoutHelper.GetElementPosition(curX, curY);
		if (RenderToggle(editor, gameInputState, gameRenderSetup, screenMousePos,
			curX, curY, btnWidth, btnHeight, "Hide Obstacles", editor->hideObstacles))
		{
			editor->hideObstacles = !editor->hideObstacles;
		}
		gridLayoutHelper.IncrementElementCount();


		gridLayoutHelper.GetElementPosition(curX, curY);
		if (RenderToggle(editor, gameInputState, gameRenderSetup, screenMousePos,
			curX, curY, btnWidth, btnHeight, "Debug Grid", editor->highlightGrid))
		{
			editor->highlightGrid = !editor->highlightGrid;
		}
		gridLayoutHelper.IncrementElementCount();

		/*
		gridLayoutHelper.GetElementPosition(curX, curY);
		if (RenderToggle(editor, gameInputState, gameRenderSetup, screenMousePos,
			curX, curY, btnWidth, btnHeight, "Set Pathing Start", editor->choosingPathingStart))
		{
			editor->choosingPathingEnd = false;
			editor->choosingPathingStart = !editor->choosingPathingStart;
		}
		gridLayoutHelper.IncrementElementCount();


		gridLayoutHelper.GetElementPosition(curX, curY);
		if (RenderToggle(editor, gameInputState, gameRenderSetup, screenMousePos,
			curX, curY, btnWidth, btnHeight, "Set Pathing End", editor->choosingPathingEnd))
		{
			editor->choosingPathingStart = false;
			editor->choosingPathingEnd = !editor->choosingPathingEnd;
		}
		gridLayoutHelper.IncrementElementCount();


		gridLayoutHelper.GetElementPosition(curX, curY);
		if (RenderSpecialButton(editor, gameInputState, gameRenderSetup, screenMousePos,
			curX, curY, btnWidth, btnHeight, "Clear Pathing Start"))
		{
			editor->coreData->editorEvents.push(EditorEvent::CLEAR_PATHING_START);
		}
		gridLayoutHelper.IncrementElementCount();


		gridLayoutHelper.GetElementPosition(curX, curY);
		if (RenderSpecialButton(editor, gameInputState, gameRenderSetup, screenMousePos,
			curX, curY, btnWidth, btnHeight, "Clear Pathing End"))
		{
			editor->coreData->editorEvents.push(EditorEvent::CLEAR_PATHING_END);
		}
		gridLayoutHelper.IncrementElementCount();

		gridLayoutHelper.GetElementPosition(curX, curY);
		if (RenderSpecialButton(editor, gameInputState, gameRenderSetup, screenMousePos,
			curX, curY, btnWidth, btnHeight, "PATH!"))
		{
			editor->coreData->editorEvents.push(EditorEvent::PATH);
		}
		gridLayoutHelper.IncrementElementCount();
		*/


		gridLayoutHelper.GetElementPosition(curX, curY);
		if (RenderToggle(editor, gameInputState, gameRenderSetup, screenMousePos,
			curX, curY, btnWidth, btnHeight, "Enter SimMode", editor->isInSimMode))
		{
			editor->isInSimMode = !editor->isInSimMode;
			editor->coreData->editorEvents.push(EditorEvent::ENTER_SIM_MODE);
		}
		gridLayoutHelper.IncrementElementCount();



		float thickness = 0.5;
		curX = startX;
		curY = startY;
		int temp = gridLayoutHelper.GetNumRow();
		float lineHeight = gridLayoutHelper.GetNumRow() * btnHeight;

		for (int x = 0; x <= gridLayoutHelper.numCol; x++)
		{
			curX = startX + btnWidth * x;

			glm::vec3 pos = glm::vec3(curX, curY, 0);
			glm::vec3 end = pos;
			end.y -= lineHeight;

			GameRender::RenderLine(gameRenderCommands, group, gameAssets, GameRender::COLOR_BLUE, pos, end, thickness);
		}

		
		curX = startX;
		curY = startY;
		float lineWidth = gridLayoutHelper.numCol * btnWidth;
		for (int y = 0; y <= gridLayoutHelper.GetNumRow(); y++)
		{
			curY = startY - btnHeight * y;

			glm::vec3 pos = glm::vec3(curX, curY, 0);
			glm::vec3 end = pos;
			end.x += lineWidth;

			GameRender::RenderLine(gameRenderCommands, group, gameAssets, GameRender::COLOR_BLUE, pos, end, thickness);
		}

	}
};