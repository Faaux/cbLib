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
#include "cbModel.h"

#include <GL/glew.h>



Win32PlatformCode Platform;
TransientStorage *TransStorage;
cbConsole *Console;
debug_table *GlobalDebugTable;


cbInternal void ProcessRenderCommands(GameState* gameState, RenderCommandGroup* renderCommands)
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

cbInternal void Render(float deltaTime, GameState* gameState, RenderCommandGroup* renderCommands)
{
	TIMED_FUNCTION();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#if 0
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
#if 1
	static cbModel *model = cbLoadModel("res\\models\\sub\\model\\model.dae");
	static cbShaderProgram* program = cbCreateProgram("shaders\\pbr.v", "shaders\\pbr.f");


	static glm::mat4 modelMatrix(1.f);
	static glm::mat4 viewMatrix;
	static float aspectRatio = (float)Platform.GetWindowWidth() / (float)Platform.GetWindowHeight();
	static glm::mat4 projectionMatrix = glm::perspective(
		glm::radians(60.0f),
		aspectRatio,
		0.1f,
		10000.0f
	);

	static glm::vec3 camPos(1, 3, 1);
	static float rotInDegree = 0;
	static float distance = 3.f;
	static float stepSize = 1;

	ImGui::DragFloat("StepSize", &stepSize);
	ImGui::DragFloat("Height", &camPos.y, stepSize);
	ImGui::DragFloat("Rotation", &rotInDegree, 1.0f, 0.f, 360.f);
	ImGui::DragFloat("Distance", &distance, stepSize);

	float radians = glm::radians(rotInDegree);
	camPos.x = cos(radians) * distance;
	camPos.z = sin(radians) * distance;

	ImGui::Text("X: %f", camPos.x);
	ImGui::Text("Z: %f", camPos.z);


	viewMatrix = glm::lookAt(
		camPos,
		glm::vec3(0, 0.3f, 0),
		glm::vec3(0, 1, 0)
	);

	cbUseProgram(program);

	static glm::vec3 lightPos(0, 1, 0);
	static float accum = 0;
	accum += deltaTime;
	if (accum > 2 * Pi)
		accum -= 2 * Pi;
	lightPos.x = cos(accum) * 4;
	lightPos.z = sin(accum) * 4;

	GLuint lightLoc = cbGetUniformLocation(program, "lightPos");
	glUniform3f(lightLoc, lightPos.x, lightPos.y, lightPos.z);

	GLuint modelLoc = cbGetUniformLocation(program, "modelMatrix");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &modelMatrix[0][0]);

	GLuint viewLoc = cbGetUniformLocation(program, "viewMatrix");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &viewMatrix[0][0]);

	GLuint projLoc = cbGetUniformLocation(program, "projectionMatrix");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projectionMatrix[0][0]);

	cbRenderModel(program, model);


#endif

	ProcessRenderCommands(gameState, renderCommands);
}

cbInternal void Update()
{

}

cbInternal event_result *ExtractLastFrameInformation(uint64 &cycles, uint32 &size)
{
	TIMED_FUNCTION();

	uint64 startIndex = GlobalDebugTable->CurrentIndex;
	debug_event *start = &GlobalDebugTable->Events[startIndex];
	Assert(start->Type == FrameStart);


#define SetCurrentToNextElement() GlobalDebugTable->CurrentIndex = (GlobalDebugTable->CurrentIndex + 1) & 0xFFFF; \
				current = &GlobalDebugTable->Events[GlobalDebugTable->CurrentIndex]
	debug_event *current;
	SetCurrentToNextElement();

	// Check how many event there are for this frame
	uint32 length = 0;
	while (current->Type != FrameEnd)
	{
		Assert(current->GUID);
		SetCurrentToNextElement();
		length++;
	}

	cycles = current->Clock - start->Clock;

	Assert(length % 2 == 0); // Else Begin/End mismatch

							 // Setup buffers
	size = length / 2;
	event_result *eventStack = (event_result *)malloc(size * sizeof(event_result));
	static event_result *eventList = 0;
	if (eventList)
		free(eventList);
	eventList = (event_result *)malloc(size * sizeof(event_result));
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
			slot->GUID = current->GUID;
			slot->ElapsedCylces = current->Clock;
			slot->Index = eventIndex++;
			break;
		}
		case EndBlock:
		{
			currentLevel--;

			event_result *eventResult = eventStack + currentLevel;
			eventResult->ElapsedCylces = current->Clock - eventResult->ElapsedCylces;
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
	return eventList;
}

cbInternal cbQuicksortCompare(CompareByClock)
{
	uint64 lhClock = ((event_result *)lhs)->ElapsedCylces;
	uint64 rhClock = ((event_result *)rhs)->ElapsedCylces;

	if (lhClock < rhClock)
		return 1;
	return lhClock == rhClock ? 0 : -1;
}

cbInternal cbSwap(SwapEventResult)
{
	event_result tmp = *(event_result *)lhs;
	*(event_result *)lhs = *(event_result *)rhs;
	*(event_result *)rhs = tmp;
}

