#pragma once
#include "cbInclude.h"
#include "cbPlatform.h"
#include "cbMemory.h"
#include "cbConsole.h"


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
	cbArena RenderGroupArena;
	cbArena ShaderArena;
};


extern Win32PlatformCode Platform;
extern TransientStorage* TransStorage;
extern cbConsole* Console;