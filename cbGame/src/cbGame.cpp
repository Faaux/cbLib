#include <cbGame.h>
#include <cbInclude.h>
#include <cbBasic.h>
#include "cbOpenGLRenderer.h"

Win32PlatformCode Platform;

internal void Render(float deltaTime, GameMemory* memory, cbArena* renderArena)
{
	const mem_size size = 24;
	char num[size];
	char text[] = "Frame MS: ";
	const mem_size concatSize = ArrayCount(text) + ArrayCount(num) - 1;
	char concat[concatSize];
	
	static float smoothedDeltaTime = 0;
	smoothedDeltaTime = deltaTime * 0.1f + smoothedDeltaTime * 0.9f;
	cbFtoA(smoothedDeltaTime * 1000.f, num, size);

	char* toRender = cbConcatStr(concat, concatSize, text, ArrayCount(text), num, ArrayCount(num));
	PushRenderString(renderArena, 64, 10, 10, concatSize, toRender);

    Platform.SwapBuffer();
}

internal void Update()
{
}

EXPORT GAME_LOOP(GameLoop)
{
	Platform = gameMemory->Platform;

	GameState* gameState = (GameState*)gameMemory->PermanentStorage;
	if(!gameState->IsInitialized)
	{
		cbArena totalArena;
		mem_size totalSize = gameMemory->PermanentStorageSize - sizeof(GameState);
		InitArena(&totalArena, totalSize, (uint8 *)gameMemory->PermanentStorage + sizeof(GameState));
		gameState->Arena = totalArena;
		gameState->ArenaSize = totalSize;

		gameState->IsInitialized = true;
	}

	cbArena renderArena;
	InitArena(&renderArena, renderCommands->BufferSize, renderCommands->BufferBase);

    Update();
    Render(deltaTime, gameMemory, &renderArena);
}