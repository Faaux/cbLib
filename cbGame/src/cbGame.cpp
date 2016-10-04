#include "cbFont.cpp"
#include <GL/glew.h>
#include <cbGame.h>
#include <cbInclude.h>
#include <cbBasic.h>

Win32PlatformCode PlatformCode;

internal void Render(float deltaTime)
{
    glClearColor(0.2f, 0.4f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	char num[16];
	cbFtoA(deltaTime, num, ArrayCount(num));	
	char text[] = "Last MSg: ";

	char* concat = cbConcatStr(text, ArrayCount(text), num, ArrayCount(num));

	//static float size = 0;
	//size += deltaTime;
	//size = fmod(size, 2.f);
    DrawString(concat, 64.f, 0, 0);

	free(concat);
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
}