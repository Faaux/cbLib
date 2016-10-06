#include <cbGame.h>
#include <cbInclude.h>
#include <cbBasic.h>
#include <cbRenderGroup.h>

Win32PlatformCode Platform;

internal void Render(float deltaTime, GameMemory* memory, RenderCommandGroup* renderCommands)
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
	PushRenderString(renderCommands, 64, 10, 10, concatSize, toRender);
	PushRenderString(renderCommands, 64, 10, 64 * 1, concatSize, toRender);
	PushRenderString(renderCommands, 64, 10, 64 * 2, concatSize, toRender);
	PushRenderString(renderCommands, 64, 10, 64 * 3, concatSize, toRender);
	PushRenderString(renderCommands, 64, 10, 64 * 4, concatSize, toRender);
	PushRenderString(renderCommands, 64, 10, 64 * 5, concatSize, toRender);
	PushRenderString(renderCommands, 64, 10, 64 * 6, concatSize, toRender);
	PushRenderString(renderCommands, 64, 10, 64 * 7, concatSize, toRender);

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
    Render(deltaTime, gameMemory, renderCommands);
}