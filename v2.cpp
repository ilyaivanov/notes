#define UNICODE
#define WIN32_LEAN_AND_MEAN

#define FULLSCREEN

#include "win32.cpp"
#include "anim.cpp"
#include "vim.cpp"

enum Mode { Normal, Insert, ReplaceChar, VisualLine, Visual, Modal };
Mode mode = Normal;

HWND win;
AppState appState;
HFONT font;
i32 fontSize;
#define initialFontSize 15

HDC windowDc;
HDC drawingDc;

v3 fontColor = {0.85, 0.85, 0.85};
v3 white = {1, 1, 1};
v3 grey = {0.05, 0.05, 0.05};
v3 bg = {0.05, 0.05, 0.05};
v3 red = {1, 0.2, 0.2};
v3 line = {0.1, 0.1, 0.1};
v3 lineNumberColor = {0.3, 0.3, 0.3};
v3 lineColor = {0.1, 0.1, 0.1};
v3 lineInsertColor = {0.15, 0.1, 0.1};
v3 lineVisualColor = {0.1, 0.1, 0.2};
v3 cursorColor = {1, 220.0f / 255.0f, 50.0f / 255.0f};
v3 cursorInsertColor = {1, 60.0f / 255.0f, 60.0f / 255.0f};
v3 cursorVisualColor = {120.0f / 255.0f, 120.0f / 255.0f, 255.0f / 255.0f};
v3 cursorTextColor = {0.05, 0.05, 0.05};
v3 selectedBg = {0, 0, 1};
v3 selectedText = {0.05, 0.05, 0.05};

HBITMAP bitmap;
BITMAPINFO bitmapInfo;
MyBitmap canvas;

void PaintWindow() {
  StretchDIBits(windowDc, 0, 0, canvas.width, canvas.height, 0, 0, canvas.width, canvas.height,
                canvas.pixels, &bitmapInfo, DIB_RGB_COLORS, SRCCOPY);
}

Buffer leftBuffer;
Buffer rightBuffer;
Buffer middleBuffer;

Buffer* selectedBuffer;

void SelectLeftBuffer() {
  if (selectedBuffer == &rightBuffer)
    selectedBuffer = &middleBuffer;
  else
    selectedBuffer = &leftBuffer;
}

void SelectRightBuffer() {
  if (selectedBuffer == &leftBuffer)
    selectedBuffer = &middleBuffer;
  else
    selectedBuffer = &rightBuffer;
}

// TODO: fix this shity code, I need this in order to comminicate with vim.cpp, which currently
// accepts references
Buffer& GetSelectedBuffer() {
  if (selectedBuffer == &leftBuffer)
    return leftBuffer;
  else if (selectedBuffer == &middleBuffer)
    return middleBuffer;
  else
    return rightBuffer;
}

void LoadFile(c16* path, Buffer& buffer) {
  memcpy(buffer.path, path, wstrlen(path) * sizeof(c16));

  i64 size = GetMyFileSize(path);
  c8* file = (c8*)valloc(size);
  ReadFileInto(path, size, file);
  i32 wideCharsCount = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, file, size, 0, 0);
  c16* text = (c16*)valloc(wideCharsCount * sizeof(c16));
  MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, file, size, text, wideCharsCount);

  i32 bufferPos = 0;
  for (i32 i = 0; i < wideCharsCount; i++) {
    if (text[i] != '\r') {
      buffer.text[bufferPos] = text[i];
      bufferPos++;
    }
  }
  buffer.textLen = bufferPos;

  vfree(file);
  vfree(text);
  // RebuildLines();
}

void InitBuffer(const c16* path, Buffer& buffer) {
  buffer.textCapacity = MB(2) / sizeof(c16);
  buffer.text = (c16*)valloc(buffer.textCapacity * sizeof(c16));
  LoadFile((c16*)path, buffer);
}

