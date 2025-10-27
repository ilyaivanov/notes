#define UNICODE
#define _UNICODE
#define WIN32_LEAN_AND_MEAN
#define USE_SSE2

#include "win32.cpp"
#include "text.cpp"
#include "sincos.cpp"
#include "slider.cpp"
#include "vim.cpp"
#include "tests.cpp"

const c16* path = L"sample.txt";
f32 appTime;

HWND win;
HDC dc;
HDC deviceContext;

i32 isRunning = 0;
i32 isFullscreen = 0;
v2 screen;
v2 mouse;
HFONT segoe;
HFONT consolas;

HBITMAP bitmap;
BITMAPINFO bitmapInfo;
MyBitmap canvas;

enum Mode { Normal, Insert };
Mode mode;
i32 fontSize = 14;
i32 logsFontSize = 13;
f32 lineHeight = 1.15;
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

Rect textRect;
void RebuildLines() {
  SelectObject(deviceContext, bitmap);
  SelectObject(deviceContext, segoe);
  i32 wordStart = 0;
  i32 lineStart = 0;
  buffer.lines[0] = (LineBreak){.isSoft = 0, .textPos = 0};
  buffer.linesLen = 1;
  c16* text = buffer.text;
  // i32 isStartingFromLineStart = 1;

  f32 maxWidth = textRect.width - pagePadding * 2;

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

  // buffer.lines[buffer.linesLen++].textPos = buffer.textLen;
}

void Init(c16* text, i32 len) {
  buffer.linesCapacity = 1024;
  buffer.lines = (LineBreak*)valloc(buffer.linesCapacity * sizeof(LineBreak));

  buffer.textCapacity = MB(2) / sizeof(c16);
  buffer.text = (c16*)valloc(buffer.textCapacity * sizeof(c16));
  i32 bufferPos = 0;
  for (i32 i = 0; i < len; i++) {
    if (text[i] != '\r') {
      buffer.text[bufferPos] = text[i];
      bufferPos++;
    }
  }
  buffer.textLen = bufferPos;

  RebuildLines();
}

void PrintTests(Rect rect) {
  i32 padding = 5;
  i32 x = rect.x + padding;
  i32 y = rect.y + padding;

  v3 currentColor = {0.8, 0.8, 0.1};
  v3 grayColor = {0.3, 0.3, 0.3};
  v3 errorColor = {0.8, 0.2, 0.2};
  v3 okColor = {0.2, 0.8, 0.2};

  TEXTMETRIC textMetric;
  GetTextMetrics(deviceContext, &textMetric);
  f32 fontHeight = textMetric.tmAscent + textMetric.tmDescent;

  SelectFont(deviceContext, consolas);

  for (i32 i = 0; i < testLen; i++) {
    if (!tests[i].hasExecuted)
      SetColors(grayColor, black);
    else if (tests[i].error)
      SetColors(errorColor, black);
    else
      SetColors(okColor, black);

    i32 len = wstrlen(tests[i].label);
    f32 width = GetTextWidth(deviceContext, tests[i].label, 0, len);
    TextOutW(deviceContext, x, y, tests[i].label, len);
    if (tests[i].error) {
      i32 errorLen = wstrlen(tests[i].error);
      TextOutW(deviceContext, x + width + 5, y, tests[i].error, errorLen);
    }
    y += fontHeight * 0.8;
  }
}

void PrintStatus(Rect r) {
  v3 statusColor = {0.08, 0.08, 0.08};
  PaintRect(&canvas, r.x, r.y, r.width, r.height, statusColor);

  CharBuffer buff = {};
  Append(&buff, L"Offset: ");
  Append(&buff, GetTextWidth(deviceContext, buffer.text, FindLineStart(buffer), buffer.cursor));

  Append(&buff, L"  Desired: ");
  Append(&buff, buffer.desiredOffset);

  SetColors(white, statusColor);
  SelectFont(deviceContext, consolas);
  TextOutW(deviceContext, r.x + 5, r.y + 3, buff.content, buff.len);
}

