#include <cbGame.h>
#include <cbInclude.h>
#include <cbBasic.h>
#include <cbRenderGroup.h>

Win32PlatformCode Platform;

internal void Render(float deltaTime, GameState* state, RenderCommandGroup* renderCommands)
{
	const mem_size size = 24;
	char num[size];
	char text[] = "Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet cl Frame ms: ";
	const mem_size concatSize = ArrayCount(text) + ArrayCount(num) - 1;
	char concat[concatSize];
	
	static float smoothedDeltaTime = 0;
	smoothedDeltaTime = deltaTime * 0.1f + smoothedDeltaTime * 0.9f;
	cbFtoA(smoothedDeltaTime * 1000.f, num, size);
	char* toRender = cbConcatStr(concat, concatSize, num, ArrayCount(num), text, ArrayCount(text));
	PushRenderString(renderCommands, 24, 10, Platform.GetWindowHeight() - 30, toRender);

	AddStringToConsole(state->Console, toRender);
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

		gameState->Console = PushStruct(&totalArena, cbConsole);
		gameState->Console->IsVisible = true;
	}

    Update();
    Render(deltaTime, gameState, renderCommands);
}