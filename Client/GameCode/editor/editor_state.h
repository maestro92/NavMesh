#pragma once

#include "../../PlatformShared/platform_shared.h"
#include <string>
#include <queue>
#include "../world.h"

struct EntityOption
{
	std::string name;

	// agent
	bool isAgent;
	float agentRadius;

	// obstacles
	std::vector<glm::vec3> vertices;

	void clear()
	{
		name.clear();
		vertices.clear();
		isAgent = false;
		agentRadius = -1;
	}
};

enum EditorEvent {
	SAVE,
	TRIANGULATE,
	CLEAR_PATHING_START,
	CLEAR_PATHING_END,
	PATH,
	ENTER_SIM_MODE
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

	struct DebugConfig
	{
		bool debugCamera;
		bool debugCursorSimPos;
	};

	struct PathingDebugConfig
	{
		bool showTunnel;
		bool showPortals;
		
		bool showModifiedTunnel;
		bool showModifiedPortals;
		bool showAnePerps;
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

	Editor::DebugConfig debugConfig;

	Editor::PathingDebugConfig pathingDebugConfig;

	bool hideObstacles;

	bool highlightTriangle;
	bool highlightGrid;
	bool isEditingEntities;

	bool showTriangleIds;


	bool choosingPathingStart;
	bool choosingPathingEnd;

	std::string mapCellDebugString;
	HighlightGridCell hightlightMapCell;

	bool isInSimMode;

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