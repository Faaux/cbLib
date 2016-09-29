#pragma once
#include <cbPlatform.h>

#define GAME_LOOP(name) void name(float deltaTime, Win32PlatformCode platformCode)
typedef GAME_LOOP(game_loop);
inline GAME_LOOP(GameLoopStub)
{
}