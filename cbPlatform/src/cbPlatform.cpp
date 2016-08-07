#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#include <Strsafe.h>

#include <GL/glew.h>
#include <GL/wglew.h>

#include <cbGame.h>
#include <cbInclude.h>

#ifdef __cplusplus
extern "C" {
#endif
	int _fltused = 0;
#ifdef __cplusplus
}
#endif

#ifdef UNICODE
typedef LPWSTR LPTSTR;
#else
typedef LPSTR LPTSTR;
#endif

struct Win32GameCode
{
	HMODULE GameCodeDLL;
	FILETIME LastWriteTime;

	game_loop *GameLoop;
	bool IsValid;
};

internal HDC DeviceContext;
internal LARGE_INTEGER _lastCounter;
internal uint64 _perfCountFrequency;
internal float _deltaTime;
internal float _currentFps;
internal bool _isCloseRequested;

internal LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT Result = 0;

	switch (message)
	{
	case WM_DESTROY:
	case WM_QUIT:
		_isCloseRequested = true;
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return Result;

}

internal void Win32InitOpenGL()
{
	PIXELFORMATDESCRIPTOR DesiredFormat = {};
	DesiredFormat.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	DesiredFormat.nVersion = 1;
	DesiredFormat.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
	DesiredFormat.cColorBits = 32;
	DesiredFormat.cAlphaBits = 8;
	DesiredFormat.cDepthBits = 32;
	DesiredFormat.iLayerType = PFD_MAIN_PLANE;
	DesiredFormat.iPixelType = PFD_TYPE_RGBA;

	int suggestedFormatIndex = ChoosePixelFormat(DeviceContext, &DesiredFormat);
	PIXELFORMATDESCRIPTOR suggestedFormat;
	DescribePixelFormat(DeviceContext, suggestedFormatIndex, sizeof(suggestedFormat), &suggestedFormat);
	SetPixelFormat(DeviceContext, suggestedFormatIndex, &suggestedFormat);

	HGLRC tempContext = wglCreateContext(DeviceContext);
	if (wglMakeCurrent(DeviceContext, tempContext))
	{
		glewExperimental = TRUE;
		GLenum err = glewInit();
		if (err != GLEW_OK)
		{
			// Problem: glewInit failed, something is seriously wrong.
			return;
		}

		int pixel_attribs[] = { WGL_DRAW_TO_WINDOW_ARB,
							   GL_TRUE,
							   WGL_SUPPORT_OPENGL_ARB,
							   GL_TRUE,
							   WGL_DOUBLE_BUFFER_ARB,
							   GL_TRUE,
							   WGL_PIXEL_TYPE_ARB,
							   WGL_TYPE_RGBA_ARB,
							   WGL_COLOR_BITS_ARB,
							   32,
							   WGL_DEPTH_BITS_ARB,
							   24,
							   WGL_STENCIL_BITS_ARB,
							   8,
							   WGL_SAMPLE_BUFFERS_ARB,
							   1,    // Number of buffers (must be 1 at time of writing)
							   WGL_SAMPLES_ARB,
							   8,    // Number of samples
							   0 };

		// Find proper Pixel Format
		int pixelFormat;
		uint32 numFormat;
		wglChoosePixelFormatARB(DeviceContext, pixel_attribs, nullptr, 1, &pixelFormat, &numFormat);
		suggestedFormat = {};
		DescribePixelFormat(DeviceContext, pixelFormat, sizeof(suggestedFormat), &suggestedFormat);
		SetPixelFormat(DeviceContext, pixelFormat, &suggestedFormat);

		if (wglewIsSupported("WGL_ARB_create_context") == 1)
		{
			GLint attribs[] = { WGL_CONTEXT_MAJOR_VERSION_ARB, 3, WGL_CONTEXT_MINOR_VERSION_ARB, 3, 0 };

			HGLRC context = wglCreateContextAttribsARB(DeviceContext, 0, attribs);
			wglMakeCurrent(NULL, NULL);
			wglDeleteContext(tempContext);
			wglMakeCurrent(DeviceContext, context);
		}

		if (wglewIsSupported("WGL_EXT_swap_control") == 1)
		{
			wglSwapIntervalEXT(0);
		}
		// Enable depth test
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glEnable(GL_MULTISAMPLE);
		glEnable(GL_CULL_FACE);
		glShadeModel(GL_SMOOTH);
		int OpenGLVersion[2];
		glGetIntegerv(GL_MAJOR_VERSION, &OpenGLVersion[0]);
		glGetIntegerv(GL_MINOR_VERSION, &OpenGLVersion[1]);

		// LogInfoLine("OpenGL Version:  %i.%i", OpenGLVersion[0], OpenGLVersion[1]);
	}
	else
	{
		// ToDo: Invalid!
	}
}

