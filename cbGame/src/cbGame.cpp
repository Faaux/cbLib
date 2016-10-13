#include <cbGame.h>
#include <cbInclude.h>
#include <cbKeys.h>
#include <cbRenderGroup.h>
#include <GL/glew.h>

Win32PlatformCode Platform;

#include "cbFont.cpp"
#include "imgui.cpp"
#include "imgui_draw.cpp"
#include "imgui_demo.cpp"

static GLuint bgShaderId;
static int          g_ShaderHandle = 0, g_VertHandle = 0, g_FragHandle = 0;
static int          g_AttribLocationTex = 0, g_AttribLocationProjMtx = 0;
static int          g_AttribLocationPosition = 0, g_AttribLocationUV = 0, g_AttribLocationColor = 0;
static unsigned int g_VboHandle = 0, g_VaoHandle = 0, g_ElementsHandle = 0;
internal void ImGuiRender(ImDrawData* draw_data)
{
	// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
	ImGuiIO& io = ImGui::GetIO();
	int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
	int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
	if (fb_width == 0 || fb_height == 0)
		return;
	draw_data->ScaleClipRects(io.DisplayFramebufferScale);

	// Backup GL state
	GLint last_program; glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
	GLint last_texture; glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
	GLint last_active_texture; glGetIntegerv(GL_ACTIVE_TEXTURE, &last_active_texture);
	GLint last_array_buffer; glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
	GLint last_element_array_buffer; glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &last_element_array_buffer);
	GLint last_vertex_array; glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);
	GLint last_blend_src; glGetIntegerv(GL_BLEND_SRC, &last_blend_src);
	GLint last_blend_dst; glGetIntegerv(GL_BLEND_DST, &last_blend_dst);
	GLint last_blend_equation_rgb; glGetIntegerv(GL_BLEND_EQUATION_RGB, &last_blend_equation_rgb);
	GLint last_blend_equation_alpha; glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &last_blend_equation_alpha);
	GLint last_viewport[4]; glGetIntegerv(GL_VIEWPORT, last_viewport);
	GLint last_scissor_box[4]; glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
	GLboolean last_enable_blend = glIsEnabled(GL_BLEND);
	GLboolean last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
	GLboolean last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
	GLboolean last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);

	// Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_SCISSOR_TEST);
	glActiveTexture(GL_TEXTURE0);

	// Setup viewport, orthographic projection matrix
	glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
	const float ortho_projection[4][4] =
	{
		{ 2.0f / io.DisplaySize.x, 0.0f,                   0.0f, 0.0f },
		{ 0.0f,                  2.0f / -io.DisplaySize.y, 0.0f, 0.0f },
		{ 0.0f,                  0.0f,                  -1.0f, 0.0f },
		{ -1.0f,                  1.0f,                   0.0f, 1.0f },
	};
	glUseProgram(g_ShaderHandle);
	glUniform1i(g_AttribLocationTex, 0);
	glUniformMatrix4fv(g_AttribLocationProjMtx, 1, GL_FALSE, &ortho_projection[0][0]);
	glBindVertexArray(g_VaoHandle);

	for (int n = 0; n < draw_data->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = draw_data->CmdLists[n];
		const ImDrawIdx* idx_buffer_offset = 0;

		glBindBuffer(GL_ARRAY_BUFFER, g_VboHandle);
		glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)cmd_list->VtxBuffer.Size * sizeof(ImDrawVert), (GLvoid*)cmd_list->VtxBuffer.Data, GL_STREAM_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ElementsHandle);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx), (GLvoid*)cmd_list->IdxBuffer.Data, GL_STREAM_DRAW);

		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
		{
			const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
			if (pcmd->UserCallback)
			{
				pcmd->UserCallback(cmd_list, pcmd);
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
				glScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
				glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer_offset);
			}
			idx_buffer_offset += pcmd->ElemCount;
		}
	}

	// Restore modified GL state
	glUseProgram(last_program);
	glActiveTexture(last_active_texture);
	glBindTexture(GL_TEXTURE_2D, last_texture);
	glBindVertexArray(last_vertex_array);
	glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, last_element_array_buffer);
	glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
	glBlendFunc(last_blend_src, last_blend_dst);
	if (last_enable_blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
	if (last_enable_cull_face) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
	if (last_enable_depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
	if (last_enable_scissor_test) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
	glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
	glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
}

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

	static float accum = 0, f = 8.f;
	//ImGui::SliderFloat("Background Speed", &f, 0.0f, 100.0f);
	accum += deltaTime * f;
	
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
	ImGui::Render();
}

