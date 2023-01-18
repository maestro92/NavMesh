#pragma once


#include "../NavMesh/debug_interface.h"

#include <vector>
#include <unordered_map>
#include <iostream>
struct DebugElement;

// Represents a begin and closing block
struct ProfileNode
{
	DebugElement* element;
	uint16 threadId;
	uint32 parentRelativeClock;
	uint32 duration;

	std::vector<ProfileNode*> children;

	ProfileNode(int threadIdInit)
	{
		threadId = threadIdInit;
	}

	void AddChild(ProfileNode* child)
	{
		children.push_back(child);
	}
};


// does this overlap with ProfileNode?
struct OpenDebugBlock
{
	uint32 startingFrameIndex;
	DebugEvent openingEvent;
	ProfileNode* profileNode;

	// if stored in the free list linked list in DebugState->headFreeBlock
	// next stores the next one 

	// if this node is being used, next is storing the parent 
	// OpenDebugBlock* next;
};



#define MAX_REGIONS_PER_FRAME 2*4096
struct DebugFrame
{
	uint64 beginClock;
	uint64 endClock;
	float wallSecondsElapsed;

	uint32 numProfileBlocks;

	int frameIndex;

	// all profile nodes, regardless of threads are under this
	ProfileNode* rootProfileNode;

	void PrintDebug()
	{
		std::cout << "frameIndex " << frameIndex << ", beginClock " << beginClock << " endClock " << endClock << std::endl;
	}
};




struct DebugThread
{
	int id;

	// LinkedList of OpenBlocks, this is implemented as a stack
	// OpenDebugBlock* headOpenBlock;

	std::vector<OpenDebugBlock> openDebugBlockStack;

	DebugThread(int threadId)
	{
		id = threadId;
	}

	OpenDebugBlock* GetTopOpenBlock()
	{
		return &openDebugBlockStack[openDebugBlockStack.size() - 1];
	}

	bool CurrentlyHasAnOpenBlock()
	{
		return openDebugBlockStack.size() > 0;
	}
};




struct DebugElement
{
//	std::string GUID;
//	std::string name;
	char* GUID;
	char* name;
	uint32 lineNumber;

	std::vector<ProfileNode*> storedEvents; // replace this with a fifoList

	DebugElement()	{}

	DebugElement(char* GUIDInit)
	{
		GUID = GUIDInit;
	}

	// needs a deep copy
	ProfileNode* AddProfileNode(ProfileNode* node)
	{
		// get this from the memory arena
		storedEvents.push_back(node);
		return storedEvents[storedEvents.size() - 1];
	}
};

struct HashTable
{
	std::unordered_map<std::string, DebugElement> table;		// to be replaced by our own hash table
};




struct DebugState
{
	bool isInitalized;

	// Things for collation
	DebugFrame* collationFrame;
	DebugFrame* mostRecentFrame;
	MemoryArena collationArena;
	uint32 maxFrames;
	uint32 numFrames;
	DebugFrame* frames;

	// a linked list of threads.
	// we are using a linked list for all the threads.
	// DebugThread* headThread;
	std::vector<DebugThread> threads;							// To be replaced by linked list
	// HashTable* debugElements;
	// std::unordered_map<std::string, DebugElement> debugElements;
	std::vector<DebugElement> debugElements;

	// a linked list of free blocks. When DebugThreads wants a new OpenDebugBlock
	// you request it from here.
	OpenDebugBlock* headFreeBlock;


	// Things for rendering
	RenderSystem::RenderGroup* renderGroup;
	MemoryArena debugArena; 

	void PrintDebugElement()
	{
		std::cout << "printing debugElements" << debugElements.size() << std::endl;
		for (int i = 0; i < debugElements.size(); i++)
		{
			std::cout << "		" << debugElements[i].GUID << std::endl;
		}
	}
};



void initDebugFrame(DebugFrame* debugFrame, uint64 beginClock, MemoryArena* memoryArena, int frameIndex)
{
	debugFrame->beginClock = beginClock;
	debugFrame->endClock = 0;
	debugFrame->wallSecondsElapsed = 0.0f;
	debugFrame->frameIndex = frameIndex;

	debugFrame->rootProfileNode->children.clear();
}


DebugThread* TryGetOrCreateDebugThread(DebugState* debugState, uint32 threadId)
{
	for (int i = 0; i < debugState->threads.size(); i++)
	{
		if (debugState->threads[i].id == threadId)
		{
			return &debugState->threads[i];
		}
	}

	DebugThread debugThread(threadId);
	debugState->threads.push_back(debugThread);
	return &debugState->threads[debugState->threads.size() - 1];
}



bool IsMatchingDebugEvents(const DebugEvent& lhs, const DebugEvent& rhs)
{
	return lhs.threadId == rhs.threadId;
}