f32 GetFontHeight() {
  TEXTMETRIC textMetric;
  GetTextMetrics(deviceContext, &textMetric);
  f32 fontHeight = textMetric.tmAscent + textMetric.tmDescent;
  return fontHeight;
}

v2 GetCursorPos() {
  f32 fontHeight = GetFontHeight();
  v2 pos = {textRect.x + pagePadding, textRect.y + pagePadding};
  v2 runningCursor = pos;
  v2 cursorPos = {};

  for (i32 i = 0; i < buffer.linesLen - 1; i++) {
    i32 start = buffer.lines[i].textPos;
    i32 end = (i == buffer.linesLen - 1) ? buffer.textLen : buffer.lines[i + 1].textPos;

    i32 isCursorVisibleOnLine = ((buffer.cursor >= start && buffer.cursor < end) ||
                                 (buffer.cursor == buffer.textLen && i == buffer.linesLen - 1));

    if (isCursorVisibleOnLine) {
      cursorPos.x =
          runningCursor.x + GetTextWidth(deviceContext, buffer.text, start, buffer.cursor);
      cursorPos.y = runningCursor.y;
      break;
    }

    if (buffer.lines[i + 1].isSoft)
      runningCursor.y += fontHeight;
    else
      runningCursor.y += fontHeight * lineHeight;
  }
  return cursorPos;
}

void UpdateAndDraw(f32 lastFrameMs) {
  timeToCursorBlink -= lastFrameMs;

  SelectObject(deviceContext, bitmap);
  SelectFont(deviceContext, segoe);
  TEXTMETRIC textMetric;
  GetTextMetrics(deviceContext, &textMetric);
  f32 fontHeight = textMetric.tmAscent + textMetric.tmDescent;

  f32 sin;
  f32 cos;
  SinCos((appTime - cursorBlinkStart) / 300.0f, &sin, &cos);

  textRect = {0, 0, i32(screen.x * (testsShown ? 0.7 : 1.0)), (i32)screen.y};

  f32 statusHeight = 30.0;
  textRect.height -= statusHeight;

  v2 pos = {textRect.x + pagePadding, textRect.y + pagePadding};
  RebuildLines();

  f32 a = 1;
  if (timeToCursorBlink <= 0)
    a = abs(cos);

  i32 cursorWidth = 2;

  v3 grey = {0.13, 0.13, 0.13};
  v3 cursorBgRed = {0.4, 0.1, 0.1};

  v2 running = pos;

  for (i32 i = 0; i < buffer.linesLen - 1; i++) {
    i32 start = buffer.lines[i].textPos;
    i32 end = (i == buffer.linesLen - 1) ? buffer.textLen : buffer.lines[i + 1].textPos;

    TextOutW(deviceContext, running.x, running.y, buffer.text + start, end - start);

    if (buffer.lines[i + 1].isSoft)
      running.y += fontHeight;
    else
      running.y += fontHeight * lineHeight;
  }

  f32 cursorHeight = fontHeight * 1.1;
  v2 cursorPos = GetCursorPos();

  u32 cursorColor = 0xffffff;

  if (mode == Insert)
    cursorColor = 0xff2222;

  PaintRectA(&canvas, cursorPos.x, cursorPos.y, cursorWidth, cursorHeight, cursorColor, a);

  if (testsShown) {
    Rect logRect = {textRect.x + textRect.width, textRect.y, i32(screen.x - textRect.width),
                    (i32)screen.y};
    PrintTests(logRect);
  }

  Rect statusRect = {.x = textRect.x,
                     .y = i32(textRect.y + textRect.height),
                     .width = textRect.width,
                     .height = (i32)statusHeight};

  PrintStatus(statusRect);

  StretchDIBits(dc, 0, 0, screen.x, screen.y, 0, 0, screen.x, screen.y, canvas.pixels, &bitmapInfo,
                DIB_RGB_COLORS, SRCCOPY);
}

