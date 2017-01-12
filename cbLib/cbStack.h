#pragma once
#include "cbInclude.h"
#include "cbMemory.h"

struct cbStack
{
	void *Base;
	void *Current;
	uint64 Size;
	uint64 SizeLeft;
};

struct cbStackHeader
{
	bool IsFree;
	uint64 Size;
};

inline bool IsStackEmpty(cbStack *stack)
{
	return stack->Size == stack->SizeLeft;
}

inline cbStack CreateStack(void *memory, uint64 size)
{
	cbStack result;
	result.Base = memory;
	result.Current = (uint8 *)result.Base + size;
	result.Size = size;
	result.SizeLeft = size;

	return result;
}

inline cbStack *CreateStack(cbArena *arena, uint64 size)
{
	cbStack* result = (cbStack *)PushSize(arena, size + sizeof(cbStack));
	result->Base = (uint8 *)result + sizeof(cbStack);
	result->Current = (uint8 *)result->Base + size;
	result->Size = size;
	result->SizeLeft = size;

	return result;
}

inline void *PushStackElem(cbStack *stack, uint64 size)
{
	uint64 totalSize = size + sizeof(cbStackHeader);
	// Stack Full!
	if (stack->SizeLeft < totalSize)
	{
		Assert(false);
		return nullptr;
	}
	uint8 *result = (uint8 *)stack->Current - size;
	uint8 *headerStart = result - sizeof(cbStackHeader);

	ZeroSize(totalSize, headerStart);
	stack->Current = headerStart;
	stack->SizeLeft -= size + sizeof(cbStackHeader);

	((cbStackHeader *)headerStart)->Size = size;

	return result;
}

inline void FreeStackElem(cbStack *stack, void* elem)
{
	uint8 *current = (uint8 *)stack->Current;

	//Check that we are not at the end;
	Assert(!IsStackEmpty(stack));

	bool found = false;
	while (!found)
	{
		if (current + sizeof(cbStackHeader) == elem)
		{
			found = true;
			((cbStackHeader *)current)->IsFree = true;
		}
		else
		{
			// Go To Next Elem
			current = current + (((cbStackHeader *)current)->Size + sizeof(cbStackHeader));
			Assert(current < (uint8 *)stack->Base + stack->Size); // Tried to free something that isnt in this stack
		}
	}

	// If we freed the first element, cleanup stack

	if (current == (uint8 *)stack->Current)
	{
		while (((cbStackHeader *)stack->Current)->IsFree && stack->Current != (uint8*)stack->Base + stack->Size)
		{
			uint64 totalSize = ((cbStackHeader *)stack->Current)->Size + sizeof(cbStackHeader);

			stack->SizeLeft += totalSize;
			stack->Current = (uint8 *)stack->Current + totalSize;
		}
	}
}