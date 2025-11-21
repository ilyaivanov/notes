#define UNICODE
#define WIN32_LEAN_AND_MEAN

// #define FULLSCREEN

#include "../win32.cpp"
#include "item.cpp"
// #include "anim.cpp"
// #include "vim.cpp"

// enum Mode { Normal, Insert, ReplaceChar, VisualLine, Visual, Modal };
// Mode mode = Normal;

HWND win;
AppState appState;
HFONT font;
i32 fontSize;
#define initialFontSize 14

HDC windowDc;
HDC drawingDc;

v3 fontColor = {0.85, 0.85, 0.85};
v3 white = {1, 1, 1};
v3 black = {0, 0, 0};
v3 grey = {0.05, 0.05, 0.05};
v3 bg = {0.05, 0.05, 0.05};
v3 red = {1, 0.2, 0.2};
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

void DrawApp();
LRESULT OnEvent(HWND handle, UINT message, WPARAM wParam, LPARAM lParam) {
  switch (message) {
  case WM_KEYDOWN:
    if (wParam == 'Q') {
      PostQuitMessage(0);
      appState.isRunning = false;
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

void UpdateFontSize() {
  if (font)
    DeleteFont(font);

  font = CreateAppFont(appState.dc, L"Segoe UI", FW_NORMAL, fontSize, CLEARTYPE_QUALITY);

  // ANTIALIASED_QUALITY);
}

v2 padding = {10, 5};

void PaintSplit(Item* root, Rect rect) {
  SetColors(white, bg);
  v2 runningPos = vec2(rect.x, rect.y) + padding;

  StackEntry stack[200];
  int stackLen = 0;
  f32 step = 20;

  stack[stackLen++] = {root, -1};

  while (stackLen > 0) {
    StackEntry entry = stack[--stackLen];

    if (entry.level >= 0) {
      TextOutA(appState.dc, runningPos.x + entry.level * step, runningPos.y, entry.item->text,
               entry.item->textLen);

      runningPos.y += GetFontHeight();
    }

    for (i32 i = entry.item->childrenLen - 1; i >= 0; i--) {
      stack[stackLen++] = {entry.item->children[i], entry.level + 1};
    }
  }
}

Item* root;

void Init() {
  fontSize = initialFontSize;
  UpdateFontSize();

  FileContent file = ReadMyFileImp("links.txt");
  root = ParseFileIntoRoot(file.content, file.size);
  // CreateItem(root, "foo");
  // CreateItem(root->children[0], "nested foo");
  // CreateItem(root->children[0], "nested foo 2");
  // CreateItem(root, "bar");
  // CreateItem(root, "buzz");
};

void DrawApp() {
  memset(canvas.pixels, round(bg.x * 255.0f), canvas.width * canvas.height * 4);
  SelectFont(appState.dc, font);
  SelectBitmap(drawingDc, bitmap);

  Rect left = {0, 0, appState.size.x / 3, appState.size.y};
  Rect middle = {left.x + left.width, left.y, left.width, left.height};
  Rect right = {middle.x + middle.width, middle.y, middle.width, middle.height};

  // PaintSplit(root, left);
  PaintSplit(root, middle);
  // PaintSplit(root, right);

  PaintRect(left.x + left.width - 1, left.y, 2, left.height, line);
  PaintRect(middle.x + middle.width - 1, middle.y, 2, middle.height, line);

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
  while (appState.isRunning) {
    MSG msg;

    while (PeekMessageW(&msg, 0, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    DrawApp();

    i64 frameEnd = GetPerfCounter();
    appState.lastFrameTimeMs = (f32)(frameEnd - frameStart) / (f32)freq * 1000.0f;
    frameStart = frameEnd;
    appState.appTimeMs = (f32)(frameEnd - appStart) / (f32)freq * 1000.0f;
  }

  ExitProcess(0);
}
