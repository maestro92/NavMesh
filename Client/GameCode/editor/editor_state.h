#pragma once

#include "../../PlatformShared/platform_shared.h"
#include <string>
// #include <vector>
#include <queue>
#include "../world.h"

struct EntityOption
{
	std::string name;
	std::vector<glm::vec3> vertices;
};

enum EditorEvent {
	SAVE,
	TRIANGULATE
};

struct EditorStateData
{
	std::queue<EditorEvent> editorEvents;
};

struct EditorState {
	EntityOption* options;
	int numOptions;

	EntityOption* selected;
	EntityOption* highlighted;
	bool consumingMouse;

	// get rid of this indirection
	EditorStateData* coreData;

	Entity* draggedEntity;
	glm::vec3 draggedPivot;

	bool IsInSelectionMode()
	{
		return selected != NULL;
	}
};

