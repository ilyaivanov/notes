#define UNICODE
#define _UNICODE
// #define WIN32_LEAN_AND_MEAN

#include "gl/glFunctions.c"
#include "gl/openglProgram.c"
#include "math.cpp"
#include "win32.cpp"
#include <gl/GL.h>
#include <math.h>

#include <wrl.h>
// #include "../anim.cpp"
#include "WebView2.h"
#include "font.cpp"
// #include "rand.cpp"
// #include "sound.cpp"
using namespace Microsoft::WRL;

f32 appTime;
f32 frameTime;

HWND win;
HDC dc;

i32 isRunning = 0;
i32 isRunningFrameByFrame = 0;
i32 isFullscreen = 1;
i32 keys[256];
i32 keysDown[256];
v2 screen;
v2 mouse;
i32 mouseButtons[2];

i32 isDebugShown = 0;
i32 mouseButtonsDown[2];
i32 keysPressedThisFrame = 0;
i32 mouseButtonsPressedThisFrame = 0;

#define isDebug
void UpdateAndDraw(f32 delta);

LRESULT OnEvent(HWND handle, UINT message, WPARAM wParam, LPARAM lParam) {
  switch (message) {
  case WM_KEYDOWN:
    keys[wParam] = 1;
    keysDown[wParam] = 1;
    keysPressedThisFrame++;
#ifdef isDebug
    if (wParam == 'Q') {
      PostQuitMessage(0);
      isRunning = 0;
    }
    if (wParam == 'F') {
      isFullscreen = !isFullscreen;
      SetFullscreen(win, isFullscreen);
    }
#endif
    break;
  case WM_KEYUP:
    keys[wParam] = 0;
    break;
  case WM_MOUSEMOVE:
    mouse.x = LOWORD(lParam);
    mouse.y = screen.y - HIWORD(lParam);
    break;
  case WM_LBUTTONDOWN:
    mouseButtonsPressedThisFrame++;
    mouseButtons[0] = 1;
    mouseButtonsDown[0] = 1;
    break;
  case WM_RBUTTONDOWN:
    mouseButtonsPressedThisFrame++;
    mouseButtons[1] = 1;
    mouseButtonsDown[1] = 1;
    break;
  case WM_LBUTTONUP:
    mouseButtons[0] = 0;
    break;
  case WM_RBUTTONUP:
    mouseButtons[1] = 0;
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
    glViewport(0, 0, screen.x, screen.y);
    if (isRunning)
      UpdateAndDraw(0);
    break;
  }
  return DefWindowProc(handle, message, wParam, lParam);
}

u32 POINTS_PER_VERTEX = 5;
// clang-format off
float vertices[] = {
     // position                  // texture coords
     0.5f,  0.5f, 0.0f,           1.0f, 1.0f,  // top right
     0.5f, -0.5f, 0.0f,           1.0f, 0.0f,  // bottom right
    -0.5f, -0.5f, 0.0f,           0.0f, 0.0f,  // bottom left
    -0.5f,  0.5f, 0.0f,           0.0f, 1.0f,  // top left     
};

u32 indices[] = {
  0, 1, 3,
  1, 2, 3
};

// clang-format on
inline void SetMat4(u32 target, mat4 v) {
  glUniformMatrix4fv(target, 1, GL_TRUE, v.values);
}

inline void SetV4(u32 target, f32 x, f32 y, f32 z, f32 w) {
  glUniform4f(target, x, y, z, w);
}

inline void SetV4(u32 target, v4 c) { glUniform4f(target, c.x, c.y, c.z, c.w); }

GLint program;
u32 vbo;
u32 vao;
u32 ebo;
u32 viewLocation;
u32 projectionLocation;
u32 colorLocation;
u32 radiusLocation;

enum Shape { TextureShape = 1, SquareShape, CircleShape };
u32 shapeLocation;

FontData regularFont;

struct Texture {
  v2 size;
  u32 id;
};

