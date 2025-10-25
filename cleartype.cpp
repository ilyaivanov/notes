
#define UNICODE
#define _UNICODE
#define WIN32_LEAN_AND_MEAN
#define USE_SSE2

// later I will move glFunction and glProgram files here and will render everything vie opengl
// I want to be able to have two rendering mechanisms, would be an interesting architectural task
// #define USE_OPENGL
#include "win32.cpp"
#include "font.cpp"
// #include "sincos.cpp"
#include "slider.cpp"

#include "pow.h"
// #include <math.h>

f32 appTime;
f32 frameTime;

HWND win;
HDC dc;
HDC deviceContext;

i32 showStats = 0;
i32 isRunning = 0;
i32 isFullscreen = 0;
v2 screen;

v3 foreground = {255, 255, 255};
v3 background = {0, 20, 30};

FontData consolas;
FontData consolasBig;
FontData segoeWhite;
FontData segoe;
// HFONT title;

i32 cursor;

BITMAPINFO bitmapInfo;
// HBITMAP bitmap;
MyBitmap canvas;

// u32 ToWinColor(u32 color) {
//   return ((color & 0xff0000) >> 16) | (color & 0x00ff00) | ((color & 0x0000ff) << 16);
// }

MouseState mouseState;
Slider bgPicker;
Slider fgPicker;

f32 lineHeight = 1.0;

f32 itemPadding = 10.0;

f32 DrawLabel(i32 x, i32 y, FontData* font, wchar_t* message, i32 len) {
  // Rect rect = {0, 0, (i32)screen.x, (i32)screen.y};

  f32 runningY = (f32)y;
  i32 startX = x;
  for (i32 i = 0; i < len; i++) {
    wchar_t c = message[i];
    if (c == '\n') {
      x = startX;
      runningY += (f32)font->charHeight * lineHeight;
    } else {
      DrawAppTexture(&canvas, &font->textures[c], x, round(runningY));
      x += font->textures[c].width;
    }
  }
  return runningY - y + font->charHeight * lineHeight;
}

void PrintFrameStats(f32 lastFrameMs) {
  CharBuffer buff = {};
  Append(&buff, L"Frame: ");
  if (lastFrameMs == 0)
    AppendLine(&buff, 0);
  else
    AppendLine(&buff, (i32)(1000.0f / lastFrameMs));

  PrintLine(&canvas, &consolas, 0, 0, buff.content, buff.len, 1);
}

// f32 DrawItemAt(i32 x, i32 y, FontData* font, wchar_t* text, i32 len) {
//   PaintRect(&canvas, x, y + font->charHeight / 2.0f, 5, 5, 0xaaaaaa);
// //   return DrawLabel(x + 10, y, font, text, len);
// }

f32 timeToCursorBlink = 1000;
f32 cursorBlinkStart = 0;

void PaintGraph(i32 x, i32 y, i32 vals[256], u32 color) {
  for (i32 i = 0; i < 256; i++) {
    PaintRect(&canvas, x + i * 2, y - vals[i], 2, vals[i], color);
  }
}

//
v3 GetFocusPixelAt(FontData* font, i32 x, i32 y) {
  MyBitmap tex = font->textures['l'];
  return ToRGB(tex.pixels[x + y * tex.width]);
}

void PrintFocusPixel(i32 x, i32 y, FontData* font) {
  CharBuffer pixel = {};

  AppendLine(&pixel, GetFocusPixelAt(font, 1, 6));
  PrintLine(&canvas, &consolas, x, y, pixel.content, pixel.len, 1);

  pixel.len = 0;
  AppendLine(&pixel, GetFocusPixelAt(font, 3, 6));
  PrintLine(&canvas, &consolas, x + 300, y, pixel.content, pixel.len, 1);
}

f32 q(f32 a) {
  return clamp(0.48535 * a * a + 0.57875 * a - 0.04106, 0, 1.0);
}

// Works for positive base only
float pow2(float base, float exp) {

  if (base == 0)
    return 0;
  // v4sf b = exp_ps(_mm_mul_ps(

  v4sf x = _mm_set_ps(base, 0, 0, 0);
  v4sf y = _mm_set_ps(exp, 0, 0, 0);

  v4sf l = log_ps(x);
  v4sf m = _mm_mul_ps(y, l);
  v4sf res = exp_ps(m);
  float out[4];
  _mm_storeu_ps(out, res);
  return out[3];
}

#define gamma 1.8
f32 tosrgb(f32 a) {
  return pow2(a, gamma);
}

