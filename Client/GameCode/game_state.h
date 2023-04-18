#pragma once
#include "../PlatformShared/platform_shared.h"
#include "world.h"
#include "../NavMesh/asset.h"
#include "editor/editor_state.h"
#include "simulation_state.h"

// This is mirroring the sim_region struct in handmade_sim_region.h
struct GameState
{
	bool isInitalized;

	World world;

	Entity debugCameraEntity;

	bool mouseIsDebugMode;

	MemoryArena memoryArena;
	SimulationState simState;
	EditorState editorState;
};

struct TransientState
{
	bool isInitalized;
	MemoryArena memoryArena;
	GameAssets* assets;
	FontId debugFontId;
	LoadedFont* debugLoadedFont;
};

