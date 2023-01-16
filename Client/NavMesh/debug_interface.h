#pragma once

#include "../PlatformShared/platform_shared.h"

#include <iostream>

/*
struct DebugTable;

extern DebugTable* globalDebugTable;
*/

struct DebugModeState
{
	bool mouseDebugMode;
	bool cameraDebugMode;
};


enum DebugEventType
{
	FrameMarker,
	BeginBlock,
	EndBlock
};

struct DebugEvent
{
	uint64 clock;
	char* GUID;
	uint16 threadId;
	uint16 coreIndex;
	uint8 type;
	float wallSecondsElapsed;
};


#define RecordDebugEvent(globalDebugTable, eventType, GUIDInit)	\
	uint64 eventArrayIndex_eventIndex = globalDebugTable->eventArrayIndex_EventIndex++;	\
	uint32 arrayIndex = eventArrayIndex_eventIndex >> 32;			\
	uint32 eventIndex = eventArrayIndex_eventIndex & 0xFFFFFFFF;	\
	DebugEvent *Event = &globalDebugTable->events[arrayIndex][eventIndex];	\
	Event->clock = __rdtsc();										\
	Event->type = (uint8)eventType;									\
	Event->coreIndex = 0;		\
	Event->GUID = GUIDInit;		\
	Event->threadId = 1;		


// if you do something like BEGIN_BLOCK();  BEGIN_BLOCK()
// where things are on the same line, __COUNTER__ makes it so that its guaranteed to be unique
// here we want to concatenate "a b c d". Apparently, the preprocessor automatically concatenates adjacent strings
// https://stackoverflow.com/questions/5256313/c-c-macro-string-concatenation
// This is equivalent of handmade hero's UniqueFileCounterString()
#define GenerateGUID_(a,b,c,d)  a "|" #b "|" #c "|" d
#define GenerateGUID(a,b,c,d)  GenerateGUID_(a,b,c,d)
#define DEBUG_NAME(name) GenerateGUID(__FILE__, __LINE__, __COUNTER__, name)

#define BEGIN_BLOCK_(GUID)	{RecordDebugEvent(DebugEventType::BeginBlock, GUID);}
#define END_BLOCK_(GUID)	{RecordDebugEvent(DebugEventType::EndBlock, GUID);}

#define BEGIN_BLOCK(blockName)	BEGIN_BLOCK_(DEBUG_NAME(blockName))
#define END_BLOCK() END_BLOCK_(DEBUG_NAME("END_BLOCK_"))

#define FRAME_MARKER(secondsElapsedInit)	{RecordDebugEvent(DebugEventType::FrameMarker, DEBUG_NAME("Frame Marker"));	\
				Event->wallSecondsElapsed = secondsElapsedInit;}

struct DebugTable
{
	// everyone is writing into this array 
	uint32 currentEventArrayIndex;	// do we need this?

	// first 32 biit is arrayIndex, 2nd 32 bit is eventIndex
	uint64 volatile eventArrayIndex_EventIndex;

	DebugEvent events[2][65536];
};
