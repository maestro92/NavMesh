#pragma once

#include "../../PlatformShared/platform_shared.h"
#include <string>
#include <queue>
#include "../world.h"

struct EntityOption
{
	std::string name;
	std::vector<glm::vec3> vertices;
};

enum EditorEvent {
	SAVE,
	SHOW_GRID,
	DEBUG_TRIANGLE,
	DEBUG_GRID,
	TRIANGULATE
};

struct EditorStateData
{
	std::queue<EditorEvent> editorEvents;
};

struct HighlightGridCell
{
	int cellX;
	int cellY;
	MapCell* cell;

	void Reset()
	{
		cellX = -1;
		cellY = -1;
		cell = NULL;
	}

	bool IsEmpty()
	{
		return cellX == -1 && cellY == -1 && cell == NULL;
	}
};

struct EditorState {

	EntityOption* options;
	int numOptions;

	EntityOption* selected;
	EntityOption* highlighted;
	bool consumingMouse;

	// get rid of this indirection
	EditorStateData* coreData;
	bool showGrid;
	bool highlightTriangle;
	bool highlightGrid;

	std::string mapCellDebugString;
	HighlightGridCell hightlightMapCell;

	Entity* draggedEntity;
	glm::vec3 draggedPivot;

	bool IsInSelectionMode()
	{
		return selected != NULL;
	}
};

namespace EditorMain
{
	glm::vec4 COLOR_HIGHLIGHT_GRID = glm::vec4(1, 1, 0, 0.5);
}