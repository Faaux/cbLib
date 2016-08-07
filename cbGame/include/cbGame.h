#pragma once

#define GAME_LOOP(name) void name(float deltaTime)
typedef GAME_LOOP(game_loop);
inline GAME_LOOP(GameLoopStub)
{
}