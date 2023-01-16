#pragma once


struct MemoryArena
{
	uint8* base;
	MemoryIndex size;
	MemoryIndex used;

	void Init(void* baseIn, MemoryIndex sizeIn)
	{
		base = (uint8*)baseIn;
		size = sizeIn;
		used = 0;
	}
};


// 100	alignment of 4
// 1000 alignment of 8
// 10000 alignment of 16
// assume we want offset of 16
// memory we want are 0, 16, 32, 48

// if resultPointer is 18, mask is 01111
// alignmentOffset is 16 - 18 % 16 = 16 - 2 = 14

MemoryIndex BytesToNextAlignment(MemoryArena* arena, MemoryIndex alignment)
{
	MemoryIndex alignmentOffset = 0;

	MemoryIndex resultPointer = (MemoryIndex)arena->base + arena->used;
	MemoryIndex alignmentMask = alignment - 1;

	if (resultPointer & alignmentMask)
	{
		alignmentOffset = alignment - (resultPointer & alignmentMask);
	}

	return alignmentOffset;
}

#define PushStruct(arena, type)			(type*)PushSize_(arena, sizeof(type))
#define PushArray(arena, count, type)	(type*)PushSize_(arena, (count) * sizeof(type))
#define PushSize(arena, size)			PushSize_(arena, size)

void* PushSize_(MemoryArena* arena, MemoryIndex sizeInit, MemoryIndex alignment = 4)
{
	MemoryIndex size = sizeInit;

	MemoryIndex bytesToGetNext = BytesToNextAlignment(arena, alignment);
	size += bytesToGetNext;


	if ((arena->used + size) > arena->size)
	{
		int a = 1;
	}
	assert((arena->used + size) <= arena->size);

	void* result = arena->base + arena->used + bytesToGetNext;
	arena->used += size;

	assert(size >= sizeInit);

	return (result);
}

void memcpyDiy(char* src, char* dst, int numBytes)
{
	while (numBytes)
	{
		*dst = *src;
		dst++;
		src++;
		numBytes--;
	}
}