#pragma once
#include <cbInclude.h>
#include <cbPlatform.h>
#include <cbMemory.h>
#include <cbConsole.h>

extern Win32PlatformCode Platform;

struct GameState
{
	bool IsInitialized;
	mem_size ArenaSize;
	cbArena Arena;

	cbConsole* Console;
};

struct TransientStorage
{
	bool IsInitialized;
	mem_size RenderGroupSize;
	cbArena RenderGroupArena;
};