internal void Update()
{
}

internal void InitImGui()
{
	ImGuiIO& io = ImGui::GetIO();

	// Set Key Mapping
	io.KeyMap[ImGuiKey_Tab] = cbKey_TAB;
	io.KeyMap[ImGuiKey_LeftArrow] = cbKey_LEFTARROW;
	io.KeyMap[ImGuiKey_RightArrow] = cbKey_RIGHTARROW;
	io.KeyMap[ImGuiKey_UpArrow] = cbKey_UPARROW;
	io.KeyMap[ImGuiKey_DownArrow] = cbKey_DOWNARROW;
	io.KeyMap[ImGuiKey_PageUp] = cbKey_PAGEUP;
	io.KeyMap[ImGuiKey_PageDown] = cbKey_PAGEDOWN;
	io.KeyMap[ImGuiKey_Home] = cbKey_HOME;
	io.KeyMap[ImGuiKey_End] = cbKey_END;
	io.KeyMap[ImGuiKey_Delete] = cbKey_DELETE;
	io.KeyMap[ImGuiKey_Backspace] = cbKey_BACKSPACE;
	io.KeyMap[ImGuiKey_Enter] = cbKey_ENTER;
	io.KeyMap[ImGuiKey_Escape] = cbKey_ESCAPE;
	io.KeyMap[ImGuiKey_A] = cbKey_A;
	io.KeyMap[ImGuiKey_C] = cbKey_C;
	io.KeyMap[ImGuiKey_V] = cbKey_V;
	io.KeyMap[ImGuiKey_X] = cbKey_X;
	io.KeyMap[ImGuiKey_Y] = cbKey_Y;
	io.KeyMap[ImGuiKey_Z] = cbKey_Z;

	io.RenderDrawListsFn = ImGuiRender;
	io.SetClipboardTextFn = Platform.SetClipboardText;
	io.GetClipboardTextFn = Platform.GetClipboardText;


	// Backup GL state
	GLint last_texture, last_array_buffer, last_vertex_array;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);

	const GLchar *vertex_shader =
		"#version 330\n"
		"uniform mat4 ProjMtx;\n"
		"in vec2 Position;\n"
		"in vec2 UV;\n"
		"in vec4 Color;\n"
		"out vec2 Frag_UV;\n"
		"out vec4 Frag_Color;\n"
		"void main()\n"
		"{\n"
		"	Frag_UV = UV;\n"
		"	Frag_Color = Color;\n"
		"	gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
		"}\n";

	const GLchar* fragment_shader =
		"#version 330\n"
		"uniform sampler2D Texture;\n"
		"in vec2 Frag_UV;\n"
		"in vec4 Frag_Color;\n"
		"out vec4 Out_Color;\n"
		"void main()\n"
		"{\n"
		"	Out_Color = Frag_Color * texture( Texture, Frag_UV.st);\n"
		"}\n";

	g_ShaderHandle = glCreateProgram();
	g_VertHandle = glCreateShader(GL_VERTEX_SHADER);
	g_FragHandle = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(g_VertHandle, 1, &vertex_shader, 0);
	glShaderSource(g_FragHandle, 1, &fragment_shader, 0);
	glCompileShader(g_VertHandle);
	glCompileShader(g_FragHandle);
	glAttachShader(g_ShaderHandle, g_VertHandle);
	glAttachShader(g_ShaderHandle, g_FragHandle);
	glLinkProgram(g_ShaderHandle);

	g_AttribLocationTex = glGetUniformLocation(g_ShaderHandle, "Texture");
	g_AttribLocationProjMtx = glGetUniformLocation(g_ShaderHandle, "ProjMtx");
	g_AttribLocationPosition = glGetAttribLocation(g_ShaderHandle, "Position");
	g_AttribLocationUV = glGetAttribLocation(g_ShaderHandle, "UV");
	g_AttribLocationColor = glGetAttribLocation(g_ShaderHandle, "Color");

	glGenBuffers(1, &g_VboHandle);
	glGenBuffers(1, &g_ElementsHandle);

	glGenVertexArrays(1, &g_VaoHandle);
	glBindVertexArray(g_VaoHandle);
	glBindBuffer(GL_ARRAY_BUFFER, g_VboHandle);
	glEnableVertexAttribArray(g_AttribLocationPosition);
	glEnableVertexAttribArray(g_AttribLocationUV);
	glEnableVertexAttribArray(g_AttribLocationColor);

