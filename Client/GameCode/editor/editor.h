#pragma once
#include "../PlatformShared/platform_shared.h"

#include "../game_render.h"
#include "../game_state.h"
#include "editor_state.h"

namespace Editor
{

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
		editorState->options = new EntityOption[10];

		editorState->options[0].name = "Square Rock";

		editorState->options[1].name = "Rock 1";

		editorState->options[2].name = "Rock 2";

		editorState->options[3].name = "Rock 3";

	}


	void RenderChoice(
		GameRender::GameRenderState* gameRenderState,
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

		// background
		GameRender::PushBitmap(gameRenderCommands, renderGroup, defaultBitmap, glm::vec4(0, 0, 0.25, 0.25), profileRectMin,
			halfDim, GameRender::AlignmentMode::Left, GameRender::AlignmentMode::Top);

		glm::vec3 pos = profileRectMin - glm::vec3(0, height, 0);
		GameRender::DEBUGTextLine(entityOption->name.c_str(), gameRenderState, pos, 1);
	}



	void TickAndRenderEditorMenu(GameMemory* gameMemory,
		GameInputState* gameInputState,
		GameRender::GameRenderState* gameRenderState,
		glm::ivec2 windowDimensions, DebugModeState* debugModeState)
	{
		GameAssets* gameAssets = gameRenderState->gameAssets;
		RenderSystem::RenderGroup* group = gameRenderState->renderGroup;
		RenderSystem::GameRenderCommands* gameRenderCommands = gameRenderState->gameRenderCommands;

		GameState* gameState = (GameState*)gameMemory->permenentStorage;
		TransientState* transientState = (TransientState*)gameMemory->transientStorage;

		EditorState* editor = &gameState->editorState;

		float halfWidth = gameRenderCommands->settings.dims.x / 2.0f;
		float halfHeight = gameRenderCommands->settings.dims.y / 2.0f;


		int numCol = 2;
		int numRow = 2;
		int optionWidth = 100;
		int optionHeight = 100;

		int startX = halfWidth - numCol * optionWidth;
		int startY = halfHeight;

		int curX = startX;
		int curY = startY;

		int entityOptionIndex = 0;
		for (int y = 0; y < numCol; y++)
		{
			curY = startY - optionHeight * y;

			for (int x = 0; x < numRow; x++)
			{
				curX = startX + optionWidth * x;

				EntityOption* entityOption = &editor->options[entityOptionIndex];

				RenderChoice(gameRenderState, entityOption, curX, curY, optionWidth, optionHeight);
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