void OnCursorUpdated() {
  timeToCursorBlink = 600;
  cursorBlinkStart = appTime + timeToCursorBlink;
}

void RemovePrevChar() {
  if (buffer.cursor > 0) {
    RemoveCharAt(buffer, buffer.cursor - 1);

    buffer.cursor--;
    OnCursorUpdated();
    RebuildLines();
    UpdateDesiredOffset(buffer, deviceContext);
  }
}

void SaveFile() {
  i32 utf8Count = WideCharToMultiByte(CP_UTF8, 0, buffer.text, buffer.textLen, 0, 0, 0, 0);

  c8* text = (c8*)valloc(utf8Count * sizeof(c8));
  WideCharToMultiByte(CP_UTF8, 0, buffer.text, buffer.textLen, text, utf8Count, 0, 0);
  WriteMyFile(path, text, utf8Count);
}

i32 ignoreNextCharEvent = 0;
LRESULT OnEvent(HWND handle, UINT message, WPARAM wParam, LPARAM lParam) {
  switch (message) {
  case WM_CHAR: {

    if (mode == Insert) {
      if (ignoreNextCharEvent)
        ignoreNextCharEvent = 0;
      else {
        if (wParam == VK_BACK)
          RemovePrevChar();
        else {
          if (wParam == '\r')
            InsertCharAt(buffer, buffer.cursor, '\n');
          else
            InsertCharAt(buffer, buffer.cursor, wParam);

          buffer.cursor++;
          RebuildLines();
          UpdateDesiredOffset(buffer, deviceContext);
        }
      }
    }
    break;
  }
  case WM_KEYDOWN:
    SelectFont(deviceContext, segoe);
    if (mode == Normal) {
      if (wParam == 'Q') {
        SaveFile();
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
      if (wParam == 'W') {
        JumpWordForward(buffer);
        OnCursorUpdated();
        UpdateDesiredOffset(buffer, deviceContext);
      }
      if (wParam == 'B') {
        JumpWordBackward(buffer);
        OnCursorUpdated();
        UpdateDesiredOffset(buffer, deviceContext);
      }
      if (wParam == 'X') {
        RemoveCharAt(buffer, buffer.cursor);

        OnCursorUpdated();
        RebuildLines();
        UpdateDesiredOffset(buffer, deviceContext);
      }
      if (wParam == 'O') {
        i32 target = 0;
        if (IsKeyPressed(VK_SHIFT))
          target = FindLineStart(buffer);
        else
          target = FindLineEnd(buffer);

        InsertCharAt(buffer, target, '\n');
        buffer.cursor = target;

        mode = Insert;
        ignoreNextCharEvent = 1;

        OnCursorUpdated();
        RebuildLines();
        UpdateDesiredOffset(buffer, deviceContext);
      }
      if (wParam == VK_BACK) {
        RemovePrevChar();
      }
      if (wParam == 'T') {
        RunTests();
      }
      if (wParam == '\r') {
        InsertCharAt(buffer, buffer.cursor, '\n');
        buffer.cursor++;

        OnCursorUpdated();
        RebuildLines();
        UpdateDesiredOffset(buffer, deviceContext);
      }

      if (wParam == 'I') {
        mode = Insert;
        ignoreNextCharEvent = 1;
      }
    } else if (mode == Insert) {
      if (wParam == VK_ESCAPE)
        mode = Normal;
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
      UpdateDesiredOffset(buffer, deviceContext);
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
  consolas = CreateAppFont(dc, L"Consolas", FW_NORMAL, logsFontSize);

  isRunning = 1;

  i64 size = GetMyFileSize(path);
  c8* file = (c8*)valloc(size);
  ReadFileInto(path, size, file);
  i32 wideCharsCount = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, file, size, 0, 0);
  c16* text = (c16*)valloc(wideCharsCount * sizeof(c16));
  MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, file, size, text, wideCharsCount);
  Init(text, wideCharsCount);
  InitTests();

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
