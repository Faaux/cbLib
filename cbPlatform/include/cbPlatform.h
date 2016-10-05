#pragma once
#include <cbInclude.h>
#include <cbMemory.h>

#define FREE_IMAGE(name) void name(unsigned char *image)
typedef FREE_IMAGE(win32_free_image);
FREE_IMAGE(Win32FreeImage);

#define LOAD_IMAGE(name) unsigned char *name(const char *path, int &width, int &height)
typedef LOAD_IMAGE(win32_load_image);
LOAD_IMAGE(Win32LoadImage);

#define READ_FILE(name) void *name(const char *path, long &size)
typedef READ_FILE(win32_read_file);
READ_FILE(Win32ReadFile);

#define FREE_FILE(name) void name(void* memory)
typedef FREE_FILE(win32_free_file);
FREE_FILE(Win32ReadFile);

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
	win32_swap_buffer *SwapBuffer;
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
};