cbInternal void EvaluateDebugInfo()
{
	TIMED_FUNCTION();

	ImGui::SetNextWindowPos(ImVec2(10, 10));
	static float winAlpha = .7f;
	if (!ImGui::Begin("Information", 0, ImVec2(340, ImGui::GetIO().DisplaySize.y - 20), winAlpha, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
	{
		ImGui::End();
		return;
	}
	float frameTime = 1000.0f / ImGui::GetIO().Framerate;

	// Framerate Graph
	const int recordHistory = 200;
	static int currentHistory = 0;
	static float* fpsHistory = (float *)malloc(recordHistory * sizeof(float));
	static int ticker = 0;
	static float frameTimeBuffer = 0;
	frameTimeBuffer += frameTime;
	if (ticker++ % 5 == 0)
	{
		fpsHistory[currentHistory] = frameTimeBuffer / 5.f;
		currentHistory = (currentHistory + 1) % recordHistory;
		ticker = 0;
		frameTimeBuffer = 0.f;
	}

	ImGui::PlotLines("##MsPlot", fpsHistory, recordHistory, currentHistory, 0, FLT_MAX, FLT_MAX, ImVec2(0, 80));
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

	// Profiling	
	static bool didOnce = false;
	if (!didOnce)
	{
		didOnce = true;
		ImGui::SetNextTreeNodeOpen(true);
	}
	if (ImGui::CollapsingHeader("Profiling"))
	{
		uint64 cycles;
		uint32 size;
		event_result *eventList = ExtractLastFrameInformation(cycles, size);

		static int e = 0;
		ImGui::RadioButton("Sort by %", &e, 0); ImGui::SameLine();
		ImGui::RadioButton("Sort by Order", &e, 1);

		static bool showFilename = false;
		static bool showMsEstimate = true;
		ImGui::Checkbox("Show ms", &showMsEstimate); ImGui::SameLine();
		ImGui::Checkbox("Show Filename", &showFilename);

		if (e == 0)
		{
			// Resort by %
			cbQuicksort(eventList, size, sizeof(event_result), CompareByClock, SwapEventResult);
		}




		float perc_w = showMsEstimate ? 130.f : 90.0f;
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		ImGui::BeginChild("p_Perc", ImVec2(perc_w, 0), true, ImGuiWindowFlags_NoScrollbar);
		{
			ImGui::Text("Percentage %%");
			if (showMsEstimate)
			{
				ImGui::SameLine();
				ImGui::Text(" | ms");
			}
			ImGui::Separator();
		}
		ImGui::EndChild();
		ImGui::SameLine();
		ImGui::InvisibleButton("vsplitter_1", ImVec2(8.0f, 0));

		ImGui::SameLine();
		ImGui::BeginChild("p_Name", ImVec2(0, 0), true, showFilename ? ImGuiWindowFlags_AlwaysHorizontalScrollbar : 0);
		{
			ImGui::Text("Name");
			ImGui::Separator();
		}
		ImGui::EndChild();

		ImGui::PopStyleVar();

		ImGui::BeginChild("p_Perc");
		ImGui::Spacing();
		ImGui::EndChild();

		// #2 Column
		ImGui::BeginChild("p_Name");
		ImGui::Spacing();
		ImGui::EndChild();

		for (uint32 index = 0; index < size; index++)
		{
			event_result *curResult = &eventList[index];

			float percentage = curResult->ElapsedCylces / (float)cycles * 100;

			char* cursor = cbGetLastPosOf('\\', curResult->GUID) + 1;

			char fileName[20] = {};
			char *fileNameP = fileName;
			int fileNameLength = 0;
			while (*cursor != '|')
			{
				Assert(fileNameLength++ < ArrayCount(fileName));
				*fileNameP++ = *cursor++;
			}
			*fileNameP = 0;
			cursor++;

			char line[7] = {};
			char *lineP = line;
			int lineLength = 0;
			while (*cursor != '|')
			{
				Assert(lineLength++ < ArrayCount(line));
				*lineP++ = *cursor++;
			}
			*lineP = 0;
			cursor++;

			// Skip Cursor Pos
			while (*cursor++ != '|') {}

			// #1 Column
			ImGui::BeginChild("p_Perc");
			ImGui::Text("%-2.3f%%", percentage);
			if (showMsEstimate)
			{
				ImGui::SameLine(60);
				ImGui::Text("|");
				ImGui::SameLine(70);
				ImGui::Text("%-2.3f", percentage / 100.f * frameTime);

			}
			ImGui::EndChild();

			// #2 Column
			ImGui::BeginChild("p_Name");
			ImGui::Text("%s", cursor);
			if (showFilename)
			{
				ImGui::SameLine(165);
				ImGui::Text(" | %s [%s]", fileName, line);
			}
			ImGui::EndChild();



		}
		ImGui::Columns(1);

		ImGui::BeginChild("p_Name");
		float scroll = ImGui::GetScrollY();
		ImGui::EndChild();
		ImGui::BeginChild("p_Perc");
		ImGui::SetScrollY(scroll);
		ImGui::EndChild();
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

		cbArena modelArena;
		mem_size modelArenaSize = Kilobytes(256);
		InitArena(&modelArena, modelArenaSize, currentMemoryLocation);
		TransStorage->ModelArena = modelArena;
		cbInitModelTable(&TransStorage->ModelArena);
		currentMemoryLocation += modelArenaSize;

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
	//ImGui::ShowTestWindow();
	AddImguiConsole(gameState->Console);

	RenderCommandGroup renderCommands = RenderCommandStruct(TransStorage->RenderGroupArena.Size, TransStorage->RenderGroupArena.Base, Platform.GetWindowWidth(), Platform.GetWindowHeight());
	Render(deltaTime, gameState, &renderCommands);

	EvaluateDebugInfo();

	// End Imgui Frame
	BEGIN_BLOCK("Imgui Render");
	ImGui::Render();
	END_BLOCK();
}