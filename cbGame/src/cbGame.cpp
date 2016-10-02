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
    DrawString(concat, .5f, 10, 10);

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