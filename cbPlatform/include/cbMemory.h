#pragma once

// ToDo: Make this whole thing Thread Safe
#include <cbInclude.h>
#include <cstring>

struct cbArena
{
	uint8 *Start;
	size_t Size;
	size_t Used;
};

#define ZeroStruct(Instance) ZeroSize(sizeof(Instance), &(Instance))
#define ZeroArray(Count, Pointer) ZeroSize(Count*sizeof((Pointer)[0]), Pointer)
inline void ZeroSize(size_t size, void *ptr)
{
	uint8 *byte = (uint8 *)ptr;
	while (size--)
	{
		*byte++ = 0;
	}
}

inline void InitArena(cbArena *mem, size_t size, void* ptr)
{
	mem->Start = (uint8*)ptr;
	mem->Size = size;
	mem->Used = 0;
}

enum ArenaPushFlags
{
	ZeroArenaMemory = 0x01
};

struct ArenaPushParams
{
	uint32 Alignment;
	uint32 Flags;
};

inline ArenaPushParams GetDefaultArenaParams()
{
	ArenaPushParams params;
	params.Alignment = 4;
	params.Flags = ZeroArenaMemory;

	return params;
}


#define PushStruct(Arena, type, ...) (type *)PushSize_(Arena, sizeof(type), ## __VA_ARGS__)
#define PushArray(Arena, Count, type, ...) (type *)PushSize_(Arena, (Count)*sizeof(type), ## __VA_ARGS__)
#define PushSize(Arena, Size, ...) PushSize_(Arena, Size, ## __VA_ARGS__)
#define PushCopy(Arena, Size, Source, ...) Copy(Size, Source, PushSize_(Arena, Size, ## __VA_ARGS__))

inline size_t GetAlignmentSize(cbArena* mem, uint32 alignment)
{
	size_t result = 0;

	size_t resultPointer = (size_t)mem->Start + mem->Used;
	size_t alignmentMask = alignment - 1;

	if(resultPointer & alignmentMask)
	{
		result = alignment - (resultPointer & alignmentMask);
	}

	return result;
}

inline void *PushSize_(cbArena *mem, size_t size, ArenaPushParams params = GetDefaultArenaParams())
{
	size_t alignmentSize = GetAlignmentSize(mem, params.Alignment);
	size_t sizeWithAlignment = size + alignmentSize;

	Assert(mem->Used + sizeWithAlignment <= mem->Size);

	void* result = mem->Start + mem->Used + alignmentSize;
	mem->Used += sizeWithAlignment;

	Assert(sizeWithAlignment >= size);

	if(params.Flags & ZeroArenaMemory)
	{
		ZeroSize(size, result);
	}
	return result;
}

inline void Clear(cbArena* mem)
{
	InitArena(mem, mem->Size, mem->Start);
}

inline void SubArena(cbArena *result, cbArena *arena, size_t size, ArenaPushParams Params = GetDefaultArenaParams())
{
	result->Size = size;
	result->Start = (uint8 *)PushSize_(arena, size, Params);
	result->Used = 0;
}

inline void * Copy(size_t size, void *sourceInit, void *destInit)
{
	uint8 *source = (uint8 *)sourceInit;
	uint8 *dest = (uint8 *)destInit;
	while (size--) { *dest++ = *source++; }

	return destInit;
}