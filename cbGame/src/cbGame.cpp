#include <GL/glew.h>
#include <cbGame.h>
#include <cbInclude.h>

#ifdef __cplusplus
extern "C" {
#endif
int _fltused = 0;
#ifdef __cplusplus
}
#endif

internal void Render()
{
    glClearColor(.2f, 0.4f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

internal void Update()
{
}

EXPORT GAME_LOOP(GameLoop)
{
    Update();
    Render();
}