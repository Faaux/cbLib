#include "cbFont.cpp"
#include <GL/glew.h>
#include <cbGame.h>
#include <cbInclude.h>
#include <cbBasic.h>
#include <cbMemory.h>

Win32PlatformCode PlatformCode;
Win32Memory* GameMemory;

internal void Render(float deltaTime)
{
    glClearColor(0.2f, 0.4f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	size_t size = 24;
	char* num = PushArray(GameMemory->transientStorage, size, char);
	
	static float smoothedDeltaTime = 0;
	smoothedDeltaTime = deltaTime * 0.1f + smoothedDeltaTime * 0.9f;
	cbFtoA(smoothedDeltaTime * 1000.f, num, size);
	char text[] = "Last MSg: ";

	char* concat = cbConcatStr(GameMemory->transientStorage, text, ArrayCount(text), num, ArrayCount(num));

	//static float size = 0;
	//size += deltaTime;
	//size = fmod(size, 2.f);
    DrawString(concat, 64.f, 0, 0);

    PlatformCode.SwapBuffer();
}

internal void Update()
{
}

EXPORT GAME_LOOP(GameLoop)
{
    Update();
    Render(deltaTime);
}

EXPORT GAME_INIT(GameInit)
{
    PlatformCode = platformCode;
	GameMemory = memory;
}

EXPORT GAME_FREE(GameFree)
{
	FreeFont();
}