f32 fromsrgb(f32 a) {
  return pow2(a, 1.0 / gamma);
}

void UpdateAndDraw(f32 lastFrameMs) {
  f32 res = pow2(2, 2);
  res = pow2(3, 10.3); // 19950564.9
  timeToCursorBlink -= lastFrameMs;

  mouseState.isDown = GetAsyncKeyState(VK_LBUTTON) & 0b1000000000000000;
  if (foreground != segoe.foreground || background != segoe.background)
    CreateFont(&segoe, 12, L"Segoe UI", FW_NORMAL, foreground, background);

  if (showStats)
    PrintFrameStats(lastFrameMs);

  // PaintRectA(&canvas, 20, 20, 200, 200, 0xffaa22, 0.6);

#ifdef showGraph
  i32 y = 0;
  i32 x = 50;
  //
  wchar_t* str0 = (wchar_t*)L"Title";

  i32 vals[256];
  i32 ress[256];

  FillVals(vals, L"Segoe UI", 16);
  i32 min = vals[0];
  i32 max = vals[255];

  i32 row = 0;
  for (i32 i = 0; i <= 255; i++) {
    CharBuffer buff = {};
    Append(&buff, i);
    Append(&buff, L" - ");
    Append(&buff, vals[i]);
    Append(&buff, L" - ");
    f32 alpha = (f32)i / 255.0f;

    i32 res =
        round(lerp(min, max, clamp(0.48535 * alpha * alpha + 0.57875 * alpha - 0.04106, 0.0, 1.0)));
    // i32 res = round(lerp(min, max, alpha));

    ress[i] = res;
    Append(&buff, res);
    Append(&buff, L" (");
    Append(&buff, res - vals[i]);
    Append(&buff, L")");
    if ((y + (row + 2) * segoe.charHeight) > screen.y) {
      row = 0;
      x += 250;
    }

    PrintLine(&canvas, &segoe, x, y + row * segoe.charHeight, buff.content, buff.len, 1);
    row++;
  }

  PaintGraph(1000, 500, ress, 0xff2222);
  PaintGraph(1000, 500, vals, 0xffffff);
#endif

  // PaintRect(&canvas, 1027, 500, 120, segoe.charHeight, 0x88aabb);
  i32 pixelSize = 25;

  i32 y = screen.y - segoe.charHeight * pixelSize;
  PaintRect(&canvas, 0, y, 1100, segoeWhite.charHeight * pixelSize,
            FromRGB(background.x, background.y, background.z));
  PrintLineEmulated(&canvas, &segoeWhite, 0, y, (wchar_t*)L"Hello", 5, pixelSize);

  PrintLine(&canvas, &segoe, 1300, y, (wchar_t*)L"Hello", 5, pixelSize);

  DrawSlider(&canvas, bgPicker, &consolas, &mouseState);
  DrawSlider(&canvas, fgPicker, &consolas, &mouseState);
  f32 textY = screen.y - segoe.charHeight * pixelSize - consolas.charHeight;
  PrintFocusPixel(0, textY, &segoeWhite);
  PrintFocusPixel(1300, textY, &segoe);
  CharBuffer mousePixel = {};

  Append(&mousePixel, GetPixel(&canvas, mouseState.pos.x, mouseState.pos.y));
  PrintLine(&canvas, &consolas, 1900, textY, mousePixel.content, mousePixel.len, 1);

  CharBuffer finalResultForRed = {};
  f32 bg = background.x / 255.0;
  f32 fg = foreground.x / 255.0;
  // q(bg)* 255 + (fg - q(bg)) * (95 / 255) * 255

  f32 result = 0;
  f32 min = tosrgb(Min(bg, fg));
  f32 max = tosrgb(Max(bg, fg));
  f32 coef = tosrgb(95.0 / 255.0);
  // if (bg < fg) {
  // result = fromsrgb(lerp(min, max, coef)) * 255;
  f32 bgCoef = -0.0039104785 * fg * fg + 0.337508004 * fg + 233.1059112;

  result = 95.0 * fg + bgCoef * bg - 65 * fg * bg;
  // } else {
  //   result = lerp(fg, q(bg), (255.0 - 95.0) / 255.0) * 255;
  // }

  // result = clamp(q(bg) + ((fg)-q(bg)) * (95.0 / 255.0), 0.0, 1.0) * 255;

  Append(&finalResultForRed, result);

  PrintLine(&canvas, &consolasBig, 1600, 1000, finalResultForRed.content, finalResultForRed.len, 1);

  // SelectObject(deviceContext, bitmap);
  // SelectObject(deviceContext, segoe);
  // SetBkColor(deviceContext, RGB(TRANSPARENT_R, TRANSPARENT_G, TRANSPARENT_B));
  // SetTextColor(deviceContext, RGB(255, 255, 255));
  // TextOutW(deviceContext, 10, y, str0, wstrlen(str0));
  // y += DrawItemAt(10, y, &title, str0, wstrlen(str0)) + itemPadding;

  // wchar_t* str1 =
  //     (wchar_t*)L"Hello Wi with quite a long statement from all over the world\nAnd another
  //     line";
  // y += DrawItemAt(10, y, &segoe, str1, wstrlen(str1)) + itemPadding;
  //
  // wchar_t* str2 = (wchar_t*)L"Hello Wi with quite a long statement from all over the world\nAnd "
  //                           L"another line\nand another one";
  // y += DrawItemAt(10, y, &segoe, str2, wstrlen(str2)) + itemPadding;

  // f32 sin;
  // f32 cos;
  // SinCos((appTime - cursorBlinkStart) / 300.0f, &sin, &cos);
  // f32 a = 1;
  // if (timeToCursorBlink <= 0)
  //   a = abs(cos);
  //
  // i32 cursorWidth = 2;
  // i32 cursorStart = 10 + 10 - cursorWidth / 2.0f;

  // for (i32 i = 0; i < cursor; i++)
  //   cursorStart += title.textures[str0[i]].width;
  //
  // PaintRectA(&canvas, cursorStart, 50, cursorWidth, title.charHeight, 0xffffff, a);

  // BitBlt(dc, 0, 0, screen.x, screen.y, deviceContext, 0, 0, SRCCOPY);
  StretchDIBits(dc, 0, 0, screen.x, screen.y, 0, 0, screen.x, screen.y, canvas.pixels, &bitmapInfo,
                DIB_RGB_COLORS, SRCCOPY);
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
    if (wParam == VK_F1)
      showStats = !showStats;
    if (wParam == 'L') {
      cursor++;
      timeToCursorBlink = 600;
      cursorBlinkStart = appTime + timeToCursorBlink;
    }
    if (wParam == 'H') {
      cursor--;
      timeToCursorBlink = 600;
      cursorBlinkStart = appTime + timeToCursorBlink;
    }
    break;
  case WM_KEYUP:
    break;
  case WM_MOUSEMOVE:
    mouseState.pos.x = LOWORD(lParam);
    mouseState.pos.y = HIWORD(lParam);
    break;
  case WM_LBUTTONDOWN:
    mouseState.pressedAt = mouseState.pos;
    mouseState.isDown = 1;
    break;
  case WM_LBUTTONUP:
    mouseState.isDown = 0;
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

    bgPicker.label = (wchar_t*)L"Background";
    bgPicker.pos = vec2(60, 200);
    bgPicker.color = &background;

    fgPicker.label = (wchar_t*)L"Foreground";
    fgPicker.pos = vec2(60, 200 + SLIDER_HEIGHT + 50);
    fgPicker.color = &foreground;
    // if (bitmap)
    //   DeleteBitmap(bitmap);

    // void* bits;
    // bitmap = CreateDIBSection(deviceContext, &bitmapInfo, DIB_RGB_COLORS, &bits, 0, 0);
    // canvas.pixels = (u32*)bits;
    if (canvas.pixels)
      vfree(canvas.pixels);

    canvas.pixels = (u32*)valloc(canvas.width * canvas.height * 4);
    dc = GetDC(win);

    // glViewport(0, 0, screen.x, screen.y);
    if (isRunning)
      UpdateAndDraw(0);
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

  frameTime = 0;

  // segoe = CreateAppFont(dc, L"Segoe UI", FW_NORMAL, 30);
  // consolas = CreateAppFont(dc, L"Consolas", FW_NORMAL, 16);

  v3 white = {255, 255, 255};
  v3 black = {0, 0, 0};
  CreateFont(&consolas, 16, L"Consolas", FW_DONTCARE, white, black);
  CreateFont(&consolasBig, 30, L"Consolas", FW_BOLD, white, black);
  CreateFont(&segoe, 12, L"Segoe UI", FW_NORMAL, foreground, background);
  CreateFont(&segoeWhite, 12, L"Segoe UI", FW_NORMAL, white, black);
  // CreateFont(&title, 24, L"Segoe UI", FW_BOLD);

  isRunning = 1;
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
