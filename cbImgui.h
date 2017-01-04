#pragma once
#include "imgui.h"
#include "cbGame.h"
#include <vector>
enum cbTweaker
{
	R1,
	R2,
	R3,
	R4,
	S1,
	S2,
	S3,
	S4
};
struct Tweaker
{
	Tweaker(cbTweaker type, const char *name, void *ptr) : Type(type), Name(name), Ptr(ptr){};

	cbTweaker Type;
	const char *Name;
	void *Ptr;
};

#define RUN_ONCE(runcode) do \
{ \
    static bool code_ran = false; \
    if (!code_ran) { \
		code_ran = true; \
		runcode; \
	} \
} while (0)

extern std::vector<Tweaker> Tweakers;
#define TWEAKER(Type, Name, Ptr)	RUN_ONCE(Tweakers.emplace_back(Type, Name, Ptr);)

void ImGuiRender(ImDrawData* draw_data);
void UpdateImgui(float deltaTime, GameInput *input);
void InitImGui();
void AddImguiTweakers(GameInput *input);