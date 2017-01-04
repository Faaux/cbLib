#include "cbImgui.h"
#include "cbGame.h"
#include "cbInclude.h"
#include "cbKeys.h"
#include "cbShader.h"

#include <GL/glew.h>


std::vector<Tweaker> Tweakers;

static cbShaderProgram *imguiShader;
static unsigned int g_VboHandle = 0, g_VaoHandle = 0, g_ElementsHandle = 0, g_TextureId = 0;

void ImGuiRender(ImDrawData* draw_data)
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
	Assert(cbUseProgram(imguiShader));
	glUniform1i(cbGetUniformLocation(imguiShader ,"Texture"), 0);
	glUniformMatrix4fv(cbGetUniformLocation(imguiShader, "ProjMtx"), 1, GL_FALSE, &ortho_projection[0][0]);
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

void InitImGui()
{
	ImGuiIO& io = ImGui::GetIO();

	io.Fonts->AddFontFromFileTTF("res/Roboto-Medium.ttf", 13);

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

	imguiShader = cbCreateProgram("shaders/imgui.v", "shaders/imgui.f");

	int g_AttribLocationPosition = glGetAttribLocation(imguiShader->ShaderId, "Position");
	int g_AttribLocationUV = glGetAttribLocation(imguiShader->ShaderId, "UV");
	int g_AttribLocationColor = glGetAttribLocation(imguiShader->ShaderId, "Color");

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

	glGenTextures(1, &g_TextureId);
	glBindTexture(GL_TEXTURE_2D, g_TextureId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	io.Fonts->TexID = (void *)(intptr_t)g_TextureId;

	// Restore modified GL state
	glBindTexture(GL_TEXTURE_2D, last_texture);
	glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
	glBindVertexArray(last_vertex_array);
}

void AddImguiTweakers(GameInput *input)
{
	static bool isVisible = false;

	if(SINGLE_PRESS(input, cbKey_F1))
		isVisible = !isVisible;
	
	if (!isVisible)
		return;

	ImGui::SetNextWindowSize(ImVec2(700, 300), ImGuiSetCond_Always);
	ImGui::SetNextWindowPosCenter();
	if (!ImGui::Begin("  Tweaker", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
	{
		Assert(false);
		ImGui::End();
		return;
	}

	input->IsEnabled = false;
	

	static float speed = 0.1f;
	ImGui::DragFloat("Tweaker Speed", &speed, 0.1f);
	ImGui::Spacing();
	ImGui::Spacing();

	static ImGuiTextFilter filter;
	filter.Draw("Filter (\"incl,-excl\")", 180);	
	ImGui::Spacing();

	for(auto it = Tweakers.begin(); it != Tweakers.end(); ++it)
	{
		if (!filter.PassFilter((*it).Name))
			continue;

		switch((*it).Type)
		{
		case R1: 
			ImGui::DragFloat((*it).Name, (float *)(*it).Ptr, speed);
			break;
		case R2:
			ImGui::DragFloat2((*it).Name, (float *)(*it).Ptr, speed);
			break;
		case R3:
			ImGui::DragFloat3((*it).Name, (float *)(*it).Ptr, speed);
			break;
		case R4:
			ImGui::DragFloat4((*it).Name, (float *)(*it).Ptr, speed);
			break;
		case S1:
			ImGui::DragInt((*it).Name, (int *)(*it).Ptr, speed);
			break;
		case S2:
			ImGui::DragInt2((*it).Name, (int *)(*it).Ptr, speed);
			break;
		case S3:
			ImGui::DragInt3((*it).Name, (int *)(*it).Ptr, speed);
			break;
		case S4:
			ImGui::DragInt4((*it).Name, (int *)(*it).Ptr, speed);
			break;
		default: ;
		}
	}

	ImGui::End();
}

void UpdateImgui(float deltaTime, GameInput *input)
{
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
}