void ProcessFrameMarkerDebugEvent(DebugState* debugState, DebugEvent* event)
{
	// if we already frame 
	if (debugState->collationFrame)
	{		
		debugState->collationFrame->endClock = event->clock;
		if (debugState->collationFrame->rootProfileNode)
		{
			debugState->collationFrame->rootProfileNode->duration = debugState->collationFrame->endClock - debugState->collationFrame->beginClock;
		}

		debugState->collationFrame->wallSecondsElapsed = event->wallSecondsElapsed;

		debugState->mostRecentFrame = debugState->collationFrame;
		debugState->numFrames++;

	//	std::cout << "wallSecondsElapsed " << event->wallSecondsElapsed << std::endl;
	//	std::cout << "debugState->collationFrame->wallSecondsElapsed " << debugState->collationFrame->wallSecondsElapsed << std::endl;

	}


	int frameIndex = debugState->numFrames % debugState->maxFrames;

	debugState->collationFrame = &debugState->frames[frameIndex];
	initDebugFrame(debugState->collationFrame, event->clock, &debugState->collationArena, debugState->numFrames);
}




DebugElement* FindDebugElement(std::vector<DebugElement> debugElements, DebugEvent* event)
{
	DebugElement* result = NULL;
	for (int i = 0; i < debugElements.size(); i++)
	{
		if (AreStringsEqual(debugElements[i].GUID, event->GUID))
		{
			result = &debugElements[i];
			return result;
		}
	}
	return NULL;
}


DebugElement* GetOrCreateDebugElement(DebugState* debugState, DebugEvent* event)
{
	std::string key = std::string(event->GUID);

	// debugState->PrintDebugElement();
	// DebugElement* result = FindDebugElement(debugState->debugElements, event);

	DebugElement* result = NULL;
	for (int i = 0; i < debugState->debugElements.size(); i++)
	{
		if (AreStringsEqual(debugState->debugElements[i].GUID, event->GUID))
		{
			result = &debugState->debugElements[i];
			break;
		}
	}

	if (result == NULL)
	{
		// we create a new debug element
		debugState->debugElements.emplace_back(event->GUID);
		int index = debugState->debugElements.size() - 1;
		result = &debugState->debugElements[index];
	}
	return result;
}


OpenDebugBlock* AddTopOpenDebugBlock(DebugState* debugState, DebugThread* debugThread)
{
	OpenDebugBlock newBlock;
	debugThread->openDebugBlockStack.push_back(newBlock);

	int index = debugThread->openDebugBlockStack.size() - 1;
	return &debugThread->openDebugBlockStack[index];
}

void RemoveTopOpenDebugBlock(DebugState* debugState, DebugThread* debugThread)
{
	return debugThread->openDebugBlockStack.pop_back();
}

void InitOpenDebugBlock(OpenDebugBlock* debugBlock, int startFrameIndex, 
							ProfileNode* profileEvent, DebugEvent* openingEvent)
{
	debugBlock->startingFrameIndex = startFrameIndex;
	debugBlock->profileNode = profileEvent;
	debugBlock->openingEvent = *openingEvent;
}

void ProcessDebugEvents(DebugState* debugState, DebugEvent* debugEventsArray, uint32 numEvents)
{
	// Reset the debug table, start the pingpong 

	// move all of it into my debug storage

	for (int i = 0; i < numEvents; i++)
	{
		DebugEvent* event = &debugEventsArray[i];

		if (event->type == DebugEventType::FrameMarker)
		{
			ProcessFrameMarkerDebugEvent(debugState, event);
		}
		else if (debugState->collationFrame != NULL)
		{
		//	uint32 frameIndex = debugState->numFrames - 1;
			int threadId = 1;
			DebugThread* thread = TryGetOrCreateDebugThread(debugState, threadId);

			if (event->type == DebugEventType::BeginBlock)
			{
				DebugElement* element = GetOrCreateDebugElement(debugState, event);

				ProfileNode* profileNode = new ProfileNode(event->threadId);	// Get this from the memory arena
				profileNode->parentRelativeClock = event->clock - debugState->collationFrame->beginClock;
				profileNode->element = element;
				element->AddProfileNode(profileNode);

				uint64 clockBasis = 0; 

				ProfileNode* parentProfileNode = NULL;
				if (thread->CurrentlyHasAnOpenBlock())
				{
					parentProfileNode = thread->GetTopOpenBlock()->profileNode;
					clockBasis = thread->GetTopOpenBlock()->openingEvent.clock;
				}
				else
				{
					parentProfileNode = debugState->collationFrame->rootProfileNode;
					clockBasis = debugState->collationFrame->beginClock;
				}

				parentProfileNode->parentRelativeClock = event->clock - clockBasis;
				parentProfileNode->AddChild(profileNode);



				OpenDebugBlock* debugBlock = AddTopOpenDebugBlock(debugState, thread);
				InitOpenDebugBlock(debugBlock, debugState->collationFrame->frameIndex, profileNode, event);


			}
			else if (event->type == DebugEventType::EndBlock)
			{
				assert(debugState->collationFrame);
			//	uint32 frameIndex = debugState->numFrames - 1;

				uint64 relativeClock = event->clock - debugState->collationFrame->beginClock;

				if (thread->CurrentlyHasAnOpenBlock())
				{
					OpenDebugBlock* matchingBlock = thread->GetTopOpenBlock();

					if (IsMatchingDebugEvents(matchingBlock->openingEvent, *event))
					{	
						matchingBlock->profileNode->duration = (uint32)(event->clock - matchingBlock->openingEvent.clock);

						RemoveTopOpenDebugBlock(debugState, thread);
					}
				}

			//	std::cout << "end block" << std::endl;

			}
			else
			{
				assert(!"Invalid event type");
			}
		}
	}
}



