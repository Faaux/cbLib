#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#include "Windowsx.h"

#include <GL/glew.h>
#include <GL/wglew.h>

#include <cbGame.h>
#include <cbInclude.h>
#include <cbPlatform.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include "cbBasic.h"

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
internal HWND _hWindow;
internal HDC _deviceContext;
internal LARGE_INTEGER _lastCounter;
internal uint64 _perfCountFrequency;
internal float _deltaTime;
internal float _currentFps;
internal bool _windowWasResized = false;
internal uint32 _windowWidth = 1280;
internal uint32 _windowHeight = 720;
internal bool _isCloseRequested;

internal GameInput _gameInput;


internal void UpdateKeyState(uint32 vkCode, bool isDown)
{
	_gameInput.NewKeyboardInput.Keys[vkCode].IsDown = isDown;		
}

internal void UpdateInputText(char c)
{
	Assert(_gameInput.NewKeyboardInput.CurrentLength < 31);
	_gameInput.NewKeyboardInput.InputText[_gameInput.NewKeyboardInput.CurrentLength++] = c;
}
internal LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT Result = 0;

    switch (message)
    {
	case WM_CHAR:
	{
		char c = (char)wParam;
		UpdateInputText(c);
		break;
	}
    case WM_SIZE:
        _windowWidth = LOWORD(lParam);
        _windowHeight = HIWORD(lParam);
        _windowWasResized = true;
        break;
    case WM_DESTROY:
    case WM_QUIT:
        _isCloseRequested = true;
        break;
	case WM_MOUSEWHEEL:
		_gameInput.NewMouseInputState.WheelSteps = (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA;
		break;
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_KEYDOWN:
	case WM_KEYUP:
	{
		uint32 VKCode = (uint32)wParam;
		//bool WasDown = ((lParam & (1 << 30)) != 0);
		bool IsDown = ((lParam & (1 << 31)) == 0);
		UpdateKeyState(VKCode, IsDown);

		bool AltKeyWasDown = (lParam & (1 << 29)) ? true : false;
		if (VKCode == VK_F4 && AltKeyWasDown)
		{
			_isCloseRequested = true;
		}
		break;
	}
	
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

    int suggestedFormatIndex = ChoosePixelFormat(_deviceContext, &DesiredFormat);
    PIXELFORMATDESCRIPTOR suggestedFormat;
    DescribePixelFormat(_deviceContext, suggestedFormatIndex, sizeof(suggestedFormat), &suggestedFormat);
    SetPixelFormat(_deviceContext, suggestedFormatIndex, &suggestedFormat);

    HGLRC tempContext = wglCreateContext(_deviceContext);
    if (wglMakeCurrent(_deviceContext, tempContext))
    {
        glewExperimental = TRUE;
        GLenum err = glewInit();
        if (err != GLEW_OK)
        {
            // Problem: glewInit failed, something is seriously wrong.
            return;
        }

        int pixel_attribs[] = {WGL_DRAW_TO_WINDOW_ARB,
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
                               0};

        // Find proper Pixel Format
        int pixelFormat;
        uint32 numFormat;
        wglChoosePixelFormatARB(_deviceContext, pixel_attribs, nullptr, 1, &pixelFormat, &numFormat);
        suggestedFormat = {};
        DescribePixelFormat(_deviceContext, pixelFormat, sizeof(suggestedFormat), &suggestedFormat);
        SetPixelFormat(_deviceContext, pixelFormat, &suggestedFormat);

        if (wglewIsSupported("WGL_ARB_create_context") == 1)
        {
            GLint attribs[] = {WGL_CONTEXT_MAJOR_VERSION_ARB, 3, WGL_CONTEXT_MINOR_VERSION_ARB, 3, 0};

            HGLRC context = wglCreateContextAttribsARB(_deviceContext, 0, attribs);
            wglMakeCurrent(NULL, NULL);
            wglDeleteContext(tempContext);
            wglMakeCurrent(_deviceContext, context);
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

    _hWindow = CreateWindowExA(0, wcex.lpszClassName, "cbDefaultWindow", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, _windowWidth,
                               _windowHeight, NULL, NULL, hInstance, NULL);
    if (!_hWindow)
    {
        return;
    }

    _deviceContext = GetDC(_hWindow);

    Win32InitOpenGL();
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

    ShowWindow(_hWindow, SW_SHOWDEFAULT);
    UpdateWindow(_hWindow);
}

internal float Win32UpdatePlatform(GameInput *input)
{
	input->OldMouseInputState = input->NewMouseInputState;
	input->OldKeyboardInput = input->NewKeyboardInput;
	input->NewKeyboardInput.CurrentLength = 0;
	input->NewMouseInputState.WheelSteps = 0.f;
	DWORD mouseButtonIds[3] =
	{
		VK_LBUTTON,
		VK_RBUTTON,
		VK_MBUTTON

	};

	POINT mousePos;
	GetCursorPos(&mousePos);
	ScreenToClient(_hWindow, &mousePos);

	input->NewMouseInputState.X = mousePos.x;
	input->NewMouseInputState.Y = mousePos.y;

	for (uint32 ButtonIndex = 0; ButtonIndex < ArrayCount(mouseButtonIds); ++ButtonIndex)
	{
		input->NewMouseInputState.MouseButtons[ButtonIndex] = GetKeyState(mouseButtonIds[ButtonIndex]) & (1 << 15) ? true : false;
	}

	input->ShiftDown = (GetKeyState(VK_SHIFT) & (1 << 15)) ? true : false;
	input->AltDown = (GetKeyState(VK_MENU) & (1 << 15)) ? true : false;
	input->ControlDown = (GetKeyState(VK_CONTROL) & (1 << 15)) ? true : false;

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

    if (_windowWasResized)
    {
        glViewport(0, 0, _windowWidth, _windowHeight);
        _windowWasResized = false;
    }

    LARGE_INTEGER endCounter;
    QueryPerformanceCounter(&endCounter);

    int64 counterElapsed = endCounter.QuadPart - _lastCounter.QuadPart;
    _deltaTime = (float)counterElapsed / (float)_perfCountFrequency;
    _currentFps = 0.2f * ((float)_perfCountFrequency / (float)counterElapsed) + 0.8f * _currentFps;

    _lastCounter = endCounter;

	static bool isFirst = false;
	if(!isFirst)
	{
		isFirst = true;
		_deltaTime = 1.f / 60.f;
	}
    return _deltaTime;
}

SWAP_BUFFER(Win32SwapBuffer)
{
    SwapBuffers(_deviceContext);
}

SET_CLIPBOARD_TEXT(Win32SetClipboardText)
{
	const size_t len = strlen(text) + 1;
	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);
	memcpy(GlobalLock(hMem), text, len);
	GlobalUnlock(hMem);
	OpenClipboard(0);
	EmptyClipboard();
	SetClipboardData(CF_TEXT, hMem);
	CloseClipboard();
}

GET_CLIPBOARD_TEXT(Win32GetClipboardText)
{
	for (;;)
		if (OpenClipboard(0))
			break;

	HANDLE hData = GetClipboardData(CF_TEXT);
	char *text= (char*)(GlobalLock(hData));
	GlobalUnlock(hData);
	CloseClipboard();
	return text;
}

READ_TEXT_FILE(Win32ReadTextFile)
{
	FILE *f;
	errno_t err = fopen_s(&f, path, "rb");
	if (err != 0)
	{
		return nullptr;
	}
	// Go to end
	fseek(f, 0, SEEK_END);

	// Read file size
	size = ftell(f);

	// Readwind to start
	rewind(f);

	// Read memory
	void *memory = malloc(size + 1);
	fread(memory, size, 1, f);

	*((char *)memory + size) = 0;
	++size;

	// close stream
	fclose(f);
	return memory;
}

READ_FILE(Win32ReadFile)
{
    FILE *f;
    errno_t err = fopen_s(&f, path, "rb");
    if (err != 0)
    {
        return nullptr;
    }
    // Go to end
    fseek(f, 0, SEEK_END);

    // Read file size
    size = ftell(f);

    // Readwind to start
    rewind(f);

    // Read memory
    void *memory = malloc(size);
    fread(memory, size, 1, f);

    // close stream
    fclose(f);
    return memory;
}

FREE_FILE(Win32FreeFile)
{
	free(memory);
}

LOAD_IMAGE(Win32LoadImage)
{
    int n;
    unsigned char *data = stbi_load(path, &width, &height, &n, 4);
    return data;
}

FREE_IMAGE(Win32FreeImage)
{
    stbi_image_free(image);
}

GET_WIN_SIZE(GetWindowHeight)
{
    return _windowHeight;
}

GET_WIN_SIZE(GetWindowWidth)
{
    return _windowWidth;
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
    // Init Game
    Win32InitWindowAndOpenGL();

	// Allocate all the memory we ever need

	const mem_size permSize = Megabytes(64);
	void *permanentMem = VirtualAlloc(0, permSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	Assert(permanentMem);

	const mem_size transSize = Megabytes(256);
	void *transientMem = VirtualAlloc(0, transSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	Assert(transientMem);



	Win32PlatformCode platformCode;
	platformCode.SetClipboardText = &Win32SetClipboardText;
	platformCode.GetClipboardText = &Win32GetClipboardText;
	platformCode.SwapBuffer = &Win32SwapBuffer;
	platformCode.cbReadTextFile = &Win32ReadTextFile;
	platformCode.cbReadFile = &Win32ReadFile;
	platformCode.cbFreeFile = &Win32FreeFile;
	platformCode.cbFreeImage = &Win32FreeImage;
	platformCode.cbLoadImage = &Win32LoadImage;
	platformCode.GetWindowHeight = &GetWindowHeight;
	platformCode.GetWindowWidth = &GetWindowWidth;

	GameMemory gameState;
	gameState.PermanentStorageSize = permSize;
	gameState.PermanentStorage = permanentMem;
	gameState.TransientStorageSize = transSize;
	gameState.TransientStorage = transientMem;
	gameState.Platform = platformCode;
	
	glClearColor(0.2f, 0.4f, 0.3f, 1.0f);

    Win32GameCode gameCode = Win32LoadGameCode();

	gameState.DLLHotSwapped = true;
    while (!_isCloseRequested)
    {
        FILETIME lastWriteTime = GetLastWriteTime("cbGame.dll");
        if (CompareFileTime(&lastWriteTime, &gameCode.LastWriteTime) != 0)
        {
            Win32UnloadGameCode(&gameCode);
            gameCode = Win32LoadGameCode();
			gameState.DLLHotSwapped = true;
        }
        if (!gameCode.IsValid)
            continue;

        float deltaTime = Win32UpdatePlatform(&_gameInput);

        // Early exit
        if (_isCloseRequested)
            continue;
		
        gameCode.GameLoop(deltaTime, &gameState, &_gameInput);
		
		Win32SwapBuffer();

		gameState.DLLHotSwapped = false;
    }
    return 0;
}