void SaveCurrentBuffer() {
  i32 utf8Count =
      WideCharToMultiByte(CP_UTF8, 0, selectedBuffer->text, selectedBuffer->textLen, 0, 0, 0, 0);

  c8* text = (c8*)valloc(utf8Count * sizeof(c8));
  WideCharToMultiByte(CP_UTF8, 0, selectedBuffer->text, selectedBuffer->textLen, text, utf8Count, 0,
                      0);
  WriteMyFile(selectedBuffer->path, text, utf8Count);
  selectedBuffer->isModified = false;
}

HFONT CreateAppFont(HDC dc, const wchar_t* name, i32 weight, i32 fontSize, DWORD quality) {
  int h = -MulDiv(fontSize, GetDeviceCaps(dc, LOGPIXELSY), USER_DEFAULT_SCREEN_DPI);
  HFONT font = CreateFontW(h, 0, 0, 0,
                           weight, // Weight
                           0,      // Italic
                           0,      // Underline
                           0,      // Strikeout
                           DEFAULT_CHARSET, OUT_TT_ONLY_PRECIS, CLIP_DEFAULT_PRECIS,

                           // I've experimented with the Chrome and it doesn't render LCD
                           // quality for fonts above 32px
                           // ANTIALIASED_QUALITY,
                           // I'm experiencing troubles with LCD font rendering and
                           // setting a custom color for a shader
                           quality,

                           DEFAULT_PITCH, name);
  return font;
}

void ResizeFont(i32 newSize) {
  fontSize = newSize;
  if (font)
    DeleteFont(font);

  font = CreateAppFont(appState.dc, L"Consolas", FW_NORMAL, fontSize, ANTIALIASED_QUALITY);
}

void Init() {
  ResizeFont(initialFontSize);
  InitBuffer(L"main.cpp", leftBuffer);
  InitBuffer(L"v2.cpp", middleBuffer);
  InitBuffer(L"notes.txt", rightBuffer);
  selectedBuffer = &rightBuffer;
}

f32 GetFontHeight() {
  TEXTMETRIC textMetric;
  GetTextMetrics(appState.dc, &textMetric);
  return textMetric.tmAscent + textMetric.tmDescent;
}

void HandleMovement(char ch) {
  if (ch == L'J') {
    MoveDown(GetSelectedBuffer(), appState.dc);
  } else if (ch == L'K') {
    MoveUp(GetSelectedBuffer(), appState.dc);
  } else if (ch == L'H') {
    MoveLeft(GetSelectedBuffer());
    UpdateDesiredOffset(GetSelectedBuffer(), appState.dc);
  } else if (ch == L'L') {
    MoveRight(GetSelectedBuffer());
    UpdateDesiredOffset(GetSelectedBuffer(), appState.dc);
  } else if (ch == 'W' && IsKeyPressed(VK_SHIFT)) {
    selectedBuffer->cursor = JumpWordForwardIgnorePunctuation(GetSelectedBuffer());
    UpdateDesiredOffset(GetSelectedBuffer(), appState.dc);
  } else if (ch == 'B' && IsKeyPressed(VK_SHIFT)) {
    selectedBuffer->cursor = JumpWordBackwardIgnorePunctuation(GetSelectedBuffer());
    UpdateDesiredOffset(GetSelectedBuffer(), appState.dc);
  } else if (ch == 'W') {
    selectedBuffer->cursor = JumpWordForward(GetSelectedBuffer());
    UpdateDesiredOffset(GetSelectedBuffer(), appState.dc);
  } else if (ch == 'B') {
    selectedBuffer->cursor = JumpWordBackward(GetSelectedBuffer());
    UpdateDesiredOffset(GetSelectedBuffer(), appState.dc);
  }
}

void InsertCharAtCursor(char ch) {
  InsertCharAt(GetSelectedBuffer(), selectedBuffer->cursor, ch);
  selectedBuffer->cursor++;
  UpdateDesiredOffset(GetSelectedBuffer(), appState.dc);
  selectedBuffer->isModified = true;
}

// this is used to prevent WM_CHAR event after WM_KEYDOWN. Should not be called inside WM_CHAR
bool ignoreNextCharEvent;
void EnterInsertMode() {
  mode = Insert;
  ignoreNextCharEvent = true;
}

