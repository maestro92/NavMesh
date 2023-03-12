#pragma once

#include "../../PlatformShared/platform_shared.h"
#include <string>
#include <vector>

struct EntityOption
{
	std::string name;
	std::vector<glm::vec3> vertices;
};


struct EditorState {
	EntityOption* options;
	int numOptions;

	EntityOption* selected;
	EntityOption* highlighted;
};

