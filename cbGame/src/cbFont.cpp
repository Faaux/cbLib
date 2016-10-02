#include <GL/glew.h>
#include <GLM.h>
#include <cbGame.h>
#include <cbInclude.h>

struct FontData
{
    int width, height;
    int cellWidth, cellHeight;
    char firstCharacter;
};

struct GlyphData
{
    char charWidth;
};

static FontData fontData;
static GlyphData glyphData[256];
static GLuint texId;
static GLuint quadVAO;
static GLuint quadVBO;
static GLuint vertexShaderId;
static GLuint fragmentShaderId;
static GLuint fontShaderId;

static const char *basicFontVertexShader = "#version 330 core\n"
                                           "layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>\n"
                                           "out vec2 TexCoords;"
                                           "uniform mat4 projection;"
                                           "void main()\n"
                                           "{\n"
                                           "	gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);\n"
                                           "	TexCoords = vertex.zw;\n"
                                           "};\n";

static const char *basicFontFragmentShader = "#version 330 core\n"
                                             "in vec2 TexCoords;\n"
                                             "out vec4 color;\n"
                                             "uniform sampler2D text;\n"
                                             "uniform vec3 textColor;\n"
                                             "void main()\n"
                                             "{\n"
                                             "	vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);\n"
                                             "	color = vec4(textColor, 1.0) * sampled;\n"
                                             "};\n";

internal void InitVaoVbo()
{
    glGenVertexArrays(1, &quadVAO);
    glBindVertexArray(quadVAO);

    glGenBuffers(1, &quadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    static float quad[] = {0,   0, 0, 1, 512, 0,   1, 1, 0, 512, 0, 0,

                           512, 0, 1, 1, 512, 512, 1, 0, 0, 512, 0, 0};
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid *)0);
    glEnableVertexAttribArray(0);
}

internal void InitShader()
{
    vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vertexShaderId, 1, &basicFontVertexShader, NULL);
    glCompileShader(vertexShaderId);

    GLint result = GL_FALSE;
    int infoLogLength;
    char text[1024];
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &result);
    glGetShaderiv(vertexShaderId, GL_INFO_LOG_LENGTH, &infoLogLength);
    if (infoLogLength > 0)
    {
        glGetShaderInfoLog(vertexShaderId, infoLogLength, NULL, &text[0]);
    }

    glShaderSource(fragmentShaderId, 1, &basicFontFragmentShader, NULL);
    glCompileShader(fragmentShaderId);

    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &result);
    glGetShaderiv(fragmentShaderId, GL_INFO_LOG_LENGTH, &infoLogLength);
    if (infoLogLength > 0)
    {
        glGetShaderInfoLog(fragmentShaderId, infoLogLength, NULL, &text[0]);
    }

    fontShaderId = glCreateProgram();
    glAttachShader(fontShaderId, vertexShaderId);
    glAttachShader(fontShaderId, fragmentShaderId);
    glLinkProgram(fontShaderId);

    // Check the program
    glGetProgramiv(fontShaderId, GL_LINK_STATUS, &result);
    glGetProgramiv(fontShaderId, GL_INFO_LOG_LENGTH, &infoLogLength);
    if (infoLogLength > 0)
    {
        glGetProgramInfoLog(fontShaderId, infoLogLength, NULL, &text[0]);
    }

    glDetachShader(fontShaderId, vertexShaderId);
    glDetachShader(fontShaderId, fragmentShaderId);

    glDeleteShader(vertexShaderId);
    glDeleteShader(fragmentShaderId);
}

internal void InitTexture()
{
    long size;
	void* memory = PlatformCode.cbReadFile("Calibri.dat", size);
	int *file = (int *)memory;
    fontData.width = *file++;
    fontData.height = *file++;
    fontData.cellWidth = *file++;
    fontData.cellHeight = *file++;

    char *fileChar = (char *)file;
    fontData.firstCharacter = *fileChar++;

    for (int i = 0; i < 256; ++i)
    {
        glyphData[i].charWidth = *fileChar++;
    }

	PlatformCode.cbFreeFile(memory);

    int width, height;
    unsigned char *atlas = PlatformCode.cbLoadImage("Calibri.png", width, height);

    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, atlas);
    // can free temp_bitmap at this point
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    PlatformCode.cbFreeImage(atlas);
}

internal void InitFont()
{
    InitVaoVbo();
    InitShader();
    InitTexture();
}

internal void DrawString(char *text, float size, int x, int y)
{
    static bool wasInit = false;
    if (!wasInit)
    {
        wasInit = true;
        InitFont();
    }

    float winWidth = (float)PlatformCode.GetWindowWidth();
    float winHeight = (float)PlatformCode.GetWindowHeight();

    glm::mat4 projection = glm::ortho(0.0f, winWidth, 0.0f, winHeight);

    GLuint projLoc = glGetUniformLocation(fontShaderId, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);

    GLuint colorLoc = glGetUniformLocation(fontShaderId, "textColor");
    glUniform3f(colorLoc, 0.f, 0.f, 0.f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texId);


	glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(fontShaderId);

    float posX = (float)x;
    float posY = winHeight - y - (fontData.cellHeight * size);

    while (*text)
    {
        char toPrint = *text;
        if (toPrint >= fontData.firstCharacter && toPrint <= 255)
        {

            int row = (toPrint - fontData.firstCharacter) / (fontData.width / fontData.cellWidth);
            int col = (toPrint - fontData.firstCharacter) % (fontData.height / fontData.cellHeight);

            float left = (float)col * fontData.cellWidth;
            float top = (float)row * fontData.cellHeight;

			float heightFactor = fontData.cellHeight * size;
			float widthFactor = fontData.cellWidth * size;

            float vertices[] = {posX,
                                posY + heightFactor,
                                left / fontData.width,
                                top / fontData.height,

                                posX,
                                posY,
                                left / fontData.width,
                                (top + fontData.cellHeight) / fontData.height,

                                posX + widthFactor,
                                posY,
                                (left + fontData.cellWidth) / fontData.width,
                                (top + fontData.cellHeight) / fontData.height,

                                posX,
                                posY + heightFactor,
                                left / fontData.width,
                                top / fontData.height,

                                posX + widthFactor,
                                posY,
                                (left + fontData.cellWidth) / fontData.width,
                                (top + fontData.cellHeight) / fontData.height,

                                posX + widthFactor,
                                posY + heightFactor,
                                (left + fontData.cellWidth) / fontData.width,
                                top / fontData.height };

            posX += glyphData[toPrint].charWidth * size;
            glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glBindVertexArray(quadVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);
        }

        ++text;
    }

	glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}
