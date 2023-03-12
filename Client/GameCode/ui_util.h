#pragma once

#include "../PlatformShared/platform_shared.h"

namespace UIUtil
{
	/*
        mousePos

      0,768                 1023,768
          ______________________
         |                      |
         |                      |
         |                      |
         |                      |
         |                      |
         |______________________|
       0,0                      1023,0



        ScreenRenderPos: 1024 * 768

      -512,384               511,384
          ______________________
         |                      |
         |                      |
         |          0,0         |
         |                      |
         |                      |
         |______________________|
      -512,-384              511,-384

	*/
	glm::vec3 PlatformMouseToScreenRenderPos(RenderSystem::GameRenderCommands* gameRenderCommands, glm::ivec2 mousePos)
	{
		float halfWidth = gameRenderCommands->settings.dims.x / 2.0f;
		float halfHeight = gameRenderCommands->settings.dims.y / 2.0f;
		return glm::vec3(mousePos.x - halfWidth, mousePos.y - halfHeight, 0);
	}

}
