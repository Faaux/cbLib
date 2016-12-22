#pragma once
#include "cbInclude.h"
#include "cbPlatform.h"
#include "cbMemory.h"
#include "cbConsole.h"
#include "cbCamera.h"


struct GameState
{
	bool IsInitialized;
	mem_size ArenaSize;
	cbArena Arena;
	cbConsole* Console;
	Camera* Camera;
};

struct TransientStorage
{
	bool IsInitialized;
	cbArena RenderGroupArena;
	cbArena ShaderArena;
	cbArena ModelArena;
};


extern Win32PlatformCode Platform;
extern TransientStorage* TransStorage;
extern cbConsole* Console;