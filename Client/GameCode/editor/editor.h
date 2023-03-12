#pragma once
#include "../PlatformShared/platform_shared.h"

#include "../game_render.h"
#include "../game_state.h"

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

	void RenderChoice(RenderSystem::GameRenderCommands* gameRenderCommands,
		RenderSystem::RenderGroup* renderGroup, GameAssets* gameAssets, int x, int y, int width, int height)
	{
		BitmapId bitmapID = GetFirstBitmapIdFrom(gameAssets, AssetFamilyType::Default);
		LoadedBitmap* defaultBitmap = GetBitmap(gameAssets, bitmapID);

		glm::vec3 profileRectMin = glm::vec3(x, y, 0);
		glm::vec3 profileRectMax = glm::vec3(x + width, y + height, 0);
		glm::vec3 halfDim = (profileRectMax - profileRectMin) / 2.0f;

		// background
		GameRender::PushBitmap(gameRenderCommands, renderGroup, defaultBitmap, glm::vec4(1, 1, 1, 1), profileRectMin,
			halfDim, GameRender::AlignmentMode::Left, GameRender::AlignmentMode::Top);

	}



	void RenderEditorMenu(GameMemory* gameMemory,
		GameAssets* gameAssets,
		RenderSystem::RenderGroup* group,
		GameInputState* gameInputState,
		RenderSystem::GameRenderCommands* gameRenderCommands,
		glm::ivec2 windowDimensions, DebugModeState* debugModeState)
	{
		GameState* gameState = (GameState*)gameMemory->permenentStorage;
		TransientState* transientState = (TransientState*)gameMemory->transientStorage;

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

		for (int y = 0; y < numCol; y++)
		{
			curY = startY - optionHeight * y;

			for (int x = 0; x < numRow; x++)
			{
				curX = startX + optionWidth * x;
				RenderChoice(gameRenderCommands, group, gameAssets, curX, curY, optionWidth, optionHeight);
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