#pragma once
#include <cbInclude.h>
#include <cbPlatform.h>
#include <cbRenderGroup.h>

struct cbConsoleEntry
{
	char Text[256];
};

struct cbConsole
{
	bool IsVisible;
	uint32 LineHeight;
	uint32 CurrentEntryIndex;
	cbConsoleEntry Entries[32];
};

inline void RenderConsole(RenderCommandGroup* renderCommands, cbConsole* console)
{
	console->IsVisible = true;
	console->LineHeight = 14;

	int currentHeight = 5;
	uint32 currentIndex = console->CurrentEntryIndex;
	uint32 current = 0;
	for (;;)
	{
		if (console->Entries[currentIndex].Text[0])
		{
			PushRenderString(renderCommands, console->LineHeight, 5, currentHeight, console->Entries[currentIndex].Text);
			currentHeight += (int)(console->LineHeight);
		}

		currentIndex = (currentIndex + 1) % ArrayCount(console->Entries);
		if (current++ >= ArrayCount(console->Entries) - 1)
			break;
	}
}

inline void AddStringToConsole(cbConsole *console, char *text)
{
	cbConsoleEntry *currentEntry = &console->Entries[console->CurrentEntryIndex];

	int safeguard = ArrayCount(currentEntry->Text);
	int counter = 0;
	char *current = &currentEntry->Text[0];
	while (*text)
	{
		Assert(counter++ < safeguard - 1);
		*current++ = *text++;
	}
	*current = 0;
	++console->CurrentEntryIndex;
	if (console->CurrentEntryIndex >= ArrayCount(console->Entries))
	{
		console->CurrentEntryIndex = 0;
	}
}