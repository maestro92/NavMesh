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

		editorState->options = new EntityOption[10];
		EntityOption option;

		option.name = "Square";		
		vertices.clear();
		vertices.push_back(glm::vec3(-1.0, -1.0, 0.0));
		vertices.push_back(glm::vec3(1.0, -1.0, 0.0));
		vertices.push_back(glm::vec3(1.0, 1.0, 0.0));
		vertices.push_back(glm::vec3(-1.0, 1.0, 0.0));
		option.vertices = vertices;
		editorState->options[0] = option;


		option.name = "Concave";
		vertices.clear();
		vertices.push_back(glm::vec3(4.0, 0.0, 0.0));
		vertices.push_back(glm::vec3(2.0, 4.0, 0.0));
		vertices.push_back(glm::vec3(0.0, 6.0, 0.0));
		vertices.push_back(glm::vec3(-5.0, 0.0, 0.0));
		vertices.push_back(glm::vec3(-2.0, -5.0, 0.0));
		vertices.push_back(glm::vec3(2.0, -9.0, 0.0));
		vertices.push_back(glm::vec3(1.0, -3.0, 0.0));
		option.vertices = vertices;
		editorState->options[1] = option;


		option.name = "4 point star";
		vertices.clear();
		vertices.push_back(glm::vec3(5.0, 0.0, 0.0));
		vertices.push_back(glm::vec3(1.0, 1.0, 0.0));
		vertices.push_back(glm::vec3(0.0, 5.0, 0.0));
		vertices.push_back(glm::vec3(-1.0, 1.0, 0.0));
		vertices.push_back(glm::vec3(-5.0, 0.0, 0.0));
		vertices.push_back(glm::vec3(-1.0, -1.0, 0.0));
		vertices.push_back(glm::vec3(0.0, -5.0, 0.0));
		vertices.push_back(glm::vec3(1.0, -1.0, 0.0));
		option.vertices = vertices;
		editorState->options[2] = option;


		option.name = "Hexagon";
		vertices.clear();
		vertices.push_back(glm::vec3(5.0, 0.0, 0.0));
		vertices.push_back(glm::vec3(3.0, 4.0, 0.0));
		vertices.push_back(glm::vec3(-4.0, 6.0, 0.0));
		vertices.push_back(glm::vec3(-7.0, 4.0, 0.0));
		vertices.push_back(glm::vec3(-8.0, 0.0, 0.0));
		vertices.push_back(glm::vec3(0.0, -4.0, 0.0));
		option.vertices = vertices;
		editorState->options[3] = option;

		editorState->numOptions = 4;
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
			if (gameInputState->mouseButtons[(int)PlatformMouseButton_Left].endedDown)
			{
				editorState->selected = entityOption;
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
		int numRow = (editor->numOptions + (numCol - 1)) / numCol;		
		
		int optionWidth = 200;
		int optionHeight = 50;

		int startX = halfWidth - numCol * optionWidth;
		int startY = halfHeight;

		int curX = startX;
		int curY = startY;

		editor->highlighted = NULL;

		int entityOptionIndex = 0;
		for (int y = 0; y < numCol; y++)
		{
			curY = startY - optionHeight * y;

			for (int x = 0; x < numRow; x++)
			{
				if (entityOptionIndex >= editor->numOptions)
				{
					break;
				}

				curX = startX + optionWidth * x;

				EntityOption* entityOption = &editor->options[entityOptionIndex];

				// in UI render space, y starts from bottom
				int minX = curX;
				int minY = curY - optionHeight;
				RenderChoice(editor, gameInputState, gameRenderSetup, screenMousePos, entityOption, curX, minY, optionWidth, optionHeight);
				entityOptionIndex++;
			}
		}

		float thickness = 10;
		curX = startX;
		curY = startY;
		float lineHeight = numRow * optionHeight;

		for (int x = 0; x < numCol; x++)
		{
			curX = startX + optionWidth * x;

			glm::vec3 pos = glm::vec3(curX, curY, 0);
			glm::vec3 end = pos;
			end.y -= lineHeight;

			GameRender::RenderLine(gameRenderCommands, group, gameAssets, GameRender::COLOR_BLUE, pos, end, thickness);
		}

		
		curX = startX;
		curY = startY;
		float lineWidth = numCol * optionWidth;
		for (int y = 0; y < numRow; y++)
		{
			curY = startY - optionHeight * y;

			glm::vec3 pos = glm::vec3(curX, curY, 0);
			glm::vec3 end = pos;
			end.x += lineWidth;

			GameRender::RenderLine(gameRenderCommands, group, gameAssets, GameRender::COLOR_BLUE, pos, end, thickness);
		}
		
	}

};