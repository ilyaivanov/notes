#define UNICODE
#define WIN32_LEAN_AND_MEAN

// #define FULLSCREEN

#include "../win32.cpp"
#include "item.cpp"
#include "actions.cpp"

#define filePath L"sample.txt"
i32 step = 20;

HWND win;
AppState appState;
HFONT font;
#define initialFontSize 14

HDC windowDc;
HDC drawingDc;

v3 fontColor = {0.85, 0.85, 0.85};
v3 white = {1, 1, 1};
v3 black = {0, 0, 0};
v3 grey = {0.05, 0.05, 0.05};
v3 bg = {0.05, 0.05, 0.05};
v3 red = {0.5, 0.2, 0.2};
v3 line = {0.1, 0.1, 0.1};
v3 lineNumberColor = {0.3, 0.3, 0.3};
v3 lineColor = {0.15, 0.15, 0.15};
v3 lineInsertColor = {0.15, 0.1, 0.1};
v3 lineVisualColor = {0.1, 0.1, 0.2};
v3 cursorColor = {1, 220.0f / 255.0f, 50.0f / 255.0f};
v3 cursorInsertColor = {1, 60.0f / 255.0f, 60.0f / 255.0f};
v3 cursorVisualColor = {120.0f / 255.0f, 120.0f / 255.0f, 255.0f / 255.0f};
v3 cursorTextColor = {0.05, 0.05, 0.05};
v3 selectedBg = {0, 0, 1};
v3 selectedText = {0.05, 0.05, 0.05};

v2 padding = {20, 5};

HBITMAP bitmap;
BITMAPINFO bitmapInfo;
MyBitmap canvas;