void DrawTextureBottomLeft(v2 screen, Texture tex) {
  mat4 view = Mat4Identity();

  v2 scale = vec2(tex.size.x, tex.size.y);

  view = Mat4ScaleV3f(view, vec3(scale.x, scale.y, 1));
  view = Mat4Translate(view, screen.x, screen.y, 0);
  SetMat4(viewLocation, view);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBindTexture(GL_TEXTURE_2D, tex.id);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void DrawLabel(v2 screen, FontData *font, char *label, i32 len) {
  screen.x += font->textures['W'].width / 2.0f;
  f32 startX = screen.x;
  for (i32 i = 0; i < len; i++) {
    int code = label[i];
    if (code == '\n') {
      screen.y -= font->charHeight * 1.2;
      screen.x = startX;
    } else {
      u32 texture = font->openglTextureIds[code];
      MyBitmap *bitmap = &font->textures[code];
      Texture tex = {.size = vec2(bitmap->width, bitmap->height),
                     .id = texture};
      DrawTextureBottomLeft(screen, tex);
      screen.x += bitmap->width;
    }
  }
}

void DrawRectWorld(v2 pos, v2 size, v4 color) {
  SetV4(colorLocation, color);
  SetMat4(viewLocation, Mat4TranslateAndScale(pos, size));
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void UpdateAndDraw(f32 lastFrameMs) {
  mat4 projection = CreateScreenProjection(screen.x, screen.y);
  SetMat4(projectionLocation, projection);

  glClearColor(0.1, 0.1, 0.1, 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUniform1i(shapeLocation, SquareShape);

  CharBuffer buff = {};
  Append(&buff, "AppTime ");
  AppendLine(&buff, appTime);
  Append(&buff, "FPS ");
  AppendLine(&buff, lastFrameMs == 0 ? 0 : (i32)(1000.0f / lastFrameMs));

  glUniform1i(shapeLocation, TextureShape);
  DrawLabel(vec2(20, screen.y - 20), &regularFont, buff.content, buff.len);

  SwapBuffers(dc);
}

void ShowPage(HWND window, v2 pos, v2 size, const wchar_t *url) {
  static ComPtr<ICoreWebView2Controller> controller;
  static ComPtr<ICoreWebView2> webview;

  // Create environment
  CreateCoreWebView2EnvironmentWithOptions(
      nullptr, nullptr, nullptr,
      Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
          [window, pos, size, url](HRESULT result,
                                   ICoreWebView2Environment *env) -> HRESULT {
            // Create WebView
            env->CreateCoreWebView2Controller(
                window,
                Callback<
                    ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                    [window, pos, size,
                     url](HRESULT result,
                          ICoreWebView2Controller *ctrl) -> HRESULT {
                      if (!ctrl)
                        return E_FAIL;
                      controller = ctrl;
                      controller->get_CoreWebView2(&webview);

                      // Set bounds
                      RECT bounds;
                      bounds.left = (LONG)pos.x;
                      bounds.top = (LONG)pos.y;
                      bounds.right = (LONG)(pos.x + size.x);
                      bounds.bottom = (LONG)(pos.y + size.y);
                      controller->put_Bounds(bounds);
                      auto handler =
                          Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
                              [](HRESULT errorCode,
                                 LPCWSTR resultObjectAsJson) -> HRESULT {
                                if (SUCCEEDED(errorCode)) {
                                  // Result is JSON-encoded, e.g. "\"foo\"" for
                                  // a string.
                                  OutputDebugStringW(
                                      L"Script executed successfully\n");
                                } else {
                                  OutputDebugStringW(
                                      L"Script execution failed\n");
                                }
                                return S_OK;
                              });

                      // Navigate
                      //
                      webview->ExecuteScript(L"const a = {foo:42}; a;",
                                             handler.Get());
                      webview->Navigate(url);

                      return S_OK;
                    })
                    .Get());
            return S_OK;
          })
          .Get());
}

int WINAPI wWinMain([[maybe_unused]] HINSTANCE hInstance,
                    [[maybe_unused]] HINSTANCE hPrevInstance,
                    [[maybe_unused]] PWSTR pCmdLine,
                    [[maybe_unused]] int nCmdShow) {
  SetProcessDPIAware();

  win = OpenWindow(OnEvent);

  dc = GetDC(win);
  Win32InitOpenGL(dc);
  InitFunctions();

  if (isFullscreen)
    SetFullscreen(win, isFullscreen);

  ShowWindow(win, SW_SHOW);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  // glSwapControl(0);

  glGenBuffers(1, &vbo);
  glGenVertexArrays(1, &vao);

  glGenBuffers(1, &ebo);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
               GL_STATIC_DRAW);

  glBindVertexArray(vao);

  size_t stride = POINTS_PER_VERTEX * sizeof(float);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void *)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride,
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  program = CreateProgram("vert.glsl", "frag.glsl");
  projectionLocation = glGetUniformLocation(program, "projection");
  viewLocation = glGetUniformLocation(program, "view");
  colorLocation = glGetUniformLocation(program, "color");
  shapeLocation = glGetUniformLocation(program, "shape");
  radiusLocation = glGetUniformLocation(program, "radius");
  glUseProgram(program);

  i64 freq = GetPerfFrequency();
  appTime = 0;
  i64 appStart = GetPerfCounter();
  i64 frameStart = appStart;

  frameTime = 0;
  isRunning = 1;

  CreateFont(&regularFont, 14, "Consolas");
  ShowPage(win, vec2(200, 200), vec2(1000, 1000),
           L"https://www.youtube.com/watch?v=FvHsR0SRTOs");

  while (isRunning) {
    MSG msg;

    while (PeekMessageW(&msg, 0, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    UpdateAndDraw(frameTime);

    mouseButtonsDown[0] = 0;
    mouseButtonsDown[1] = 0;
    keysPressedThisFrame = 0;
    mouseButtonsPressedThisFrame = 0;

    i64 frameEnd = GetPerfCounter();
    frameTime = (f32)(frameEnd - frameStart) / (f32)freq * 1000.0f;
    frameStart = frameEnd;
    appTime = (f32)(frameEnd - appStart) / (f32)freq * 1000.0f;

    memset(keysDown, 0, ArrayLength(keysDown) * sizeof(keysDown[0]));
  }

  return 0;
}
