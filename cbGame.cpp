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

	ImGui::Spacing();
	const uint32 MAX = 0xFFFFFFFF;

	debug_event *current = &GlobalDebugTable->Events[GlobalDebugTable->CurrentIndex];
	Assert(current->Type == FrameStart);

	uint64 totalCylces = __rdtsc() - current->Clock;

	GlobalDebugTable->CurrentIndex = (GlobalDebugTable->CurrentIndex + 1) & MAX;
	current = &GlobalDebugTable->Events[GlobalDebugTable->CurrentIndex];

	debug_event *eventStack[32] = {};
	bool treeStateStack[32] = {};
	uint32 currentIndex = 0;

	while (current->GUID && current->Type != FrameEnd)
	{
		Assert(currentIndex < ArrayCount(eventStack));
		switch (current->Type)
		{
		case BeginBlock:
		{
			eventStack[currentIndex] = current;
			char *toPrint = cbGetLastPosOf('|', current->GUID) + 1;

			bool isOpen = true;
			for (uint32 i = 0; i < currentIndex; i++)
				isOpen = isOpen && treeStateStack[i];

			if (isOpen)
				treeStateStack[currentIndex] = ImGui::TreeNode(toPrint);
			currentIndex++;
			break;
		}
		case EndBlock:
		{
			currentIndex--;
			bool isOpen = true;
			for (uint32 i = 0; i <= currentIndex; i++)
				isOpen = isOpen && treeStateStack[i];

			if (isOpen)
			{
				debug_event *startEvent = eventStack[currentIndex];
				float percentage = (float)(current->Clock - startEvent->Clock) / (float)totalCylces * 100.f;
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%f%%", percentage);

				ImGui::TreePop();
			}

			break;
		}
		default:
			Assert(false);
			break;
		}

		GlobalDebugTable->CurrentIndex = (GlobalDebugTable->CurrentIndex + 1) & MAX;
		current = &GlobalDebugTable->Events[GlobalDebugTable->CurrentIndex];

	}
	ImGui::End();
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

	EvaluateDebugInfo();

	// End Imgui Frame
	BEGIN_BLOCK("Imgui Render");
	ImGui::Render();
	END_BLOCK();
}