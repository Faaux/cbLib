#pragma once
#include "cbInclude.h"

struct cbArena
{
	uint8 *Base;
	mem_size Size;
	mem_size Used;
};

#define ZeroStruct(Instance) ZeroSize(sizeof(Instance), &(Instance))
#define ZeroArray(Count, Pointer) ZeroSize(Count*sizeof((Pointer)[0]), Pointer)
#define ZeroSize(size, ptr) SetMemory(size, ptr, (uint8)0)
#define Fill16(value) ((uint16)(value << 8) + value)
#define Fill32(value) (((uint32)Fill16(value) << 16) + (uint32)Fill16(value))
#define Fill64(value) (((uint64)Fill32(value) << 32) + (uint64)Fill32(value))

inline void SetMemory(mem_size size, void *ptr, uint64 value)
{
	Assert(size % sizeof(uint64) == 0);

	uint64 *cur = (uint64 *)ptr;
	while (size)
	{
		*cur++ = value;
		size -= sizeof(uint64);
	}
}

inline void SetMemory(mem_size size, void *ptr, uint32 value)
{
	Assert(size % sizeof(uint32) == 0);

	uint32 *cur = (uint32 *)ptr;
	while (size)
	{
		*cur++ = value;
		size -= sizeof(uint32);
	}
}

inline void SetMemory(mem_size size, void *ptr, uint8 value)
{
	if (size % sizeof(uint64) == 0)
	{
		SetMemory(size, ptr, Fill64(value));
	}
	else if (size % sizeof(uint32) == 0)
	{
		SetMemory(size, ptr, Fill32(value));
	}
	else
	{
		uint8 *byte = (uint8 *)ptr;
		while (size--)
		{
			*byte++ = value;
		}
	}
}

inline void InitArena(cbArena *mem, mem_size size, void* ptr)
{
	mem->Base = (uint8*)ptr;
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

inline mem_size GetAlignmentSize(cbArena* mem, uint32 alignment)
{
	mem_size result = 0;

	mem_size resultPointer = (mem_size)mem->Base + mem->Used;
	mem_size alignmentMask = alignment - 1;

	if (resultPointer & alignmentMask)
	{
		result = alignment - (resultPointer & alignmentMask);
	}

	return result;
}

inline void *PushSize_(cbArena *mem, mem_size size, ArenaPushParams params = GetDefaultArenaParams())
{
	mem_size alignmentSize = GetAlignmentSize(mem, params.Alignment);
	mem_size sizeWithAlignment = size + alignmentSize;

	Assert(mem->Used + sizeWithAlignment <= mem->Size);

	void* result = mem->Base + mem->Used + alignmentSize;
	mem->Used += sizeWithAlignment;

	Assert(sizeWithAlignment >= size);

	if (params.Flags & ZeroArenaMemory)
	{
		ZeroSize(size, result);
	}
	return result;
}

inline void Clear(cbArena* mem)
{
	InitArena(mem, mem->Size, mem->Base);
}

inline void SubArena(cbArena *result, cbArena *arena, mem_size size, ArenaPushParams Params = GetDefaultArenaParams())
{
	result->Size = size;
	result->Base = (uint8 *)PushSize_(arena, size, Params);
	result->Used = 0;
}

inline void *Copy(mem_size size, void *sourceInit, void *destInit)
{
	uint8 *source = (uint8 *)sourceInit;
	uint8 *dest = (uint8 *)destInit;
	while (size--) { *dest++ = *source++; }

	return destInit;
}