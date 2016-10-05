#pragma once
#include <cbPlatform.h>

extern Win32PlatformCode PlatformCode;

#define GAME_FREE(name) void name()
typedef GAME_FREE(game_free);
inline GAME_FREE(GameFreeStub)
{
}

#define GAME_INIT(name) void name(Win32PlatformCode platformCode, Win32Memory* memory)
typedef GAME_INIT(game_init);
inline GAME_INIT(GameInitStub)
{
}

#define GAME_LOOP(name) void name(float deltaTime)
typedef GAME_LOOP(game_loop);
inline GAME_LOOP(GameLoopStub)
{
}
