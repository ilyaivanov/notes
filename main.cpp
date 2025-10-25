#define UNICODE
#define _UNICODE
#define WIN32_LEAN_AND_MEAN
#define USE_SSE2

// later I will move glFunction and glProgram files here and will render everything vie opengl
// I want to be able to have two rendering mechanisms, would be an interesting architectural task
// #define USE_OPENGL

#include "win32.cpp"
#include "text.cpp"
#include "sincos.cpp"
#include "slider.cpp"
#include "vim.cpp"

f32 appTime;

HWND win;
HDC dc;
HDC deviceContext;

i32 isRunning = 0;
i32 isFullscreen = 0;
v2 screen;
v2 mouse;
HFONT segoe;

HBITMAP bitmap;
BITMAPINFO bitmapInfo;
MyBitmap canvas;

i32 fontSize = 15;
f32 lineHeight = 1.2;
v3 white = {1, 1, 1};
v3 black = {0, 0, 0};

f32 timeToCursorBlink = 1000;
f32 cursorBlinkStart = 0;

void SetColors(v3 foreground, v3 background) {
  foreground *= 255.0;
  background *= 255.0;
  SetBkColor(deviceContext, RGB(background.x, background.y, background.z));
  SetTextColor(deviceContext, RGB(foreground.x, foreground.y, foreground.z));
}

Buffer buffer;

f32 pagePadding = 20;

void RebuildLines() {
  SelectObject(deviceContext, bitmap);
  SelectObject(deviceContext, segoe);
  i32 wordStart = 0;
  i32 lineStart = 0;
  buffer.lines[0] = (LineBreak){.isSoft = 0, .textPos = 0};
  buffer.linesLen = 1;
  wchar_t* text = buffer.text;
  // i32 isStartingFromLineStart = 1;

  f32 maxWidth = screen.x - pagePadding * 2;

  // TODO: don't use negative index as soft line-break

  for (i32 i = 0; i < buffer.textLen; i++) {
    if (text[i] == '\n') {
      if (GetTextWidth(deviceContext, text, lineStart, i) > maxWidth) {
        buffer.lines[buffer.linesLen++] = (LineBreak){.isSoft = 1, .textPos = wordStart};
        lineStart = wordStart;
      }

      buffer.lines[buffer.linesLen++] = (LineBreak){.isSoft = 0, .textPos = i + 1};
      lineStart = i + 1;

      wordStart = i + 1;
    } else if (text[i] == ' ') {
      if (GetTextWidth(deviceContext, text, lineStart, i) > maxWidth) {
        buffer.lines[buffer.linesLen++] = (LineBreak){.isSoft = 1, .textPos = wordStart};
        lineStart = wordStart;
      }
      wordStart = i + 1;
    }
  }
}

void Init() {
  wchar_t* text =
      (wchar_t*)L"foo\nbar\nLorem ipsum dolor sit amet, consectetur adipiscing elit. Etiam "
                L"placerat, "
                L"neque a "
                L"consectetur maximus, massa ex molestie est, in ornare augue nulla nec dui. Ut "
                L"blandit sem eget tellus facilisis congue. Curabitur eget venenatis felis. "
                L"Suspendisse laoreet tempus luctus. Quisque eu tincidunt tellus. Vivamus "
                L"pellentesque est sed pharetra lacinia. Vestibulum rhoncus, enim et ultricies "
                L"luctus, lorem risus tincidunt nulla, sit amet rutrum est massa ac dolor. Quisque "
                L"id nisl vulputate, pretium odio id, blandit justo. Sed tortor erat, porttitor "
                L"vel risus ut, molestie efficitur sem. Etiam quis velit magna. Suspendisse nec "
                L"pellentesque mi. Suspendisse sodales in ex laoreet semper.\nOne fucking long "
                L"line of text\nAnother one fucking long line of "
                L"text\nOne one one one one one one one one one one one one one one one one one "
                L"one\nThree\nFive";

  i32 len = wstrlen(text);

  buffer.linesCapacity = 1024;
  buffer.lines = (LineBreak*)valloc(buffer.linesCapacity * sizeof(LineBreak));

  buffer.textCapacity = MB(2) / sizeof(wchar_t);
  buffer.text = (wchar_t*)valloc(buffer.textCapacity * sizeof(wchar_t));
  memcpy(buffer.text, text, len * sizeof(wchar_t));
  buffer.textLen = len;

  RebuildLines();
  buffer.lines[buffer.linesLen++].textPos = len;
}

