#define UNICODE
#define _UNICODE
#define WIN32_LEAN_AND_MEAN

// #define USE_OPENGL
#include "win32.cpp"

f32 appTime;
f32 frameTime;

HWND win;
HDC dc;

i32 isRunning = 0;
i32 isFullscreen = 0;
v2 screen;

void UpdateAndDraw(f32 lastFrameMs) {
  CharBuffer buff = {};
  Append(&buff, "Frame: ");
  AppendLine(&buff, (i32)(1000.0f / lastFrameMs));

  // SwapBuffers(dc);
}

LRESULT OnEvent(HWND handle, UINT message, WPARAM wParam, LPARAM lParam) {
  switch (message) {
  case WM_KEYDOWN:
    if (wParam == 'Q') {
      PostQuitMessage(0);
      isRunning = 0;
    }
    if (wParam == 'F') {
      isFullscreen = !isFullscreen;
      SetFullscreen(win, isFullscreen);
    }
    break;
  case WM_KEYUP:
    break;
  case WM_MOUSEMOVE:
    break;
  case WM_LBUTTONDOWN:
    break;
  case WM_RBUTTONDOWN:
    break;
  case WM_LBUTTONUP:
    break;
  case WM_RBUTTONUP:
    break;
  case WM_DESTROY:
    PostQuitMessage(0);
    isRunning = 0;
    break;
  case WM_PAINT: {
    PAINTSTRUCT ps;
    BeginPaint(handle, &ps);
    EndPaint(handle, &ps);
  } break;
  case WM_SIZE:
    screen.x = LOWORD(lParam);
    screen.y = HIWORD(lParam);
    // glViewport(0, 0, screen.x, screen.y);
    if (isRunning)
      UpdateAndDraw(0);
    break;
  }
  return DefWindowProc(handle, message, wParam, lParam);
}

extern "C" void WinMainCRTStartup() {
  SetProcessDPIAware();

  win = OpenWindow(OnEvent);

  dc = GetDC(win);

  if (isFullscreen)
    SetFullscreen(win, isFullscreen);

  ShowWindow(win, SW_SHOW);

  i64 freq = GetPerfFrequency();
  appTime = 0;
  i64 appStart = GetPerfCounter();
  i64 frameStart = appStart;

  frameTime = 0;
  isRunning = 1;

  // CreateFont(&regularFont, 14, "Consolas");

  while (isRunning) {
    MSG msg;

    while (PeekMessageW(&msg, 0, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    UpdateAndDraw(frameTime);
    i64 frameEnd = GetPerfCounter();
    frameTime = (f32)(frameEnd - frameStart) / (f32)freq * 1000.0f;
    frameStart = frameEnd;
    appTime = (f32)(frameEnd - appStart) / (f32)freq * 1000.0f;
  }

  ExitProcess(0);
}
