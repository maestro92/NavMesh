#pragma once
#include <stdint.h>

#include "glm/ext.hpp"

#define Kilobytes(value) ((value)*1024LL)
#define Megabytes(value) (Kilobytes(value)*1024LL)
#define Gigabytes(value) (Megabytes(value)*1024LL)

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))
#define OffsetOf(type, member) (uintptr_t) &(((type*)0)->member)


typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef size_t MemoryIndex;

//#include "debug_interface.h"

#define POINTER_TO_UINT32(pointer) ((uint32)(MemoryIndex)(pointer))
#define UINT32_TO_POINTER(type, value) (type *)((MemoryIndex)value)

// make a type that this function template


std::ostream& operator<<(std::ostream& os, glm::ivec3 v)
{
	return os << v.x << " " << v.y << " " << v.z;
}

std::ostream& operator<<(std::ostream& os, glm::vec3 v)
{
	return os << v.x << " " << v.y << " " << v.z;
}

std::ostream& operator<<(std::ostream& os, glm::mat4 m)
{
	return os << m[0][0] << " " << m[0][1] << " " << m[0][2] << " " << m[0][3] << "\n"
		      << m[1][0] << " " << m[1][1] << " " << m[1][2] << " " << m[1][3] << "\n"
			  << m[2][0] << " " << m[2][1] << " " << m[2][2] << " " << m[2][3] << "\n"
	          << m[3][0] << " " << m[3][1] << " " << m[3][2] << " " << m[3][3] << "\n";;
}


struct BitmapInfo
{
	int width;
	int height;
	int pitch;
	void* memory;
};

// we are using the pointer value itself to store the value of the handle
// pointer points to null, , we change the value of the pointer to the handle

typedef BitmapInfo(*PlatformReadImageFile)(char* filename);
typedef void*(*PlatformAllocateTexture)(uint32 width, uint32 height, void* data);

typedef unsigned int(*PlatformAllocateTexture2)(uint32 width, uint32 height, void* data);



inline bool AreStringsEqual(const char *A, const char *B)
{
	// First check if they are pointing to the same pointer?
	bool result = (A == B);

	if (A && B)
	{
		while (*A && *B && (*A == *B))
		{
			++A;
			++B;
		}

		result = ((*A == 0) && (*B == 0));
	}

	return(result);
}



enum GameInputMouseButton
{
	PlatformMouseButton_Left,
	PlatformMouseButton_Middle,
	PlatformMouseButton_Right,
	PlatformMouseButton_Extended0,
	PlatformMouseButton_Extended1,

	PlatformMouseButton_Count,
};

struct GameButtonState
{
	int halfTransitionCount;
	bool endedDown;
};

struct GameInputState
{
	double dtForFrame;

	GameButtonState buttons[4];
	struct
	{
		GameButtonState moveForward;
		GameButtonState moveLeft;
		GameButtonState moveRight;
		GameButtonState moveBack;
	
	
		GameButtonState moveForward2;
		GameButtonState moveLeft2;
		GameButtonState moveRight2;
		GameButtonState moveBack2;
		GameButtonState moveUp2;
		GameButtonState moveDown2;
	};

	GameButtonState mouseButtons[PlatformMouseButton_Count];
	glm::ivec2 mousePos;

	/*
	// For debugging only;
	double mouseX, mouseY, mouseZ;
	bool shiftDown, altDown, controlDown;
	*/
};


struct PlatformAPI
{
	PlatformReadImageFile readImageFile;
	PlatformAllocateTexture allocateTexture;
};

struct DebugTable;
struct PlatformWorkQueue;

struct GameMemory
{
	uint64 permenentStorageSize;
	void* permenentStorage;

	uint64 transientStorageSize;
	void* transientStorage;

	uint64 debugStorageSize;
	void* debugStorage;
	
	DebugTable* debugTable;

	PlatformAPI platformAPI;
	PlatformWorkQueue* workQueue;
};


enum RenderGroupEntryType
{
	RenderGroupEntryType_Clear,
	RenderGroupEntryType_TexturedQuads,
};

struct RenderEntryHeader
{
	uint16 type;
};

struct RenderEntryClear
{
	glm::vec4 color;
};

struct RenderSetup
{
	glm::mat4 transformMatrix;
};

struct RenderGroupEntryTexturedQuads
{
	RenderSetup renderSetup;
	int numQuads;
	int masterVertexArrayOffset;
	int masterBitmapArrayOffset;
};

// this is just for convenience
struct RenderGroup
{
	RenderGroupEntryTexturedQuads* quads;
};


struct TexturedVertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 uv;
	glm::vec4 color;
};

struct LoadedBitmap
{
	void* memory;
	int width;
	int height;
	int pitch;
	// This is the OpenGL texture handle 
	// cant use GLuint since this is the platform layer
	uint32 textureHandle;
};

struct GameRenderSettings
{
	glm::ivec2 dims;
};

struct GameRenderCommands
{
	GameRenderSettings settings;

	uint8* pushBufferBase;
	uint32 pushBufferSize;	
	uint32 maxPushBufferSize;

	uint32 numRenderGroups;

	unsigned int maxNumVertex;
	unsigned int numVertex;
	TexturedVertex* masterVertexArray;

	// an array of pointer
	uint32 maxNumBitmaps;
	uint32 numBitmaps;
	LoadedBitmap** masterBitmapArray;


	// hack for now
	// eventually we want to add this to a render group concept
	// instead of per TexturedQuad.

	uint8* CurrentPushBufferAt()
	{
		return pushBufferBase + pushBufferSize;
	}

	bool HasSpaceFor(uint32 size)
	{
		return (pushBufferSize + size) <= maxPushBufferSize;
	}

	bool HasSpaceForVertex(int numVertices)
	{
		return numVertex + numVertices <= maxNumVertex;
	}

	void PrintDebug()
	{
		for (unsigned int i = 0; i < numVertex; i++)
		{			
			std::cout << "i " << i << ": " << masterVertexArray[i].position << std::endl;
		}
	}

};


typedef void(*GameUpdateAndRender_t)(GameMemory* gameMemory,
	GameInputState* gameInput,
	GameRenderCommands* gameRenderCommands, glm::ivec2 windowDimensions, bool isDebugMode);

typedef void(*DebugSystemUpdateAndRender_t)(GameMemory* gameMemory,
	GameInputState* gameInput,
	GameRenderCommands* gameRenderCommands, glm::ivec2 windowDimensions, bool isDebugMode);

