#pragma once

#define SWAP_BUFFER(name) void name()
typedef SWAP_BUFFER(win32_swap_buffer);
SWAP_BUFFER(Win32SwapBuffer);

struct Win32PlatformCode
{
	win32_swap_buffer *SwapBuffer;
};