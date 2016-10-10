#include <cbGame.h>
#include <cbInclude.h>
#include <cbBasic.h>
#include <cbRenderGroup.h>

Win32PlatformCode Platform;


struct cbConsoleEntry
{
	char Text[256];
};

struct cbConsole
{
	bool IsVisible;
	uint32 LineHeight;
	uint32 CurrentEntryIndex;
	cbConsoleEntry Entries[32];
};

internal cbConsole _console;

internal void RenderConsole(RenderCommandGroup* renderCommands)
{
	int currentHeight = 5;
	uint32 currentIndex = _console.CurrentEntryIndex;
	uint32 current = 0;
	for (;;)
	{
		if(_console.Entries[currentIndex].Text[0])
		{
			PushRenderString(renderCommands, _console.LineHeight, 5, currentHeight, ArrayCount(_console.Entries[currentIndex].Text), _console.Entries[currentIndex].Text);
			currentHeight += _console.LineHeight;
		}

		currentIndex = (currentIndex + 1) % ArrayCount(_console.Entries);
		if (current++ >= ArrayCount(_console.Entries) - 1)
			break;
	}
}

internal void AddStringToConsole(char *text)
{
	int safeguard = ArrayCount(_console.Entries[_console.CurrentEntryIndex].Text);
	int counter = 0;
	char *current = &_console.Entries[_console.CurrentEntryIndex].Text[0];
	while (*text)
	{
		Assert(counter++ < safeguard - 1);
		*current++ = *text++;
	}
	*current = 0;
	++_console.CurrentEntryIndex;
	if (_console.CurrentEntryIndex >= ArrayCount(_console.Entries))
	{
		_console.CurrentEntryIndex = 0;
	}
}

internal void Render(float deltaTime, GameMemory* memory, RenderCommandGroup* renderCommands)
{
	const mem_size size = 24;
	char num[size];
	char text[] = "Waited for another second: ";
	const mem_size concatSize = ArrayCount(text) + ArrayCount(num) - 1;
	char concat[concatSize];
	
	//static float smoothedDeltaTime = 0;
	//smoothedDeltaTime = deltaTime * 0.1f + smoothedDeltaTime * 0.9f;
	//cbFtoA(smoothedDeltaTime * 1000.f, num, size);
	//char* toRender = cbConcatStr(concat, concatSize, text, ArrayCount(text), num, ArrayCount(num));

	if(!_console.IsVisible)
	{
		_console.IsVisible = true;
		_console.LineHeight = 20;
	}

	static int tick = 0;
	static float deltaTimeBucket = 0;
	deltaTimeBucket += deltaTime;

	while(deltaTimeBucket >= 1.f)
	{
		tick = (tick + 1) % 32;
		deltaTimeBucket -= 1.f;
		cbItoA(tick, num, size);
		char* toRender = cbConcatStr(concat, concatSize, text, ArrayCount(text), num, ArrayCount(num));
		AddStringToConsole(toRender);
	}

	RenderConsole(renderCommands);
	

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