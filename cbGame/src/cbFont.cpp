#include <GL/glew.h>
#include <cbInclude.h>

static float quad[] = {
	-0.5, -0.5, 0,
	0.5,  -0.5, 0,
	-0.5,  0.5, 0,

	0.5,  -0.5, 0,
	0.5, 0.5, 0,
	-0.5,  0.5, 0,
};

static const GLfloat g_vertex_buffer_data[] = {
	-1.0f, -1.0f, 0.0f,
	1.0f, -1.0f, 0.0f,
	0.0f,  1.0f, 0.0f,
};

static GLuint quadVAO;
static GLuint quadVBO;
static GLuint vertexShaderId;
static GLuint fragmentShaderId; 
static GLuint fontShaderId;


static const char * basicFontVertexShader = "#version 330 core\n"
											"layout(location = 0) in vec3 position;\n"
											"void main()\n"
											"{\n"
											"	gl_Position.xyz = position;\n"
											"	gl_Position.w = 1.0;\n"
											"};\n";

static const char * basicFontFragmentShader =	"#version 330 core\n"
												"out vec3 color;\n"
												"void main()\n"
												"{\n"
												"	color = vec3(1, 0, 0);\n"
												"};\n";

internal void InitVaoVbo()
{
	glGenVertexArrays(1, &quadVAO);
	glBindVertexArray(quadVAO);

	glGenBuffers(1, &quadVBO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
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

	glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &result);
	glGetShaderiv(vertexShaderId, GL_INFO_LOG_LENGTH, &infoLogLength);
	if (infoLogLength > 0) {
		char text[1024];
		glGetShaderInfoLog(vertexShaderId, infoLogLength, NULL, &text[0]);
	}

	glShaderSource(fragmentShaderId, 1, &basicFontFragmentShader, NULL);
	glCompileShader(fragmentShaderId);

	glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &result);
	glGetShaderiv(fragmentShaderId, GL_INFO_LOG_LENGTH, &infoLogLength);
	if (infoLogLength > 0) {
		char text[1024];
		glGetShaderInfoLog(fragmentShaderId, infoLogLength, NULL, &text[0]);
	}


	fontShaderId = glCreateProgram();
	glAttachShader(fontShaderId, vertexShaderId);
	glAttachShader(fontShaderId, fragmentShaderId);
	glLinkProgram(fontShaderId);

	// Check the program
	glGetProgramiv(fontShaderId, GL_LINK_STATUS, &result);
	glGetProgramiv(fontShaderId, GL_INFO_LOG_LENGTH, &infoLogLength);
	if (infoLogLength > 0) {
		char text[1024];
		glGetProgramInfoLog(fontShaderId, infoLogLength, NULL, &text[0]);
	}


	glDetachShader(fontShaderId, vertexShaderId);
	glDetachShader(fontShaderId, fragmentShaderId);

	glDeleteShader(vertexShaderId);
	glDeleteShader(fragmentShaderId);

}

internal void InitFont()
{
	InitVaoVbo();
	InitShader();
}

internal void DrawString(char *text)
{	
	static bool wasInit = false;
	if (!wasInit)
	{
		wasInit = true;
		InitFont();
	}

	glUseProgram(fontShaderId);
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

