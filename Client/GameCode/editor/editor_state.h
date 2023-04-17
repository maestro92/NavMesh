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
	TRIANGULATE,
	CLEAR_PATHING_START,
	CLEAR_PATHING_END,
	PATH,
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

namespace Editor
{
	struct GridDisplayConfig
	{
		bool showGrid;
		bool showCellSimCoord;
		bool showCellGridCoord;
	};

}

struct EditorState {

	EntityOption* options;
	int numOptions;

	EntityOption* selectedOption;
	EntityOption* highlightedOption;
	bool consumingMouse;

	// get rid of this indirection
	EditorStateData* coreData;
	Editor::GridDisplayConfig gridConfig;

	bool hideObstacles;

	bool highlightTriangle;
	bool highlightGrid;


	bool choosingPathingStart;
	bool choosingPathingEnd;

	std::string mapCellDebugString;
	HighlightGridCell hightlightMapCell;

	Entity* draggedEntity;
	glm::vec3 draggedPivot;

	bool IsInSelectionMode()
	{
		return selectedOption != NULL;
	}
};

namespace EditorMain
{
	glm::vec4 COLOR_HIGHLIGHT_GRID = glm::vec4(1, 1, 0, 0.5);
}