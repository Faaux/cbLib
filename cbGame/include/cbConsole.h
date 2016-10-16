#pragma once
#define CONSOLE_SIZE 64
#define CONSOLE_LENGTH 1024

struct cbConsole
{
	char InputBuf[64];
	bool ScrollToBottom;
	uint32 FirstItem;
	uint32 ItemCount;
	char Items[CONSOLE_SIZE][CONSOLE_LENGTH];
};

struct cbConsoleCommand
{
	char Command[16];
	void(*Execute)(cbConsole *);
};