void RemoveCharFromLeft() {
  if (selectedBuffer->cursor > 0) {
    RemoveCharAt(GetSelectedBuffer(), selectedBuffer->cursor - 1);
    selectedBuffer->cursor--;
    UpdateDesiredOffset(GetSelectedBuffer(), appState.dc);
    selectedBuffer->isModified = true;
  }
}

void SetCursor(i32 pos) {
  selectedBuffer->cursor = pos;
  UpdateDesiredOffset(GetSelectedBuffer(), appState.dc);
}

void SetCursorKeepDesiredOffset(i32 pos) {
  selectedBuffer->cursor = pos;
}

LRESULT OnEvent(HWND handle, UINT message, WPARAM wParam, LPARAM lParam) {
  switch (message) {
  case WM_CHAR:
    if (ignoreNextCharEvent)
      ignoreNextCharEvent = false;
    else if (mode == Insert) {
      if (wParam >= ' ' && wParam <= '~')
        InsertCharAtCursor(wParam);
      else if (wParam == '\r')
        InsertCharAtCursor('\n');
      else if (wParam == VK_BACK)
        RemoveCharFromLeft();

    } else if (mode == Normal) {
      if (wParam == L'.')
        ResizeFont(fontSize + 1);
      else if (wParam == L',')
        ResizeFont(fontSize - 1);
    }
    break;
  case WM_KEYDOWN:
    if (mode == Insert) {
      if (wParam == VK_ESCAPE) {
        mode = Normal;
      }
    }
    if (mode == ReplaceChar) {
      if (wParam == VK_ESCAPE) {
        mode = Normal;
      }
    }
    if (mode == VisualLine) {
      i32 selStart = Min(selectedBuffer->cursor, selectedBuffer->selectionStart);
      i32 selEnd = Max(selectedBuffer->cursor, selectedBuffer->selectionStart);
      i32 selStartLine = FindLineStartv2(GetSelectedBuffer(), selStart);
      i32 selEndLine = FindLineEndv2(GetSelectedBuffer(), selEnd);

      if (wParam == VK_ESCAPE) {
        mode = Normal;
      }

      else if (wParam == 'D') {
        RemoveChars(GetSelectedBuffer(), selStartLine, selEndLine);
        selectedBuffer->cursor = ClampCursor(
            GetSelectedBuffer(),
            selStartLine + FindLineOffsetByDistance(GetSelectedBuffer(), appState.dc, selStartLine,
                                                    selectedBuffer->desiredOffset));
        mode = Normal;
      }

      if (wParam == 'C') {
        RemoveChars(GetSelectedBuffer(), selStartLine, selEndLine - 2);
        selectedBuffer->cursor = selStartLine;
        EnterInsertMode();
      }

      if (wParam == 'Y') {
        ClipboardCopy(appState.window, selectedBuffer->text + selStartLine,
                      selEndLine - selStartLine + 1);
        mode = Normal;
      }

      else {
        HandleMovement(wParam);
      }
    } else if (mode == Normal) {
      if (wParam == 'O' && IsKeyPressed(VK_SHIFT)) {
        i32 target = 0;
        target = FindLineStartv2(GetSelectedBuffer(), selectedBuffer->cursor);

        InsertCharAt(GetSelectedBuffer(), target, '\n');
        selectedBuffer->cursor = target;
        EnterInsertMode();
        UpdateDesiredOffset(GetSelectedBuffer(), appState.dc);
      } else if (wParam == 'O') {
        i32 target = 0;
        target = FindLineEndv2(GetSelectedBuffer(), selectedBuffer->cursor);

        InsertCharAt(GetSelectedBuffer(), target + 1, '\n');
        selectedBuffer->cursor = target + 1;
        EnterInsertMode();
        UpdateDesiredOffset(GetSelectedBuffer(), appState.dc);
      }
      if (wParam == 'P') {
        i32 len;
        c16* text = ClipboardPaste(appState.window, &len);

        i32 at = selectedBuffer->cursor;
        i32 cursorPos = at + len;

        if (HasNewLine(text)) {
          at = FindLineEndv2(GetSelectedBuffer(), at) + 1;
          cursorPos = at;
        }

        InsertCharsAt(GetSelectedBuffer(), at, text, len);
        vfree(text);
        SetCursor(cursorPos);
      }

      if (wParam == 'V' && IsKeyPressed(VK_SHIFT)) {
        selectedBuffer->selectionStart = selectedBuffer->cursor;
        mode = VisualLine;
      }

      if (wParam == VK_BACK)
        RemoveCharFromLeft();

      if (wParam == 'X') {
        if (selectedBuffer->cursor < selectedBuffer->textLen - 1) {
          RemoveCharAt(GetSelectedBuffer(), selectedBuffer->cursor);
          selectedBuffer->isModified = true;
        }
      }
      if (wParam == 'I') {
        EnterInsertMode();
      }
      if (wParam == 'Q') {
        PostQuitMessage(0);
        appState.isRunning = false;
      }
      if (wParam == 'S' && IsKeyPressed(VK_CONTROL))
        SaveCurrentBuffer();
      if (wParam == 'F') {
        appState.isFullscreen = !appState.isFullscreen;
        SetFullscreen(appState.window, appState.isFullscreen);
      }
      if (wParam == L'H' && IsKeyPressed(VK_CONTROL))
        SelectLeftBuffer();
      else if (wParam == L'L' && IsKeyPressed(VK_CONTROL))
        SelectRightBuffer();
      else if (wParam == L'J' && IsKeyPressed(VK_CONTROL))
        selectedBuffer->offset.target += GetFontHeight() * 3;
      else if (wParam == L'K' && IsKeyPressed(VK_CONTROL))
        selectedBuffer->offset.target -= GetFontHeight() * 3;
      else if (wParam == L'D' && IsKeyPressed(VK_CONTROL))
        selectedBuffer->offset.target += appState.size.y / 2;
      else if (wParam == L'U' && IsKeyPressed(VK_CONTROL))
        selectedBuffer->offset.target -= appState.size.y / 2;
      else {
        HandleMovement(wParam);
      }
    }

    break;
  case WM_DESTROY:
    PostQuitMessage(0);
    appState.isRunning = false;
    break;
  case WM_PAINT: {
    PAINTSTRUCT ps;
    BeginPaint(handle, &ps);
    EndPaint(handle, &ps);
  } break;
  case WM_SIZE: {

    canvas.width = LOWORD(lParam);
    canvas.height = HIWORD(lParam);
    appState.size.x = canvas.width;
    appState.size.y = canvas.height;

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

    // if (appState.isRunning) {
    //   OnResize(appState);
    //
    //   SelectBitmap(drawingDc, bitmap);
    //   Draw(appState);
    //   PaintWindow();
    // }
  }
  }
  return DefWindowProc(handle, message, wParam, lParam);
}

