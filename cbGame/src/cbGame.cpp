#include <cbGame.h>
#include <cbInclude.h>
#include <cbBasic.h>
#include <cbRenderGroup.h>
#include <GL/glew.h>

Win32PlatformCode Platform;

#include "cbFont.cpp"

static GLuint bgShaderId;

internal void InitBackgroundShader()
{
	long size;
	char * VertexSourcePointer = (char *)Platform.cbReadTextFile("..\\cbGame\\shader\\noise.v", size);
	char * FragmentSourcePointer = (char *)Platform.cbReadTextFile("..\\cbGame\\shader\\noise.f", size);

	GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(vertexShaderId, 1, &VertexSourcePointer, NULL);
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

	glShaderSource(fragmentShaderId, 1, &FragmentSourcePointer, NULL);
	glCompileShader(fragmentShaderId);

	glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &result);
	glGetShaderiv(fragmentShaderId, GL_INFO_LOG_LENGTH, &infoLogLength);
	if (infoLogLength > 0)
	{
		glGetShaderInfoLog(fragmentShaderId, infoLogLength, NULL, &text[0]);
	}

	bgShaderId = glCreateProgram();
	glAttachShader(bgShaderId, vertexShaderId);
	glAttachShader(bgShaderId, fragmentShaderId);
	glLinkProgram(bgShaderId);

	// Check the program
	glGetProgramiv(bgShaderId, GL_LINK_STATUS, &result);
	glGetProgramiv(bgShaderId, GL_INFO_LOG_LENGTH, &infoLogLength);
	if (infoLogLength > 0)
	{
		glGetProgramInfoLog(bgShaderId, infoLogLength, NULL, &text[0]);
	}

	glDetachShader(bgShaderId, vertexShaderId);
	glDetachShader(bgShaderId, fragmentShaderId);

	glDeleteShader(vertexShaderId);
	glDeleteShader(fragmentShaderId);

	Platform.cbFreeFile(VertexSourcePointer);
	Platform.cbFreeFile(FragmentSourcePointer);
}

internal void ProcessRenderCommands(GameState* gameState, RenderCommandGroup* renderCommands)
{
	if (gameState->Console->IsVisible)
		RenderConsole(renderCommands, gameState->Console);

	uint8 * cmdPtr = (uint8*)renderCommands->BufferDataAt;
	while (cmdPtr < renderCommands->BufferBase + renderCommands->BufferSize)
	{
		RenderCommandHeader* command = (RenderCommandHeader*)cmdPtr;
		uint32 offset = sizeof(RenderCommandHeader);
		switch (command->Action)
		{
		case RenderActionType_RenderStringData:
		{
			RenderStringData *data = (RenderStringData *)((uint8 *)command + sizeof(*command));
			DrawString(data);

			offset += sizeof(RenderStringData);
			break;
		}
		default:
			Assert(false);
			break;
		}

		cmdPtr += offset;
	}
}

internal void Render(float deltaTime, GameState* gameState, RenderCommandGroup* renderCommands)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	const mem_size size = 24;
	char num[size];
	char text[] = "Frame ms: ";
	const mem_size concatSize = ArrayCount(text) + ArrayCount(num) - 1;
	char concat[concatSize];
	
	static float smoothedDeltaTime = 0;
	smoothedDeltaTime = deltaTime * 0.1f + smoothedDeltaTime * 0.9f;
	cbFtoA(smoothedDeltaTime * 1000.f, num, size);
	char* toRender = cbConcatStr(concat, concatSize, text, ArrayCount(text), num, ArrayCount(num));
	PushRenderString(renderCommands, 24, 10, Platform.GetWindowHeight() - 30, toRender);
	
	//AddStringToConsole(state->Console, toRender);
#if 1
	static bool isInit = false;
	static GLuint bgQuadVAO;
	static GLuint bgQuadVBO;
	if(!isInit)
	{
		isInit = true;
		InitBackgroundShader();

		glGenVertexArrays(1, &bgQuadVAO);
		glBindVertexArray(bgQuadVAO);

		glGenBuffers(1, &bgQuadVBO);
		glBindBuffer(GL_ARRAY_BUFFER, bgQuadVBO);
		static float quad[] = {
			0,0,
			0,1,
			1,0,

			0,1,
			1,0,
			1,1			
		};
		glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_DYNAMIC_DRAW);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid *)0);
		glEnableVertexAttribArray(0);
	}
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glUseProgram(bgShaderId);

	glm::mat4 projection = glm::ortho(0.0f, 1.f, 0.0f, 1.f);
	static GLuint projLoc = glGetUniformLocation(bgShaderId, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);

	static float accum = 0;
	accum += deltaTime * 8.f;

	static GLuint resLoc = glGetUniformLocation(bgShaderId, "resolution");
	glUniform1i(resLoc, 5);

	static GLuint timeLoc = glGetUniformLocation(bgShaderId, "time");
	glUniform1f(timeLoc, accum);

	static GLuint scaleLoc = glGetUniformLocation(bgShaderId, "scale");
	glUniform1f(scaleLoc, 0.007f);

	glBindVertexArray(bgQuadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
#endif

	ProcessRenderCommands(gameState, renderCommands);
}

internal void Update()
{
}

EXPORT GAME_LOOP(GameLoop)
{
	Platform = gameMemory->Platform;

	GameState* gameState = (GameState*)gameMemory->PermanentStorage;
	if(!gameState->IsInitialized)
	{
		cbArena totalArena;
		mem_size totalSize = gameMemory->PermanentStorageSize - sizeof(GameState);
		InitArena(&totalArena, totalSize, (uint8 *)gameMemory->PermanentStorage + sizeof(GameState));
		gameState->Arena = totalArena;
		gameState->ArenaSize = totalSize;
		gameState->IsInitialized = true;

		gameState->Console = PushStruct(&totalArena, cbConsole);
		gameState->Console->IsVisible = false;
	}

	
	TransientStorage* transStorage = (TransientStorage *)gameMemory->TransientStorage;
	if(!transStorage->IsInitialized)
	{
		cbArena transRenderArena;
		
		mem_size renderCommandSize = Megabytes(1);
		InitArena(&transRenderArena, renderCommandSize, (uint8 *)gameMemory->TransientStorage + sizeof(TransientStorage));
		transStorage->RenderGroupSize = renderCommandSize;
		transStorage->RenderGroupArena = transRenderArena;
		transStorage->IsInitialized = true;
		PushSize(&transStorage->RenderGroupArena, renderCommandSize);
	}
	
    Update();

	RenderCommandGroup renderCommands = RenderCommandStruct(transStorage->RenderGroupSize, transStorage->RenderGroupArena.Base, Platform.GetWindowWidth(), Platform.GetWindowHeight());
    Render(deltaTime, gameState, &renderCommands);
}