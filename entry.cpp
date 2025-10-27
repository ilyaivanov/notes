#define UNICODE
#define _UNICODE
#define WIN32_LEAN_AND_MEAN
#define USE_SSE2

#include "win32.cpp"
#include "drawing.cpp"
#include "app.cpp"

HWND win;
bool isRunning;

HDC windowDc;
HDC drawingDc;

HBITMAP bitmap;
BITMAPINFO bitmapInfo;
MyBitmap canvas;

LRESULT OnEvent(HWND handle, UINT message, WPARAM wParam, LPARAM lParam) {
  switch (message) {
  case WM_KEYDOWN: {
    if (wParam == 'Q') {
      PostQuitMessage(0);
      isRunning = false;
    }
    OnKeyPress(wParam);
    break;
  }
  case WM_DESTROY:
    PostQuitMessage(0);
    isRunning = false;
    break;
  case WM_PAINT: {
    PAINTSTRUCT ps;
    BeginPaint(handle, &ps);
    EndPaint(handle, &ps);
  } break;
  case WM_SIZE: {

    canvas.width = LOWORD(lParam);
    canvas.height = HIWORD(lParam);

    bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
    bitmapInfo.bmiHeader.biBitCount = 32;
    bitmapInfo.bmiHeader.biWidth = canvas.width;
    // makes rows go down, instead of going up by default
    bitmapInfo.bmiHeader.biHeight = -canvas.height;
    bitmapInfo.bmiHeader.biPlanes = 1;
    bitmapInfo.bmiHeader.biCompression = BI_RGB;

    if (bitmap)
      DeleteBitmap(bitmap);

    void* bits;
    bitmap = CreateDIBSection(drawingDc, &bitmapInfo, DIB_RGB_COLORS, &bits, 0, 0);
    canvas.pixels = (u32*)bits;
  }
  }
  return DefWindowProc(handle, message, wParam, lParam);
}

extern "C" void WinMainCRTStartup() {
  SetProcessDPIAware();
  win = OpenWindow(OnEvent);
  ShowWindow(win, SW_SHOW);

  windowDc = GetDC(win);
  drawingDc = CreateCompatibleDC(0);

  isRunning = true;
  currentBitmap = &canvas;
  currentDc = drawingDc;
  SelectBitmap(drawingDc, bitmap);
  Init();
  while (isRunning) {
    MSG msg;

    while (PeekMessageW(&msg, 0, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    memset(canvas.pixels, 0, canvas.width * canvas.height * 4);
    Draw();
    StretchDIBits(windowDc, 0, 0, canvas.width, canvas.height, 0, 0, canvas.width, canvas.height,
                  canvas.pixels, &bitmapInfo, DIB_RGB_COLORS, SRCCOPY);
  }

  ExitProcess(0);
}
