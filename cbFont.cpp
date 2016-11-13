#include "cbFont.h"
#include "cbInclude.h"
#include "cbGame.h"
#include "cbRenderGroup.h"

#include <GL/glew.h>
#include <GLM.h>
#include <cstdio>

struct SDFFontData
{
	int LineBase;
	int LineHeight;
	int Width, Height;
	int Size;
	int Padding[4];
};

struct SDFGlyphData
{
	float TexX, TexY;
	float TexWidth, TexHeight;
	int Width, Height;
	int XOffset, YOffset;
	int XAdvance;
	bool IsValid;
};

static int _desiredPadding = 8;
static SDFFontData sdfFontData;
static SDFGlyphData sdfGlyphData[256];

static GLuint texId;
static GLuint quadVAO;
static GLuint quadVBO;
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

                                             "const float solid = 0.5;\n"

                                             "void main()\n"
                                             "{\n"
                                             "	float distance = texture2D(text, TexCoords).a;\n"
                                             "	float width = fwidth(distance);\n"
                                             "	float alpha = smoothstep(solid - width, solid + width, distance);\n"
                                             "	color = vec4(textColor, alpha);\n"
                                             "};\n";

internal void InitVaoVbo()
{
#if 0
    glGenVertexArrays(1, &quadVAO);
    glBindVertexArray(quadVAO);

    glGenBuffers(1, &quadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    static float quad[] = {0,   0, 0, 1, 512, 0,   1, 1, 0, 512, 0, 0,

                           512, 0, 1, 1, 512, 512, 1, 0, 0, 512, 0, 0};
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid *)0);
    glEnableVertexAttribArray(0);
#endif
}

internal void InitShader()
{
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

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

internal void InitTexture(char *fileName)
{
    int width, height;
    uint8 *atlas = Platform.cbLoadImage(fileName, width, height);

    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, atlas);
    // can free temp_bitmap at this point
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	Platform.cbFreeImage(atlas);
}

internal void LoadSDFMetaData()
{
	FILE *fid;
	errno_t err = fopen_s(&fid, "Segoe.fnt", "r");
	if (err != 0)
	{
		return;
	}
	char line[256];
	while (!feof(fid))
	{		
		fgets(line, sizeof(line), fid);
		int index,x,y,width,height,xoffset,yoffset,xadvance;
		if (sscanf_s(line, "info face=\"Segoe UI Bold\" size=%i bold=0 italic=0 charset=\"\" unicode=0 stretchH=100 smooth=1 aa=1 padding=%i,%i,%i,%i spacing=0,0", &sdfFontData.Size, &sdfFontData.Padding[0], &sdfFontData.Padding[1], &sdfFontData.Padding[2], &sdfFontData.Padding[3]))
		{

		}
		else if (sscanf_s(line, "common lineHeight=%i base=%i scaleW=%i scaleH=%i", &sdfFontData.LineHeight, &sdfFontData.LineBase, &sdfFontData.Width, &sdfFontData.Height))
		{			
			
		}
		else if (sscanf_s(line, "char id=%i x=%i y=%i width=%i height=%i xoffset=%i yoffset=%i xadvance=%i", &index,&x,&y,&width,&height,&xoffset,&yoffset,&xadvance))
		{
			int padWidth = (sdfFontData.Padding[1] + sdfFontData.Padding[3]);
			int padHeight = (sdfFontData.Padding[0] + sdfFontData.Padding[2]);
			sdfGlyphData[index].TexX = (x + (sdfFontData.Padding[1] - _desiredPadding)) / (float)sdfFontData.Width;
			sdfGlyphData[index].TexY = (y + (sdfFontData.Padding[0] - _desiredPadding)) / (float)sdfFontData.Height;
			sdfGlyphData[index].TexWidth = width / (float)sdfFontData.Width;
			sdfGlyphData[index].TexHeight = height / (float)sdfFontData.Height;
			sdfGlyphData[index].Width = width - (padWidth - (2 * _desiredPadding));
			sdfGlyphData[index].Height = height - (padHeight - (2 * _desiredPadding));
			sdfGlyphData[index].XOffset = xoffset + (sdfFontData.Padding[1] - _desiredPadding);
			sdfGlyphData[index].YOffset = yoffset + (sdfFontData.Padding[0] - _desiredPadding);
			sdfGlyphData[index].XAdvance = xadvance - padWidth;
			sdfGlyphData[index].IsValid = true;
		}
	}

	fclose(fid);
}