#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))
	glVertexAttribPointer(g_AttribLocationPosition, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, pos));
	glVertexAttribPointer(g_AttribLocationUV, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, uv));
	glVertexAttribPointer(g_AttribLocationColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, col));
#undef OFFSETOF


	
	unsigned char* pixels;
	int width, height;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	io.Fonts->TexID = (void *)(intptr_t)texId;

	// Restore modified GL state
	glBindTexture(GL_TEXTURE_2D, last_texture);
	glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
	glBindVertexArray(last_vertex_array);
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
	if(gameMemory->DLLHotSwapped)
	{
		cbArena transRenderArena;
		
		mem_size renderCommandSize = Megabytes(1);
		InitArena(&transRenderArena, renderCommandSize, (uint8 *)gameMemory->TransientStorage + sizeof(TransientStorage));
		transStorage->RenderGroupSize = renderCommandSize;
		transStorage->RenderGroupArena = transRenderArena;
		transStorage->IsInitialized = true;
		PushSize(&transStorage->RenderGroupArena, renderCommandSize);

		InitImGui();
	}
	
	// Set Sizes for ImGui
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize.x = (float)Platform.GetWindowWidth();
	io.DisplaySize.y = (float)Platform.GetWindowHeight();
	io.DeltaTime = deltaTime;

	// Fill Mouse Input for ImGui
	io.MousePos = ImVec2((float)input->NewMouseInputState.X, (float)input->NewMouseInputState.Y);
	io.MouseDown[0] = input->NewMouseInputState.MouseButtons[0];
	io.MouseDown[1] = input->NewMouseInputState.MouseButtons[1];
	io.MouseDown[2] = input->NewMouseInputState.MouseButtons[2];

	io.KeyCtrl = input->ControlDown;
	io.KeyShift = input->ShiftDown;
	io.KeyAlt = input->AltDown;

	io.MouseWheel = input->NewMouseInputState.WheelSteps;

	for (uint32 i = 0; i < ArrayCount(input->NewKeyboardInput.Keys); i++)
	{
		io.KeysDown[i] = input->NewKeyboardInput.Keys[i].IsDown;
	}

	for (uint32 i = 0; i < input->NewKeyboardInput.CurrentLength; i++)
	{
		io.AddInputCharacter(input->NewKeyboardInput.InputText[i]);
	}
	

	ImGui::NewFrame();

    Update();
	ImGui::ShowTestWindow();
	//ImGui::Text("Hello, world!");	
	//ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	RenderCommandGroup renderCommands = RenderCommandStruct(transStorage->RenderGroupSize, transStorage->RenderGroupArena.Base, Platform.GetWindowWidth(), Platform.GetWindowHeight());
    Render(deltaTime, gameState, &renderCommands);
}