void UpdateAndDraw(f32 lastFrameMs) {
  timeToCursorBlink -= lastFrameMs;

  SelectObject(deviceContext, bitmap);
  SelectObject(deviceContext, segoe);
  TEXTMETRIC textMetric;
  GetTextMetrics(deviceContext, &textMetric);
  f32 fontHeight = textMetric.tmAscent + textMetric.tmDescent;

  v2 pos = {pagePadding, pagePadding};

  f32 sin;
  f32 cos;
  SinCos((appTime - cursorBlinkStart) / 300.0f, &sin, &cos);
  f32 a = 1;
  if (timeToCursorBlink <= 0)
    a = abs(cos);

  i32 cursorWidth = 2;

  v3 grey = {0.13, 0.13, 0.13};

  v2 running = pos;

  for (i32 i = 0; i < buffer.linesLen - 1; i++) {
    i32 start = buffer.lines[i].textPos;
    i32 end = buffer.lines[i + 1].textPos;
    if (buffer.cursor >= start && buffer.cursor < end) {
      SetColors(white, grey);
      PaintRect(&canvas, 0, running.y, screen.x, fontHeight, grey);
    } else {
      SetColors(white, black);
    }

    TextOutW(deviceContext, running.x, running.y, buffer.text + start, end - start);

    if (buffer.cursor >= start && buffer.cursor < end) {
      i32 cursorX = running.x - cursorWidth / 2.0 +
                    GetTextWidth(deviceContext, buffer.text, start, buffer.cursor);
      PaintRectA(&canvas, cursorX, running.y, cursorWidth, fontHeight, 0xffffff, a);
    }

    if (buffer.lines[i + 1].isSoft)
      running.y += fontHeight;
    else
      running.y += fontHeight * lineHeight;
  }

  // cursor

  StretchDIBits(dc, 0, 0, screen.x, screen.y, 0, 0, screen.x, screen.y, canvas.pixels, &bitmapInfo,
                DIB_RGB_COLORS, SRCCOPY);
}

void OnCursorUpdated() {
  timeToCursorBlink = 600;
  cursorBlinkStart = appTime + timeToCursorBlink;
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
    if (wParam == 'L') {
      MoveRight(buffer, deviceContext);
      OnCursorUpdated();
    }
    if (wParam == 'H') {
      MoveLeft(buffer, deviceContext);
      OnCursorUpdated();
    }
    if (wParam == 'J') {
      MoveDown(buffer, deviceContext);
      OnCursorUpdated();
    }
    if (wParam == 'K') {
      MoveUp(buffer, deviceContext);
      OnCursorUpdated();
    }
    break;
  case WM_KEYUP:
    break;
  case WM_MOUSEMOVE:
    mouse.x = LOWORD(lParam);
    mouse.y = HIWORD(lParam);
    break;
  case WM_LBUTTONDOWN:
    break;
  case WM_LBUTTONUP:
    break;
  case WM_RBUTTONDOWN:
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
    canvas.width = screen.x = LOWORD(lParam);
    canvas.height = screen.y = HIWORD(lParam);

    bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
    bitmapInfo.bmiHeader.biBitCount = 32;
    bitmapInfo.bmiHeader.biWidth = screen.x;
    // makes rows go down, instead of going up by default
    bitmapInfo.bmiHeader.biHeight = -screen.y;
    bitmapInfo.bmiHeader.biPlanes = 1;
    bitmapInfo.bmiHeader.biCompression = BI_RGB;

    if (bitmap)
      DeleteBitmap(bitmap);

    void* bits;
    bitmap = CreateDIBSection(deviceContext, &bitmapInfo, DIB_RGB_COLORS, &bits, 0, 0);
    canvas.pixels = (u32*)bits;

    // if (canvas.pixels)
    //   vfree(canvas.pixels);
    //
    // canvas.pixels = (u32*)valloc(canvas.width * canvas.height * 4);
    dc = GetDC(win);

    // glViewport(0, 0, screen.x, screen.y);
    if (isRunning) {
      RebuildLines();
      UpdateAndDraw(0);
    }
    break;
  }
  return DefWindowProc(handle, message, wParam, lParam);
}

extern "C" void WinMainCRTStartup() {
  // int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {

  SetProcessDPIAware();

  win = OpenWindow(OnEvent);

  dc = GetDC(win);
  deviceContext = CreateCompatibleDC(0);

  if (isFullscreen)
    SetFullscreen(win, isFullscreen);

  ShowWindow(win, SW_SHOW);

  i64 freq = GetPerfFrequency();
  appTime = 0;
  i64 appStart = GetPerfCounter();
  i64 frameStart = appStart;

  f32 frameTime = 0;

  segoe = CreateAppFont(dc, L"Segoe UI", FW_NORMAL, fontSize);

  isRunning = 1;
  Init();
  UpdateAndDraw(0);
  while (isRunning) {
    MSG msg;

    while (PeekMessageW(&msg, 0, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    UpdateAndDraw(frameTime);
    memset(canvas.pixels, 0, canvas.width * canvas.height * 4);

    i64 frameEnd = GetPerfCounter();
    frameTime = (f32)(frameEnd - frameStart) / (f32)freq * 1000.0f;
    frameStart = frameEnd;
    appTime = (f32)(frameEnd - appStart) / (f32)freq * 1000.0f;
  }

  // return 0;
  ExitProcess(0);
}
