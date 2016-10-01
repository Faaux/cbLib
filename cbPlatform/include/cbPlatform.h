#pragma once
#include <cbInclude.h>

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
	win32_get_window_width *GetWindowWidth;
	win32_get_window_height *GetWindowHeight;
};