void SetColors(v3 fg, v3 bg) {
  fg *= 255.0f;
  bg *= 255.0f;
  SetBkColor(appState.dc, RGB(bg.x, bg.y, bg.z));
  SetTextColor(appState.dc, RGB(fg.x, fg.y, fg.z));
}

u32 ColorFromVec(v3 color) {
  u32 r = (u32)(color.x * 255.0);
  u32 g = (u32)(color.y * 255.0);
  u32 b = (u32)(color.z * 255.0);

  return r << 16 | g << 8 | b << 0;
}

void PaintRect(i32 x, i32 y, i32 width, i32 height, v3 color) {
  u32 c = ColorFromVec(color);
  for (i32 i = x; i < x + width; i++) {
    for (i32 j = y; j < y + height; j++) {
      if (i >= 0 && i < (i32)canvas.width && j < (i32)canvas.height && j >= 0)
        canvas.pixels[j * canvas.width + i] = c;
    }
  }
}

struct Rect {
  f32 x;
  f32 y;
  f32 width;
  f32 height;
};

struct CursorPos {
  i32 lineStart;
  i32 row;
  i32 col;
};

CursorPos GetCursorPos(Buffer& buffer) {
  i32 lineStart = 0;
  i32 line = 0;
  c16* text = buffer.text;
  CursorPos res = {};

  for (i32 i = 0; i < buffer.textLen; i++) {
    if (text[i] == L'\n' || i == buffer.textLen - 1) {
      if (lineStart <= buffer.cursor && buffer.cursor <= i) {
        res.row = line;
        res.col = i - lineStart;
        res.lineStart = lineStart;
        return res;
      }
      lineStart = i + 1;
      line++;
    }
  }
  return res;
}

