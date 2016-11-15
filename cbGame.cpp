#include "cbGame.h"
#include "cbDebug.h"
#include "cbInclude.h"
#include "cbKeys.h"
#include "cbRenderGroup.h"
#include "cbFont.h"
#include "cbShader.h"
#include "GLM.h"
#include "imgui.h"
#include "cbImgui.h"
#include <GL/glew.h>
#include "imgui_internal.h"

Win32PlatformCode Platform;
TransientStorage *TransStorage;
cbConsole *Console;
debug_table *GlobalDebugTable;

static GLuint bgShaderId;

internal void ProcessRenderCommands(GameState* gameState, RenderCommandGroup* renderCommands)
{
	TIMED_FUNCTION();
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
	TIMED_FUNCTION();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#if 1
	static bool isInit = false;
	static GLuint bgQuadVAO;
	static GLuint bgQuadVBO;
	static cbShaderProgram* program;
	if (!isInit)
	{
		isInit = true;
		program = cbCreateProgram("shaders\\noise.v", "shaders\\noise.f");

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

	cbUseProgram(program);

	static glm::mat4 projection = glm::ortho(0.0f, 1.f, 0.0f, 1.f);
	GLuint projLoc = cbGetUniformLocation(program, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);

	static float accum = 0, f = 8.f;
	//ImGui::SliderFloat("Background Speed", &f, 0.0f, 100.0f);
	accum += deltaTime * f;

	GLuint resLoc = cbGetUniformLocation(program, "resolution");
	glUniform1i(resLoc, 5);

	GLuint timeLoc = cbGetUniformLocation(program, "time");
	glUniform1f(timeLoc, accum);

	GLuint scaleLoc = cbGetUniformLocation(program, "scale");
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

struct event_result
{
	debug_event *Start;
	debug_event *End;
	uint32 Level;
	uint32 Index;
};

internal void EvaluateDebugInfo()
{
	ImGui::SetNextWindowPos(ImVec2(10, 10));
	static float winAlpha = .7f;
	if (!ImGui::Begin("Function Timings", (bool *)1, ImVec2(0, 0), winAlpha, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
	{
		ImGui::End();
		return;
	}
	float frameTime = 1000.0f / ImGui::GetIO().Framerate;

	// Framerate Graph
	const int recordHistory = 200;
	static int currentHistory = 0;
	static float* fpsHistory = (float *)malloc(recordHistory * sizeof(float));

	fpsHistory[currentHistory] = frameTime;
	currentHistory = (currentHistory + 1) % recordHistory;

	ImGui::PlotLines("##MsPlot", fpsHistory, recordHistory, currentHistory, nullptr, 0.0f, 1.0f, ImVec2(0, 80));
	ImGui::SameLine();
	ImGui::Text("%s\n%-3.4f\n\n%s\n%-3.4f", "ms/frame", frameTime, "fps", ImGui::GetIO().Framerate);

	ImGui::Spacing();
	ImGui::SliderFloat("Transparency", &winAlpha, 0.0f, 1.0f);
	if (GlobalDebugTable->CurrentIndex == GlobalDebugTable->NextIndex)
	{
		ImGui::End();
		return;
	}

	// Profiling
	ImGui::Spacing();

	uint32 startIndex = GlobalDebugTable->CurrentIndex;
	debug_event *start = &GlobalDebugTable->Events[startIndex];
	Assert(start->Type == FrameStart);


#define SetCurrentToNextElement() GlobalDebugTable->CurrentIndex = (GlobalDebugTable->CurrentIndex + 1) & 0xFFFF; \
				current = &GlobalDebugTable->Events[GlobalDebugTable->CurrentIndex]
	debug_event *current;
	SetCurrentToNextElement();

	// Check how many event there are for this frame
	uint32 length = 0;
	while (current->GUID && current->Type != FrameEnd)
	{
		SetCurrentToNextElement();
		length++;
	}

	Assert(length % 2 == 0); // Else Begin/End mismatch

	// Setup buffers
	uint32 size = length / 2;
	event_result *eventStack = (event_result *)malloc(size * sizeof(event_result));
	event_result *eventList = (event_result *)malloc(size * sizeof(event_result));
	ZeroSize(size * sizeof(event_result), eventList);
	ZeroSize(size * sizeof(event_result), eventStack);

	// Reset Current
	GlobalDebugTable->CurrentIndex = startIndex;
	SetCurrentToNextElement();

	// Extract Information
	uint32 currentLevel = 0;
	uint32 eventIndex = 0;
	for (uint32 i = 0; i < length; i++)
	{
		switch (current->Type)
		{
		case BeginBlock:
		{
			// New Block
			event_result *slot = eventStack + currentLevel++;
			slot->Start = current;
			slot->Index = eventIndex++;
			break;
		}
		case EndBlock:
		{
			currentLevel--;

			event_result *eventResult = eventStack + currentLevel;
			eventResult->End = current;
			eventResult->Level = currentLevel;

			// Put into final list
			event_result *listElement = eventList + eventResult->Index;
			*listElement = *eventResult;
			break;
		}
		default:
			Assert(false);
			break;
		}
		SetCurrentToNextElement();
	}
#undef SetCurrentToNextElement
	free(eventStack);


	// Build Tree
	uint64 totalCycles = __rdtsc() - start->Clock;
	for (uint32 i = 0; i < size; i++)
	{
		event_result *curResult = &eventList[i];
		float percentage = (curResult->End->Clock - curResult->Start->Clock) / (float)totalCycles;
		char* name = cbGetLastPosOf('|', curResult->Start->GUID) + 1;
		if(ImGui::TreeNode(curResult->Start->GUID, "%s  %-2.6f%%",name, percentage))
			ImGui::TreePop();
	}
	
	ImGui::End();

	free(eventList);
	
}

EXPORT GAME_LOOP(GameLoop)
{
	GlobalDebugTable = gameMemory->GlobalDebugTable;
	TIMED_FUNCTION();
	GameState* gameState = (GameState*)gameMemory->PermanentStorage;
	if (!gameState->IsInitialized)
	{
		cbArena totalArena;
		mem_size totalSize = gameMemory->PermanentStorageSize - sizeof(GameState);
		InitArena(&totalArena, totalSize, (uint8 *)gameMemory->PermanentStorage + sizeof(GameState));
		gameState->Arena = totalArena;
		gameState->ArenaSize = totalSize;
		gameState->IsInitialized = true;

		gameState->Console = PushStruct(&gameState->Arena, cbConsole);
	}

	Console = gameState->Console;
	Platform = gameMemory->Platform;


	TransStorage = (TransientStorage *)gameMemory->TransientStorage;
	if (gameMemory->DLLHotSwapped)
	{
		uint8* currentMemoryLocation = (uint8 *)gameMemory->TransientStorage + sizeof(TransientStorage);

		cbArena transRenderArena;
		mem_size renderCommandSize = Megabytes(1);
		InitArena(&transRenderArena, renderCommandSize, currentMemoryLocation);
		TransStorage->RenderGroupArena = transRenderArena;
		PushSize(&TransStorage->RenderGroupArena, renderCommandSize);

		currentMemoryLocation += renderCommandSize;

		cbArena shaderArena;
		mem_size shaderArenaSize = Kilobytes(256);
		InitArena(&shaderArena, shaderArenaSize, currentMemoryLocation);
		TransStorage->ShaderArena = shaderArena;
		currentMemoryLocation += shaderArenaSize;

		InitImGui();

		TransStorage->IsInitialized = true;
	}
	if (!input->OldKeyboardInput.Keys[cbKey_OEM_5].IsDown && input->NewKeyboardInput.Keys[cbKey_OEM_5].IsDown)
	{
		gameState->Console->IsVisible = !gameState->Console->IsVisible;
	}
	UpdateImgui(deltaTime, input);
	Update();

	// Start Imgui Frame
	ImGui::NewFrame();
	ImGui::ShowTestWindow();
	AddImguiConsole(gameState->Console);

	RenderCommandGroup renderCommands = RenderCommandStruct(TransStorage->RenderGroupArena.Size, TransStorage->RenderGroupArena.Base, Platform.GetWindowWidth(), Platform.GetWindowHeight());
	Render(deltaTime, gameState, &renderCommands);

	// Evaluate Timing Info and render with imgui
	BEGIN_BLOCK("Level1");
	BEGIN_BLOCK("Level2");
	BEGIN_BLOCK("Level3");
	END_BLOCK();
	END_BLOCK();
	END_BLOCK();
	EvaluateDebugInfo();

	// End Imgui Frame
	BEGIN_BLOCK("Imgui Render");
	ImGui::Render();
	END_BLOCK();
}