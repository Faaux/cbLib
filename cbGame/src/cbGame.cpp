#include "cbFont.cpp"
#include <GL/glew.h>
#include <cbGame.h>
#include <cbInclude.h>

Win32PlatformCode PlatformCode;

internal void Render()
{
    glClearColor(0.2f, 0.4f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    DrawString("Das ist ein komischer griechischer text :){}", 0.75f, 150,150);

    PlatformCode.SwapBuffer();
}

internal void Update()
{
}

EXPORT GAME_LOOP(GameLoop)
{
    Update();
    Render();
}

EXPORT GAME_INIT(GameInit)
{
    PlatformCode = platformCode;
}