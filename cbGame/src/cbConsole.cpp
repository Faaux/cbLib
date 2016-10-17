#include <cbInclude.h>
#include <cbBasic.h>
#include <cbConsole.h>
#include <cbGame.h>
#include "imgui.h"
#include <vector>

void AddLog(cbConsole* console, const char* fmt, ...)
{
	char buf[256];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, ArrayCount(buf), fmt, args);
	buf[ArrayCount(buf) - 1] = 0;
	va_end(args);
	ZeroSize(CONSOLE_LENGTH, console->Items[console->CurrentItem % CONSOLE_SIZE]);
	cbStrCopy(console->Items[console->CurrentItem++ % CONSOLE_SIZE], buf);
	if(console->ItemCount < CONSOLE_SIZE)
		console->ItemCount++;

	static bool HadWrap = false;
	if (HadWrap)
		console->FirstItem = (console->FirstItem + 1) % CONSOLE_SIZE;
	
	if (console->CurrentItem >= CONSOLE_SIZE)
	{
		console->CurrentItem %= CONSOLE_SIZE;
		HadWrap = true;
	}
	
	console->ScrollToBottom = true;
}

internal void Clear(cbConsole *console)
{
	console->CurrentItem = 0;
	console->FirstItem = 0;
	console->ItemCount = 0;
	console->ScrollToBottom = true;
	ZeroSize(ArrayCount(console->Items), console->Items);
}

internal void Rebuild(cbConsole *console)
{
	char cmdline[] = "cmd.exe /K \"cd .. & del build.txt & build.bat small>> build.txt\"";
	Platform.RunExternalProgram(cmdline, [](const char * log) { AddLog(Console, "%s\n", log); });
}

internal cbConsoleCommand _consoleCommands[] =
{
	{"build", &Rebuild},
	{"clear", &Clear}
};



internal void ExecCommand(cbConsole *console, const char* command)
{
	AddLog(console, "# %s\n", command);

	for (uint32 i = 0; i < ArrayCount(_consoleCommands); i++)
	{
		if (cbStrCmp(command, _consoleCommands[i].Command) == 0)
		{
			_consoleCommands[i].Execute(console);
			return;
		}
	}

	AddLog(console, "[error] Unknown command: '%s'\n", command);
}

internal void AddImguiConsole(cbConsole *console)
{
	if (!console->IsVisible)
		return;
	ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiSetCond_FirstUseEver);
	if (!ImGui::Begin("Console"))
	{
		ImGui::End();
		return;
	}

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
	static ImGuiTextFilter filter;
	filter.Draw("Filter (\"incl,-excl\") (\"error\")", 180);
	ImGui::PopStyleVar();
	ImGui::Separator();

	ImGui::BeginChild("ScrollingRegion", ImVec2(0, -ImGui::GetItemsLineHeightWithSpacing()), false, ImGuiWindowFlags_HorizontalScrollbar);
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing

		for (uint32 i = 0, index = console->FirstItem; i < console->ItemCount; i++, index=(index + 1) % CONSOLE_SIZE)
		{
			const char* item = console->Items[index];
			if (!filter.PassFilter(item))
				continue;
			ImVec4 col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // A better implementation may store a type per-item. For the sample let's just parse the text.
			if (strstr(item, "[error]")) col = ImColor(1.0f, 0.4f, 0.4f, 1.0f);
			else if (strncmp(item, "# ", 2) == 0) col = ImColor(1.0f, 0.78f, 0.58f, 1.0f);
			ImGui::PushStyleColor(ImGuiCol_Text, col);
			ImGui::TextUnformatted(item);
			ImGui::PopStyleColor();
		}
		if (console->ScrollToBottom)
			ImGui::SetScrollHere();
		console->ScrollToBottom = false;
		ImGui::PopStyleVar();
	}
	ImGui::EndChild();
	ImGui::Separator();

	if (ImGui::InputText("Input", console->InputBuf, ArrayCount(console->InputBuf), ImGuiInputTextFlags_EnterReturnsTrue))
	{
		char* input_end = console->InputBuf + strlen(console->InputBuf);
		while (input_end > console->InputBuf && input_end[-1] == ' ') input_end--; *input_end = 0;
		if (console->InputBuf[0])
			ExecCommand(console, console->InputBuf);
		strcpy(console->InputBuf, "");
	}

	if (ImGui::IsItemHovered() || (ImGui::IsRootWindowOrAnyChildFocused() && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0)))
		ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget

	ImGui::End();
}