internal void Win32InitWindowAndOpenGL()
{
	LARGE_INTEGER perfCount;
	QueryPerformanceFrequency(&perfCount);
	_perfCountFrequency = (uint64)perfCount.QuadPart;

	HINSTANCE hInstance = GetModuleHandle(NULL);
	WNDCLASSEXA wcex;
	wcex.cbSize = sizeof(WNDCLASSEXA);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = NULL;    // Can be null, we handle drawing ourselves
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = "cbGameWinodw";
	wcex.hIconSm = NULL;

	if (!RegisterClassExA(&wcex))
	{
		return;
	}

	HWND hWindow = CreateWindowExA(0, wcex.lpszClassName, "cbDefaultWindow", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720, NULL,
		NULL, hInstance, NULL);
	if (!hWindow)
	{
		return;
	}

	DeviceContext = GetDC(hWindow);

	Win32InitOpenGL();
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

	ShowWindow(hWindow, SW_SHOWDEFAULT);
	UpdateWindow(hWindow);
}

internal float Win32UpdatePlatform()
{
	MSG Message;
	while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
	{
		if (Message.message == WM_QUIT)
		{
			_isCloseRequested = true;
		}

		TranslateMessage(&Message);
		DispatchMessageA(&Message);
	}

	LARGE_INTEGER endCounter;
	QueryPerformanceCounter(&endCounter);

	int64 counterElapsed = endCounter.QuadPart - _lastCounter.QuadPart;
	_deltaTime = (float)counterElapsed / (float)_perfCountFrequency;
	_currentFps = 0.2f * ((float)_perfCountFrequency / (float)counterElapsed) + 0.8f * _currentFps;

	_lastCounter = endCounter;

	return _deltaTime;
}

internal void Win32SwapBuffer()
{
	SwapBuffers(DeviceContext);
}

internal FILETIME GetLastWriteTime(char *fileName)
{
	FILETIME result = {};

	WIN32_FIND_DATAA findData;
	HANDLE findHandle = FindFirstFileA(fileName, &findData);
	if (findHandle != INVALID_HANDLE_VALUE)
	{
		result = findData.ftLastWriteTime;
		FindClose(findHandle);
	}

	return result;
}

internal Win32GameCode Win32LoadGameCode()
{
	Win32GameCode result = {};

	char *sourceDllName = "cbGame.dll";
	char *tempDllName = "cbGame_temp.dll";

	result.LastWriteTime = GetLastWriteTime(sourceDllName);

	CopyFileA(sourceDllName, tempDllName, FALSE);
	result.GameCodeDLL = LoadLibraryA(tempDllName);
	result.IsValid = false;

	if (result.GameCodeDLL)
	{
		game_loop *loop = (game_loop *)GetProcAddress(result.GameCodeDLL, "GameLoop");
		if (loop)
		{
			result.GameLoop = loop;
			result.IsValid = true;
		}
		else
		{
			// TODO: LogError
		}
	}
	else
	{
		// ToDo: LogError
	}

	if (!result.IsValid)
	{
		result.GameLoop = GameLoopStub;
	}

	return result;
}

internal void Win32UnloadGameCode(Win32GameCode *gameCode)
{
	if (gameCode->GameCodeDLL)
	{
		if (!FreeLibrary(gameCode->GameCodeDLL))
		{
			auto error = GetLastError();
		}
	}

	gameCode->IsValid = false;
	gameCode->GameCodeDLL = 0;
	gameCode->GameLoop = GameLoopStub;
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	char pathToExe[MAX_PATH] = {};
	DWORD ret = GetModuleFileNameA(NULL, pathToExe, sizeof(pathToExe));

	if (ret == 0 || ret == sizeof(pathToExe))
		FatalExit(-1);

	// Init Game
	Win32InitWindowAndOpenGL();

	Win32GameCode code = Win32LoadGameCode();
	while (!_isCloseRequested)
	{
		FILETIME lastWriteTime = GetLastWriteTime("cbGame.dll");
		if (CompareFileTime(&lastWriteTime, &code.LastWriteTime) != 0)
		{
			Win32UnloadGameCode(&code);
			code = Win32LoadGameCode();
		}
		if (!code.IsValid)
			continue;

		glClear(GL_COLOR_BUFFER_BIT);
		float deltaTime = Win32UpdatePlatform();

		// Early exit
		if (_isCloseRequested)
			continue;

		// Let Game make logic
		code.GameLoop(deltaTime);

		// Swap Buffer
		Win32SwapBuffer();
	}
	return 0;
}