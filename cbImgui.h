#pragma once
#include "imgui.h"
#include "cbGame.h"
void ImGuiRender(ImDrawData* draw_data);
void UpdateImgui(float deltaTime, GameInput *input);
void InitImGui();