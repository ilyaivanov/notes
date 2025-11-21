#define UNICODE
#define WIN32_LEAN_AND_MEAN

#define FULLSCREEN
#define USE_OPENGL

#include "win32.cpp"
#include <gl/gl.h>
#include "gl.cpp"
#include "glProgra.cpp"
#include "sincos.cpp"

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

struct OpenGlState {
  GLuint vertexBuffer;
  GLuint vertexArray;
};

OpenGlState state;

#define POINTS_PER_VERTEX 3
// clang-format off
float cubeVertices[] = {
    -0.5, -0.5, 1,
     1, 1, 1, 
      0, -1, 1,
};
// clang-format on

void CreateGl() {
  glGenBuffers(1, &state.vertexBuffer);
  glGenVertexArrays(1, &state.vertexArray);

  glBindBuffer(GL_ARRAY_BUFFER, state.vertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

  glBindVertexArray(state.vertexArray);
  size_t stride = POINTS_PER_VERTEX * sizeof(float);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
  glEnableVertexAttribArray(0);
}

extern "C" void WinMainCRTStartup() {
  SetProcessDPIAware();
  win = OpenWindow(OnEvent);
  appState.window = win;

  windowDc = GetDC(win);
  u32 baseProgram;

#ifdef USE_OPENGL
  Win32InitOpenGL2(windowDc);
  InitFunctions();
  CreateGl();

  baseProgram = CreateProgram(".\\vert.glsl", ".\\frag.glsl");
#endif

#ifdef FULLSCREEN
  appState.isFullscreen = true;
  SetFullscreen(appState.window, appState.isFullscreen);
#endif

  ShowWindow(win, SW_SHOW);

  drawingDc = CreateCompatibleDC(0);
  appState.dc = drawingDc;

  i64 freq = GetPerfFrequency();
  i64 appStart = GetPerfCounter();
  i64 frameStart = appStart;

  appState.isRunning = true;
  // currentBitmap = &canvas;
  // currentDc = drawingDc;

  // glSwapControl(0);
  while (appState.isRunning) {
    MSG msg;

    while (PeekMessageW(&msg, 0, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    // memset(canvas.pixels, round(bg.x * 255.0f), canvas.width * canvas.height * 4);
    // SelectFont(appState.dc, font);
    //
    // SelectBitmap(drawingDc, bitmap);

    // PaintRect(200, 200, 200, 200, white);

    // PaintWindow();

    // f32 deltaSec = appState.lastFrameTimeMs / 1000.0f;

    glClearColor(0.0, 0.0, 0.0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glViewport(0, 0, appState.size.x, appState.size.y);
    glUseProgram(baseProgram);

    f32 sin;
    f32 cos;
    SinCos(appState.appTimeMs / 1000, &sin, &cos);
    cubeVertices[3] = 0.3 + sin * 0.4;
    cubeVertices[4] = 0.3 + cos * 0.4;

    glBindBuffer(GL_ARRAY_BUFFER, state.vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    glDrawArrays(GL_TRIANGLES, 0, ArrayLength(cubeVertices) / POINTS_PER_VERTEX);

    SwapBuffers(windowDc);

    i64 frameEnd = GetPerfCounter();
    appState.lastFrameTimeMs = (f32)(frameEnd - frameStart) / (f32)freq * 1000.0f;
    frameStart = frameEnd;
    appState.appTimeMs = (f32)(frameEnd - appStart) / (f32)freq * 1000.0f;
  }
  // Teardown(appState);

  PostQuitMessage(0);
  ExitProcess(0);
}