i32 GetLinesCount(Buffer& buffer) {
  i32 res = 1;
  for (i32 i = 0; i < buffer.textLen; i++) {
    if (buffer.text[i] == L'\n')
      res++;
  }
  return res;
}

void DrawBuffer(Buffer& buffer, Rect rect) {
  f32 padding = 10;
  f32 x = rect.x + padding;
  f32 y = rect.y + padding;
  i32 size = buffer.textLen;
  c16* text = buffer.text;

  f32 fontHeight = GetFontHeight();
  f32 runningY = y - buffer.offset.current;
  i32 lineStart = 0;
  i32 currentLine = 0;

  CursorPos cursor = GetCursorPos(buffer);
  CharBuffer buff = {};

  v3 lineColorToUse = lineColor;
  if (mode == Insert)
    lineColorToUse = lineInsertColor;
  else if (mode == VisualLine)
    lineColorToUse = lineVisualColor;

  v3 cursorColorToUse = cursorColor;
  if (mode == Insert)
    cursorColorToUse = cursorInsertColor;
  if (mode == VisualLine)
    cursorColorToUse = cursorVisualColor;

  i32 selStart = Min(buffer.cursor, buffer.selectionStart);
  i32 selEnd = Max(buffer.cursor, buffer.selectionStart);
  i32 selStartLine = FindLineStartv2(buffer, selStart);
  i32 selEndLine = FindLineEndv2(buffer, selEnd);

  i32 spaceForLineNumbers = 50;
  i32 textToLineNumbersPadding = 10;

  for (i32 i = 0; i < size; i++) {
    if (text[i] == L'\n' || i == size - 1) {
      if (runningY > -fontHeight) {
        i32 lineEnd = i;

        i32 isLineSelected = &buffer == selectedBuffer && currentLine == cursor.row;
        if (isLineSelected) {
          PaintRect(rect.x, runningY, rect.width, fontHeight, lineColorToUse);
          SetColors(white, lineColorToUse);
        } else
          SetColors(lineNumberColor, bg);

        buff.len = 0;
        Append(&buff, currentLine + 1);

        SetTextAlign(appState.dc, TA_RIGHT);
        TextOutW(appState.dc, x + (spaceForLineNumbers - textToLineNumbersPadding), round(runningY),
                 buff.content, buff.len);

        if (&buffer == selectedBuffer && mode == VisualLine) {
          i32 isLineInRange = (selStartLine <= lineStart && lineStart < selEndLine) ||
                              (selStartLine < lineEnd && lineEnd < selEndLine);

          if (isLineInRange)
            SetColors(white, selectedBg);
          else
            SetColors(fontColor, bg);
        } else if (isLineSelected)
          SetColors(white, lineColorToUse);
        else
          SetColors(fontColor, bg);

        SetTextAlign(appState.dc, TA_LEFT);
        TextOutW(appState.dc, x + spaceForLineNumbers, round(runningY), text + lineStart,
                 lineEnd - lineStart + 1);

        if (isLineSelected) {
          SetColors(cursorTextColor, cursorColorToUse);
          f32 cursorOffset =
              GetTextWidth(appState.dc, buffer.text, cursor.lineStart, buffer.cursor);

          SIZE size;
          GetTextExtentPoint32W(appState.dc, L"w", 1, &size);
          i32 charWidth = size.cx;
          PaintRect(x + spaceForLineNumbers + cursorOffset, round(runningY), charWidth, fontHeight,
                    cursorColorToUse);
          TextOutW(appState.dc, x + spaceForLineNumbers + cursorOffset, round(runningY),
                   text + buffer.cursor, 1);
        }
      }

      lineStart = i + 1;
      runningY += fontHeight;
      currentLine++;

      if (runningY > appState.size.y)
        break;
    }
  }

  if (buffer.isModified)
    SetColors(cursorInsertColor, bg);
  else
    SetColors(fontColor, bg);
  SetTextAlign(appState.dc, TA_RIGHT);
  TextOutW(appState.dc, rect.x + rect.width - padding,
           rect.y + rect.height - GetFontHeight() - padding, buffer.path, wstrlen(buffer.path));
}

