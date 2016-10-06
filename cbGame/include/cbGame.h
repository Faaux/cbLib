#pragma once
#include <cbPlatform.h>
#include <cbMemory.h>

extern Win32PlatformCode Platform;

struct GameState
{
	bool IsInitialized;

	mem_size ArenaSize;
	cbArena Arena;
};
