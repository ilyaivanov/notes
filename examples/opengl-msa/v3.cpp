#define UNICODE
#define WIN32_LEAN_AND_MEAN

// #define FULLSCREEN
#define USE_OPENGL

#include "win32.cpp"
#include <gl/gl.h>
#include "gl.cpp"
#include "glProgra.cpp"
#include "sincos.cpp"

f32 lastFrameTimeMs;
f32 appTimeMs;

HDC dc;

HWND win;
bool isRunning;
v2 size;

LRESULT OnEvent(HWND handle, UINT message, WPARAM wParam, LPARAM lParam) {
  switch (message) {
  case WM_KEYDOWN:
    if (wParam == 'Q') {
      PostQuitMessage(0);
      isRunning = false;
    }
    break;
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
    size.x = (f32)LOWORD(lParam);
    size.y = (f32)HIWORD(lParam);
  }
  }
  return DefWindowProc(handle, message, wParam, lParam);
}

struct OpenGlState {
  GLuint vertexBuffer;
  GLuint vertexArray;
};

OpenGlState state;

#define POINTS_PER_VERTEX 3

// clang-format off
float cubeVertices[] = {
    -0.5, -0.5,  1.0,
     1.0,  1.0,  1.0, 
     0.0, -1.0,  1.0,
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

  u32 baseProgram;

  dc = GetDC(win);
#ifdef USE_OPENGL
  Win32InitOpenGL2(dc);
  InitFunctions();
  CreateGl();

  baseProgram = CreateProgram(L".\\vert.glsl", L".\\frag.glsl");
#endif

#ifdef FULLSCREEN
  appState.isFullscreen = true;
  SetFullscreen(appState.window, appState.isFullscreen);
#endif

  ShowWindow(win, SW_SHOW);

  i64 freq = GetPerfFrequency();
  i64 appStart = GetPerfCounter();
  i64 frameStart = appStart;

  isRunning = true;

  // glSwapControl(0);
  while (isRunning) {
    MSG msg;

    while (PeekMessageW(&msg, 0, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    glClearColor(0.1, 0.0, 0.0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glViewport(0, 0, size.x, size.y);
    glUseProgram(baseProgram);

    f32 sin;
    f32 cos;
    SinCos(appTimeMs / 1000, &sin, &cos);
    cubeVertices[3] = 0.3 + sin * 0.4;
    cubeVertices[4] = 0.3 + cos * 0.4;

    glBindBuffer(GL_ARRAY_BUFFER, state.vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    glDrawArrays(GL_TRIANGLES, 0, ArrayLength(cubeVertices) / POINTS_PER_VERTEX);

    SwapBuffers(dc);

    i64 frameEnd = GetPerfCounter();
    lastFrameTimeMs = (f32)(frameEnd - frameStart) / (f32)freq * 1000.0f;
    frameStart = frameEnd;
    appTimeMs = (f32)(frameEnd - appStart) / (f32)freq * 1000.0f;
  }

  ExitProcess(0);
}
