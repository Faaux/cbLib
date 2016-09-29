#include <GL/glew.h>
#include <cbGame.h>
#include <cbInclude.h>

internal void Render(Win32PlatformCode platformCode)
{
    glClearColor(.2f, 0.4f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    platformCode.SwapBuffer();
}

internal void Update()
{
}

EXPORT GAME_LOOP(GameLoop)
{
    Update();
    Render(platformCode);
}