#include <GL/glew.h>
#include <cbGame.h>
#include <cbInclude.h>
#include <cbBasic.h>

Win32PlatformCode Platform;

internal void Render(float deltaTime, GameMemory* gameState)
{
    glClearColor(0.2f, 0.4f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	const mem_size size = 24;
	char num[size];
	char text[] = "Frame MS: ";
	const mem_size concatSize = ArrayCount(text) + ArrayCount(num) - 1;
	char concat[concatSize];
	
	static float smoothedDeltaTime = 0;
	smoothedDeltaTime = deltaTime * 0.1f + smoothedDeltaTime * 0.9f;
	cbFtoA(smoothedDeltaTime * 1000.f, num, size);

	char* toRender = cbConcatStr(concat, concatSize, text, ArrayCount(text), num, ArrayCount(num));
    //DrawString(concat, 64.f, 0, 0);

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

    Update();
    Render(deltaTime, gameMemory);
}