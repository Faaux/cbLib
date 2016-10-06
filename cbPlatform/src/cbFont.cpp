#include <cstdio>

#include <GL/glew.h>
#include <GLM.h>
#include <cbInclude.h>
#include <cbPlatform.h>
#include <cbRenderGroup.h>


struct SDFFontData
{
	int LineBase;
	int LineHeight;
	int Width, Height;
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
											 "uniform float smoothed;\n"
                                             "const float solid = 0.5;\n"
                                             "void main()\n"
                                             "{\n"
                                             "	float distance = texture2D(text, TexCoords).a;\n"
                                             "	float alpha = smoothstep(solid, solid + smoothed, distance);\n"
                                             "	color = vec4(textColor, alpha);\n"
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
    uint8 *atlas = Win32LoadImage(fileName, width, height);

    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, atlas);
    // can free temp_bitmap at this point
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

    Win32FreeImage(atlas);
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
		if (sscanf_s(line, "info face=\"Segoe UI Bold\" size=59 bold=0 italic=0 charset=\"\" unicode=0 stretchH=100 smooth=1 aa=1 padding=%i,%i,%i,%i spacing=0,0", &sdfFontData.Padding[0], &sdfFontData.Padding[1], &sdfFontData.Padding[2], &sdfFontData.Padding[3]))
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

internal void DrawString(RenderStringData *data)
{
	static bool wasInit = false;
	if (!wasInit)
	{
		wasInit = true;
		InitSDF();
	}

	float winWidth = (float)GetWindowWidth();
	float winHeight = (float)GetWindowHeight();

	glm::mat4 projection = glm::ortho(0.0f, winWidth, 0.0f, winHeight);

	GLuint projLoc = glGetUniformLocation(fontShaderId, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);

	GLuint colorLoc = glGetUniformLocation(fontShaderId, "textColor");
	glUniform3f(colorLoc, 0.f, 0.f, 0.f);

	float scale = (float)data->SizeInPx / sdfFontData.LineHeight;
	float smoothed;
	if(scale < 1)
	{
		smoothed = 0.1;
	}
	else
	{
		smoothed = 0.05;
	}
	GLuint smoothedLoc = glGetUniformLocation(fontShaderId, "smoothed");
	glUniform1f(smoothedLoc, smoothed);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texId);


	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUseProgram(fontShaderId);

	float posX = (float)data->X;
	float posY = winHeight - data->Y;

	char* text = &data->Text[0];
	while (*text)
	{
		char toPrint = *text;
		if (toPrint >= 0 && toPrint <= 255)
		{
			if(!sdfGlyphData[toPrint].IsValid)
				continue;

			float glyphHeightScaled = sdfGlyphData[toPrint].Height * scale;
			float glyphWidthScaled = sdfGlyphData[toPrint].Width * scale;

			float texLeft = (float)sdfGlyphData[toPrint].TexX;
			float texTop = (float)sdfGlyphData[toPrint].TexY;
			float texWidth = sdfGlyphData[toPrint].TexWidth;
			float texHeight = sdfGlyphData[toPrint].TexHeight;

			float internalPosX = posX + (sdfGlyphData[toPrint].XOffset * scale);
			float internalPosY = posY - (sdfGlyphData[toPrint].YOffset * scale);


			float vertices[] = { 
				// Top Left
				internalPosX,
				internalPosY,
				texLeft,
				texTop,

				// Bottom Left
				internalPosX,
				internalPosY - glyphHeightScaled,
				texLeft,
				texTop + texHeight,

				// Bottom Right
				internalPosX + glyphWidthScaled,
				internalPosY - glyphHeightScaled,
				(texLeft + texWidth), 
				(texTop + texHeight),

				// Top Left
				internalPosX,
				internalPosY,
				texLeft,
				texTop,

				// Bottom Right
				internalPosX + glyphWidthScaled,
				internalPosY - glyphHeightScaled,
				(texLeft + texWidth),
				(texTop + texHeight),

				// Top Right
				internalPosX + glyphWidthScaled,
				internalPosY,
				(texLeft + texWidth),
				texTop
			};

			posX += sdfGlyphData[toPrint].XAdvance * scale;
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

internal void FreeFont()
{
	glDeleteVertexArrays(1, &quadVAO);
	glDeleteBuffers(1, &quadVBO);
	glDeleteProgram(fontShaderId);
	glDeleteTextures(1, &texId);
}