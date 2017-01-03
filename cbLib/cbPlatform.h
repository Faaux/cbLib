#pragma once
#include "cbInclude.h"
#include "cbDebug.h"

struct cbFiletime
{
	uint32 LowPart;
	uint32 HighPart;
};
#define RUN_EXTERNAL_PROGRAM(name) void name(char* command, void(*Output)(const char*))
typedef RUN_EXTERNAL_PROGRAM(win32_run_external_program);
RUN_EXTERNAL_PROGRAM(Win32RunExternalProgram);

#define COMPARE_FILE_TIME(name) int name(const cbFiletime lhs, const cbFiletime rhs)
typedef COMPARE_FILE_TIME(win32_cmp_file_time);
COMPARE_FILE_TIME(Win32CompareFileTime);

#define GET_LAST_FILE_TIME(name) cbFiletime name(const char* file)
typedef GET_LAST_FILE_TIME(win32_get_last_file_time);
GET_LAST_FILE_TIME(Win32GetLastFileTime);

#define SET_CLIPBOARD_TEXT(name) void name(const char* text)
typedef SET_CLIPBOARD_TEXT(win32_set_clipboard_text);
SET_CLIPBOARD_TEXT(Win32SetClipboardText);

#define GET_CLIPBOARD_TEXT(name) const char* name()
typedef GET_CLIPBOARD_TEXT(win32_get_clipboard_text);
GET_CLIPBOARD_TEXT(Win32GetClipboardText);

#define FREE_IMAGE(name) void name(unsigned char *image)
typedef FREE_IMAGE(win32_free_image);
FREE_IMAGE(Win32FreeImage);

#define LOAD_IMAGE(name) unsigned char *name(const char *path, int &width, int &height)
typedef LOAD_IMAGE(win32_load_image);
LOAD_IMAGE(Win32LoadImage);

#define READ_TEXT_FILE(name) char *name(const char *path, long &size)
typedef READ_TEXT_FILE(win32_read_text_file);
READ_TEXT_FILE(Win32ReadTextFile);

#define READ_FILE(name) void *name(const char *path, long &size)
typedef READ_FILE(win32_read_file);
READ_FILE(Win32ReadFile);

#define FREE_FILE(name) void name(void* memory)
typedef FREE_FILE(win32_free_file);
FREE_FILE(Win32FreeFile);

#define SWAP_BUFFER(name) void name()
typedef SWAP_BUFFER(win32_swap_buffer);
SWAP_BUFFER(Win32SwapBuffer);

#define GET_WIN_SIZE(name) uint32 name()
typedef GET_WIN_SIZE(win32_get_window_width);
GET_WIN_SIZE(GetWindowWidth);

typedef GET_WIN_SIZE(win32_get_window_height);
GET_WIN_SIZE(GetWindowHeight);

struct Win32PlatformCode
{
	win32_run_external_program *RunExternalProgram;
	win32_cmp_file_time *CompareFileTime;
	win32_get_last_file_time *GetLastFileTime;
	win32_set_clipboard_text *SetClipboardText;
	win32_get_clipboard_text *GetClipboardText;
	win32_swap_buffer *SwapBuffer;
	win32_read_text_file *cbReadTextFile;
	win32_read_file *cbReadFile;
	win32_free_file *cbFreeFile;
	win32_load_image *cbLoadImage;
	win32_free_image *cbFreeImage;
	win32_get_window_width *GetWindowWidth;
	win32_get_window_height *GetWindowHeight;
};

struct GameMemory
{
	mem_size PermanentStorageSize;
	void *PermanentStorage;

	mem_size TransientStorageSize;
	void *TransientStorage;

	Win32PlatformCode Platform;
	debug_table *GlobalDebugTable;

	bool DLLHotSwapped;
};

struct GameMouseInput
{
	uint32 X, Y;
	float WheelSteps;
	bool MouseButtons[3];
	
};

struct Key
{
	bool IsDown;
};

struct GameKeyboardInput
{
	Key Keys[256];
	uint32 CurrentLength;
	char InputText[32];
};

struct GameInput
{
	GameMouseInput NewMouseInputState;
	GameMouseInput OldMouseInputState;
	GameKeyboardInput OldKeyboardInput;
	GameKeyboardInput NewKeyboardInput;

	bool ShiftDown;
	bool AltDown;
	bool ControlDown;
};

#define PRESSED(input, key) input->OldKeyboardInput.Keys[key].IsDown && input->NewKeyboardInput.Keys[key].IsDown
#define SINGLE_PRESS(input, key) !input->OldKeyboardInput.Keys[key].IsDown && input->NewKeyboardInput.Keys[key].IsDown

#define GAME_LOOP(name) void name(float deltaTime, GameMemory* gameMemory, GameInput* input)
typedef GAME_LOOP(game_loop);
inline GAME_LOOP(GameLoopStub)
{
}