internal void InitFont()
{
	static bool wasInit = false;
	if(!wasInit)
	{
		wasInit = true;
		InitVaoVbo();
		InitShader();
	}
}

internal void InitSDF()
{
	LoadSDFMetaData();
	InitTexture("Segoe.png");
	InitFont();
}

void DrawString(RenderStringData *data)
{
	static bool wasInit = false;
	if (!wasInit)
	{
		wasInit = true;
		InitSDF();
	}

	float winWidth = (float)Platform.GetWindowWidth();
	float winHeight = (float)Platform.GetWindowHeight();

	float scale = (float)data->Size / (float)sdfFontData.Size;

	float posX = (float)data->X;
	float posY = winHeight - data->Y;

	char* text = &data->Text[0];
	uint32 verticeCount = 6 * data->CurrentLength;
	float *vertices = (float *)malloc(sizeof(float) * verticeCount * 4);
	float *firstVertice = vertices;
	while (*text)
	{
		char toPrint = *text++;

		Assert(toPrint >= 0 && toPrint <= 255);
		Assert(sdfGlyphData[toPrint].IsValid);				
			
		float glyphHeightScaled = sdfGlyphData[toPrint].Height * scale;
		float glyphWidthScaled = sdfGlyphData[toPrint].Width * scale;

		float texLeft = (float)sdfGlyphData[toPrint].TexX;
		float texTop = (float)sdfGlyphData[toPrint].TexY;
		float texWidth = sdfGlyphData[toPrint].TexWidth;
		float texHeight = sdfGlyphData[toPrint].TexHeight;

		float internalPosX = posX + (sdfGlyphData[toPrint].XOffset * scale);
		float internalPosY = posY - (sdfGlyphData[toPrint].YOffset * scale);

		posX += sdfGlyphData[toPrint].XAdvance * scale;

		// Top Left
		*vertices++ = internalPosX;
		*vertices++ = internalPosY;
		*vertices++ = texLeft;
		*vertices++ = texTop;

		// Bottom Left
		*vertices++ = internalPosX;
		*vertices++ = internalPosY - glyphHeightScaled;
		*vertices++ = texLeft;
		*vertices++ = texTop + texHeight;

		// Bottom Right
		*vertices++ = internalPosX + glyphWidthScaled;
		*vertices++ = internalPosY - glyphHeightScaled;
		*vertices++ = (texLeft + texWidth);
		*vertices++ = (texTop + texHeight);

		// Top Left
		*vertices++ = internalPosX;
		*vertices++ = internalPosY;
		*vertices++ = texLeft;
		*vertices++ = texTop;

		// Bottom Right
		*vertices++ = internalPosX + glyphWidthScaled;
		*vertices++ = internalPosY - glyphHeightScaled;
		*vertices++ = (texLeft + texWidth);
		*vertices++ = (texTop + texHeight);

		// Top Right
		*vertices++ = internalPosX + glyphWidthScaled;
		*vertices++ = internalPosY;
		*vertices++ = (texLeft + texWidth);
		*vertices++ = texTop;
	}

	glm::mat4 projection = glm::ortho(0.0f, winWidth, 0.0f, winHeight);

	glUseProgram(fontShaderId);

	GLuint projLoc = glGetUniformLocation(fontShaderId, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);

	GLuint colorLoc = glGetUniformLocation(fontShaderId, "textColor");
	glUniform3f(colorLoc, 0.f, 0.f, 0.f);


	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texId);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glGenVertexArrays(1, &quadVAO);
	glBindVertexArray(quadVAO);

	glGenBuffers(1, &quadVBO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * verticeCount * 4, firstVertice, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid *)0);
	glEnableVertexAttribArray(0);


	glDrawArrays(GL_TRIANGLES, 0, verticeCount);
	glBindVertexArray(0);

	glDeleteVertexArrays(1, &quadVAO);
	glDeleteBuffers(1, &quadVBO);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	free(firstVertice);
}

internal void FreeFont()
{
	glDeleteVertexArrays(1, &quadVAO);
	glDeleteBuffers(1, &quadVBO);
	glDeleteProgram(fontShaderId);
	glDeleteTextures(1, &texId);
}