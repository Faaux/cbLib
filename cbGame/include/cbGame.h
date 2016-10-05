#pragma once
#include <cbPlatform.h>

extern Win32PlatformCode Platform;

#define GAME_LOOP(name) void name(float deltaTime, GameMemory* gameMemory)
typedef GAME_LOOP(game_loop);
inline GAME_LOOP(GameLoopStub)
{
}

struct GameState
{
	bool IsInitialized;

	mem_size ArenaSize;
	cbArena Arena;
};