void DrawInfo() {
  f32 fontHeight = GetFontHeight();
  v2 padding = {10, 10};
  v2 size = {300, 300};
  v2 topLeft = appState.size - padding - size - vec2(0, appState.size.y / 2 - size.y / 2);
  v3 bg = {0.2, 0.1, 0.1};
  PaintRect(topLeft.x, topLeft.y, size.x, size.y, bg);

  topLeft += vec2(5, 5);
  SetColors(white, bg);

  CharBuffer buf = {};
  Append(&buf, L"FPS: ");
  Append(&buf, (i32)(round(1000.0f / appState.lastFrameTimeMs)));

  SetTextAlign(appState.dc, TA_LEFT);
  TextOutW(appState.dc, topLeft.x, topLeft.y, buf.content, buf.len);
  topLeft.y += fontHeight;
  buf.len = 0;

  Append(&buf, L"Desired: ");
  Append(&buf, (i32)(selectedBuffer->desiredOffset));

  TextOutW(appState.dc, topLeft.x, topLeft.y, buf.content, buf.len);
  topLeft.y += fontHeight;
  buf.len = 0;
}

extern "C" void WinMainCRTStartup() {
  SetProcessDPIAware();
  InitAnimations();
  win = OpenWindow(OnEvent);
  appState.window = win;

#ifdef FULLSCREEN
  appState.isFullscreen = true;
  SetFullscreen(appState.window, appState.isFullscreen);
#endif

  ShowWindow(win, SW_SHOW);

  windowDc = GetDC(win);
  drawingDc = CreateCompatibleDC(0);
  appState.dc = drawingDc;

  i64 freq = GetPerfFrequency();
  i64 appStart = GetPerfCounter();
  i64 frameStart = appStart;

  appState.isRunning = true;
  // currentBitmap = &canvas;
  // currentDc = drawingDc;

  Init();
  while (appState.isRunning) {
    MSG msg;

    while (PeekMessageW(&msg, 0, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    memset(canvas.pixels, round(bg.x * 255.0f), canvas.width * canvas.height * 4);
    SelectFont(appState.dc, font);

    SelectBitmap(drawingDc, bitmap);

    Rect left = {0, 0, appState.size.x / 3, appState.size.y};
    Rect middle = {left.x + left.width, left.y, left.width, left.height};
    Rect right = {middle.x + middle.width, middle.y, middle.width, middle.height};

    DrawBuffer(leftBuffer, left);
    DrawBuffer(middleBuffer, middle);
    DrawBuffer(rightBuffer, right);

    PaintRect(left.x + left.width - 1, left.y, 2, left.height, line);
    PaintRect(middle.x + middle.width - 1, middle.y, 2, middle.height, line);

    DrawInfo();
    // Draw(appState);
    PaintWindow();

    f32 deltaSec = appState.lastFrameTimeMs / 1000.0f;
    UpdateSpring(&leftBuffer.offset, deltaSec);
    UpdateSpring(&middleBuffer.offset, deltaSec);
    UpdateSpring(&rightBuffer.offset, deltaSec);

    i64 frameEnd = GetPerfCounter();
    appState.lastFrameTimeMs = (f32)(frameEnd - frameStart) / (f32)freq * 1000.0f;
    frameStart = frameEnd;
    appState.appTimeMs = (f32)(frameEnd - appStart) / (f32)freq * 1000.0f;
  }
  // Teardown(appState);

  PostQuitMessage(0);
  ExitProcess(0);
}
