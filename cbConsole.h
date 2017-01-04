#pragma once
#define CONSOLE_SIZE 64
#define CONSOLE_LENGTH 1024
#include "cbPlatform.h"

struct cbConsole
{
	bool IsVisible;
	char InputBuf[64];
	bool ScrollToBottom;
	uint32 FirstItem;
	uint32 CurrentItem;
	uint32 ItemCount;
	char Items[CONSOLE_SIZE][CONSOLE_LENGTH];
};

struct cbConsoleCommand
{
	char Command[16];
	void(*Execute)(cbConsole *);
};

void AddLog(cbConsole* console, const char* fmt, ...);
void AddImguiConsole(GameInput* input, cbConsole *console);