void PaintWindow() {
  StretchDIBits(windowDc, 0, 0, canvas.width, canvas.height, 0, 0, canvas.width, canvas.height,
                canvas.pixels, &bitmapInfo, DIB_RGB_COLORS, SRCCOPY);
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

void UpdateFontSize() {
  if (font)
    DeleteFont(font);

  font = CreateAppFont(appState.dc, L"Segoe UI", FW_NORMAL, fontSize,
                       CLEARTYPE_QUALITY); // ANTIALIASED_QUALITY
}

i32 GetTextWidth(char* text, i32 from, i32 to) {
  RECT rc = {0, 0, 0, 0};

  DrawTextA(appState.dc, text + from, to - from, &rc, DT_SINGLELINE | DT_NOPREFIX | DT_CALCRECT);
  return rc.right - rc.left;
}

i32 SelectedItemTextWidth(i32 to) {
  return GetTextWidth(selectedItem->text, 0, to);
}

bool ignoreNextCharEvent;
void EnterInsertMode() {
  mode = Insert;
  ignoreNextCharEvent = true;
}

void HandleMovement(UINT wParam) {
  if (wParam == 'H' && IsKeyPressed(VK_MENU))
    MoveItemLeft(selectedItem);
  if (wParam == 'J' && IsKeyPressed(VK_MENU))
    MoveItemDown(selectedItem);
  if (wParam == 'K' && IsKeyPressed(VK_MENU))
    MoveItemUp(selectedItem);
  if (wParam == 'L' && IsKeyPressed(VK_MENU))
    MoveItemRight(selectedItem);
}

void UpdateCursorPosWithDesiredOffset(i32 pos) {
  cursor.pos = clamp(pos, 0, selectedItem->textLen);
  cursor.desiredOffset = GetItemLevel(selectedItem) * step + SelectedItemTextWidth(cursor.pos);
}

void RemoveWord() {
  i32 to = cursor.pos - 1;
  i32 from = cursor.pos - 1;
  char* text = selectedItem->text;
  while (from >= 0 && text[from] == ' ')
    from--;

  while (from >= 0 && IsAlphaNumeric(text[from]))
    from--;

  from++;
  RemoveChars(selectedItem, from, to);
  UpdateCursorPosWithDesiredOffset(from);
}

void HandleEnter() {
  i32 charsToNext = selectedItem->textLen - cursor.pos;
  Item* newItem = CreateEmptyItem(charsToNext + EMPTY_ITEM_TEXT_CAPACITY);
  InsertChildAt(selectedItem->parent, newItem, IndexOf(selectedItem) + 1);
  if (charsToNext > 0) {
    InsertCharsAt(newItem, 0, selectedItem->text + cursor.pos, charsToNext);
    RemoveChars(selectedItem, cursor.pos + 1, selectedItem->textLen);
  }

  selectedItem = newItem;
  UpdateCursorPosWithDesiredOffset(0);
}

void FindPositionBasedOnDesiredOffset() {
  i32 levelOffset = GetItemLevel(selectedItem) * step;
  i32 offset = levelOffset;
  i32 targetLen = 0;
  while (offset < cursor.desiredOffset && targetLen < selectedItem->textLen) {
    targetLen++;
    offset = levelOffset + SelectedItemTextWidth(targetLen);
  }

  if (targetLen > 0) {
    i32 prevOffset = levelOffset + SelectedItemTextWidth(targetLen - 1);
    if (offset - cursor.desiredOffset > cursor.desiredOffset - prevOffset)
      targetLen--;
  }

  cursor.pos = targetLen;
}

void DrawApp();
LRESULT OnEvent(HWND handle, UINT message, WPARAM wParam, LPARAM lParam) {
  switch (message) {
  case WM_CHAR:
    if (ignoreNextCharEvent)
      ignoreNextCharEvent = false;

    else {
      if (mode == Normal) {
        AppendChar(wParam);
      } else if (mode == Insert) {
        if (wParam == VK_ESCAPE) {
          mode = Normal;
        } else if (wParam == VK_RETURN)
          HandleEnter();
        else if (wParam == VK_BACK) {
          if (cursor.pos > 0) {
            RemoveChars(selectedItem, cursor.pos - 1, cursor.pos - 1);
            UpdateCursorPosWithDesiredOffset(cursor.pos - 1);
          }
        } else {
          InsertCharAt(selectedItem, cursor.pos, wParam);
          UpdateCursorPosWithDesiredOffset(cursor.pos + 1);
        }
      }
    }
    break;
  case WM_SYSCOMMAND:
    if (wParam == SC_KEYMENU)
      return 0;

    break;
  case WM_SYSKEYDOWN:
  case WM_KEYDOWN:
    if (mode == Insert) {
      HandleMovement(wParam);

      if (wParam == 'W' && IsKeyPressed(VK_CONTROL)) {
        ignoreNextCharEvent = true;
        RemoveWord();
      }
    }
    if (mode == Normal) {
      if (IsLetter(wParam)) {
        if (!IsKeyPressed(VK_SHIFT))
          AppendChar(ToLower(wParam));
        else
          AppendChar(wParam);

        ignoreNextCharEvent = true;
      }

      if (IsKeyPressed(VK_CONTROL)) {
        if (wParam == VK_OEM_PLUS)
          AppendChar('=');

        if (wParam == VK_OEM_MINUS)
          AppendChar('-');
      }

      if (wParam == VK_BACK) {
        if (cursor.pos > 0) {
          RemoveChars(selectedItem, cursor.pos - 1, cursor.pos - 1);
          UpdateCursorPosWithDesiredOffset(cursor.pos - 1);
        }
      }

      if (wParam == VK_RETURN)
        HandleEnter();

      if (wParam == VK_F11) {
        appState.isFullscreen = !appState.isFullscreen;
        SetFullscreen(win, appState.isFullscreen);
      }

      if (wParam == 'Q') {
        PostQuitMessage(0);
        appState.isRunning = false;
      }

      if (wParam == 'S' && IsKeyPressed(VK_CONTROL)) {
        i32 capacity = MB(2);
        void* buffer = valloc(capacity);
        i32 bytesWritten;
        SerializeRoot(root, buffer, &bytesWritten, capacity);
        WriteMyFile(filePath, (char*)buffer, bytesWritten);
        vfree(buffer);
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

    if (appState.isRunning) {
      DrawApp();
    }
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

struct Rect {
  f32 x;
  f32 y;
  f32 width;
  f32 height;
};

void PaintRect(i32 x, i32 y, i32 width, i32 height, v3 color) {
  u32 c = ColorFromVec(color);
  for (i32 i = x; i < x + width; i++) {
    for (i32 j = y; j < y + height; j++) {
      if (i >= 0 && i < (i32)canvas.width && j < (i32)canvas.height && j >= 0)
        canvas.pixels[j * canvas.width + i] = c;
    }
  }
}

void PaintRect(Rect rect, v3 color) {
  PaintRect(rect.x, rect.y, rect.width, rect.height, color);
}

f32 GetFontHeight() {
  TEXTMETRIC textMetric;
  GetTextMetrics(appState.dc, &textMetric);
  return textMetric.tmAscent + textMetric.tmDescent;
}

void PaintSplit(Item* root, Rect rect) {
  v2 runningPos = vec2(rect.x, rect.y) + padding;

  StackEntry stack[200];
  int stackLen = 0;

  stack[stackLen++] = {root, -1};

  while (stackLen > 0) {
    StackEntry entry = stack[--stackLen];
    bool isClosed = !entry.item->isOpen && entry.item->childrenLen > 0;

    v3 lineColorToUse = lineColor;
    v3 cursorColorToUse = cursorColor;
    if (mode == Insert) {
      lineColorToUse = lineInsertColor;
      cursorColorToUse = cursorInsertColor;
    }

    if (entry.level >= 0) {
      if (entry.item == selectedItem) {
        PaintRect(rect.x, runningPos.y, rect.width, GetFontHeight(), lineColorToUse);
        SetColors(white, lineColorToUse);
      } else {
        SetColors(white, bg);
      }

      if (isClosed) {
        PaintRect(rect.x, runningPos.y, 4, GetFontHeight(), red);
      }

      f32 x = runningPos.x + entry.level * step;
      char* text = entry.item->text;
      i32 len = entry.item->textLen;
      HDC dc = appState.dc;
      RECT re = {(i32)x, (i32)runningPos.y, i32(rect.x + rect.width), i32(rect.y + rect.height)};
      DrawTextA(dc, text, len, &re, DT_NOPREFIX | DT_LEFT | DT_TOP);

      if (entry.item == selectedItem) {
        i32 cursorX = x + SelectedItemTextWidth(cursor.pos);
        PaintRect(cursorX, runningPos.y, 1, GetFontHeight(), cursorColorToUse);
      }
      runningPos.y += GetFontHeight();
    }

    if (!isClosed) {
      for (i32 i = entry.item->childrenLen - 1; i >= 0; i--) {
        stack[stackLen++] = {entry.item->children[i], entry.level + 1};
      }
    }
  }
}

void Init() {
  fontSize = initialFontSize;
  UpdateFontSize();

  FileContent file = ReadMyFileImp(filePath);
  root = ParseFileIntoRoot(file.content, file.size);
  selectedItem = root->children[0];
  vfree(file.content);
};

void AppendCommandBuffer(CharBuffer& buff, CommandBuffer& commandBuffer) {
  CharBuffer commandFormatted = {};
  for (i32 i = 0; i < commandBuffer.len; i++) {
    u32 code = commandBuffer.keys[i].code;
    u32 flags = commandBuffer.keys[i].flags;
    if (flags & CtrlKey || flags & AltKey || flags & WinKey)
      Append(&commandFormatted, "<");
    if (flags & CtrlKey)
      Append(&commandFormatted, "C");
    if (flags & AltKey)
      Append(&commandFormatted, "A");
    if (flags & WinKey)
      Append(&commandFormatted, "W");

    if (flags & CtrlKey || flags & AltKey || flags & WinKey)
      Append(&commandFormatted, "-");

    if (code == VK_ESCAPE)
      Append(&commandFormatted, "Esc");
    else {
      if (code == ' ')
        Append(&commandFormatted, '_');
      else
        Append(&commandFormatted, (char)code);
    }

    if (flags & CtrlKey || flags & AltKey || flags & WinKey)
      Append(&commandFormatted, ">");
  }

  Append(&buff, commandFormatted.content);
}

void DrawApp() {
  memset(canvas.pixels, round(bg.x * 255.0f), canvas.width * canvas.height * 4);

  SelectFont(appState.dc, font);

  SelectBitmap(drawingDc, bitmap);
  RECT windowRect = {0, 0, (i32)appState.size.x, (i32)appState.size.y};

  Rect left = {0, 0, appState.size.x, appState.size.y};
  // Rect middle = {left.x + left.width, left.y, left.width, left.height};
  // Rect right = {middle.x + middle.width, middle.y, middle.width, middle.height};

  // PaintSplit(root, left);
  PaintSplit(root, left);
  // PaintSplit(root, right);

  // PaintRect(left.x + left.width - 1, left.y, 2, left.height, line);
  // PaintRect(middle.x + middle.width - 1, middle.y, 2, middle.height, line);

  RECT footerRect = windowRect;
  footerRect.top = windowRect.bottom - 200;
  footerRect.right -= 10;
  footerRect.bottom -= 5;

  CharBuffer buff = {};
  Append(&buff, L"Desired: ");
  Append(&buff, cursor.desiredOffset);

  Append(&buff, L"\nActual: ");
  Append(&buff, GetItemLevel(selectedItem) * step);
  Append(&buff, L" + ");
  Append(&buff, GetTextWidth(selectedItem->text, 0, cursor.pos));

  Append(&buff, L"\nFont: ");
  Append(&buff, fontSize);

  Append(&buff, L"\nCommand (");
  Append(&buff, command.len);
  Append(&buff, L"): ");
  AppendCommandBuffer(buff, command);

  Append(&buff, L"\nLastCommand (");
  Append(&buff, lastCommand.len);
  Append(&buff, L"): ");
  AppendCommandBuffer(buff, lastCommand);

  v3 color = {0.8, 0.8, 0.8};
  SetColors(color, bg);
  DrawTextW(appState.dc, buff.content, buff.len, &footerRect, DT_NOPREFIX | DT_BOTTOM | DT_RIGHT);
  PaintWindow();
}

extern "C" void WinMainCRTStartup() {
  SetProcessDPIAware();
  // InitAnimations();
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

  Init();
  InitActions();
  while (appState.isRunning) {
    MSG msg;

    while (PeekMessageW(&msg, 0, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    DrawApp();

    // Sleep(10);
    i64 frameEnd = GetPerfCounter();
    appState.lastFrameTimeMs = (f32)(frameEnd - frameStart) / (f32)freq * 1000.0f;
    frameStart = frameEnd;
    appState.appTimeMs = (f32)(frameEnd - appStart) / (f32)freq * 1000.0f;
  }

  ExitProcess(0);
}
