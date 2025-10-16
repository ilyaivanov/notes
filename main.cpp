#define UNICODE
#define _UNICODE
#define WIN32_LEAN_AND_MEAN

// #define USE_OPENGL
#include "win32.cpp"
#include "font.cpp"

f32 appTime;
f32 frameTime;

HWND win;
HDC dc;

i32 isRunning = 0;
i32 isFullscreen = 0;
v2 screen;
FontData regularFont;

BITMAPINFO bitmapInfo;
MyBitmap canvas;

// u32 ToWinColor(u32 color) {
//   return ((color & 0xff0000) >> 16) | (color & 0x00ff00) | ((color & 0x0000ff) << 16);
// }

//
// Math
//

f32 lerp(f32 from, f32 to, f32 v) {
  return (1 - v) * from + to * v;
}

inline u8 RoundU8(f32 v) {
  return (u8)(v + 0.5);
}

inline u32 AlphaBlendColors(u32 from, u32 to, f32 factor) {
  u8 fromR = (u8)((from & 0xff0000) >> 16);
  u8 fromG = (u8)((from & 0x00ff00) >> 8);
  u8 fromB = (u8)((from & 0x0000ff) >> 0);

  u8 toR = (u8)((to & 0xff0000) >> 16);
  u8 toG = (u8)((to & 0x00ff00) >> 8);
  u8 toB = (u8)((to & 0x0000ff) >> 0);

  u8 blendedR = RoundU8(lerp(fromR, toR, factor));
  u8 blendedG = RoundU8(lerp(fromG, toG, factor));
  u8 blendedB = RoundU8(lerp(fromB, toB, factor));

  return (blendedR << 16) | (blendedG << 8) | (blendedB << 0);
}
inline u32 AlphaBlendGreyscale(u32 destination, u8 source, u32 color) {
  u8 destR = (u8)((destination & 0xff0000) >> 16);
  u8 destG = (u8)((destination & 0x00ff00) >> 8);
  u8 destB = (u8)((destination & 0x0000ff) >> 0);

  u8 sourceR = (u8)((color & 0xff0000) >> 16);
  u8 sourceG = (u8)((color & 0x00ff00) >> 8);
  u8 sourceB = (u8)((color & 0x0000ff) >> 0);

  f32 a = ((f32)source) / 255.0f;
  u8 blendedR = RoundU8(lerp(destR, sourceR, a));
  u8 blendedG = RoundU8(lerp(destG, sourceG, a));
  u8 blendedB = RoundU8(lerp(destB, sourceB, a));

  return (blendedR << 16) | (blendedG << 8) | (blendedB << 0);
}
inline void CopyMonochromeTextureRectTo(const MyBitmap* canvas, [[maybe_unused]] const Rect* rect,
                                        MyBitmap* sourceT, i32 offsetX, i32 offsetY,
                                        [[maybe_unused]] u32 color) {
  u32* row = (u32*)canvas->pixels + offsetX + offsetY * canvas->width;
  u32* source = (u32*)sourceT->pixels + sourceT->width * (sourceT->height - 1);
  for (i32 y = 0; y < (i32)sourceT->height; y += 1) {
    u32* pixel = row;
    u32* sourcePixel = source;
    for (i32 x = 0; x < (i32)sourceT->width; x += 1) {
      // stupid fucking logic needs to extracted outside of the loop
      // if (*sourcePixel != 0 && (y + offsetY) > rect->y && (x + offsetX) > rect->x &&
      //     (x + offsetX) < (rect->x + rect->width) && (y + offsetY) < (rect->y + rect->height))
      *pixel = *sourcePixel;
      //   *pixel = AlphaBlendGreyscale(*pixel, *sourcePixel, color);

      sourcePixel += 1;
      pixel += 1;
    }
    source -= sourceT->width;
    row += canvas->width;
  }
}

void PaintRect(MyBitmap* bitmap, i32 x, i32 y, i32 width, i32 height, u32 color) {
  for (i32 i = x; i < x + width; i++) {
    for (i32 j = y; j < y + height; j++) {
      if (i >= 0 && i < (i32)bitmap->width && j < (i32)bitmap->height && j >= 0)
        bitmap->pixels[j * bitmap->width + i] = color;
    }
  }
}

void PrintFrameStats(f32 lastFrameMs) {
  CharBuffer buff = {};
  Append(&buff, "Frame: ");
  if (lastFrameMs == 0)
    AppendLine(&buff, 0);
  else
    AppendLine(&buff, (i32)(1000.0f / lastFrameMs));

  Rect rect = {0, 0, (i32)screen.x, (i32)screen.y};
  i32 x = 20;
  i32 y = 20;

  for (i32 i = 0; i < buff.len; i++) {
    wchar_t c = buff.content[i];
    CopyMonochromeTextureRectTo(&canvas, &rect, &regularFont.textures[c], x, y, 0xffffff);
    x += regularFont.charWidth;
  }
}

void UpdateAndDraw(f32 lastFrameMs) {
  PrintFrameStats(lastFrameMs);

  PaintRect(&canvas, 20, 200, 200, 200, 0xff2222);

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
    break;
  case WM_KEYUP:
    break;
  case WM_MOUSEMOVE:
    break;
  case WM_LBUTTONDOWN:
    break;
  case WM_RBUTTONDOWN:
    break;
  case WM_LBUTTONUP:
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
  SetProcessDPIAware();

  win = OpenWindow(OnEvent);

  dc = GetDC(win);

  if (isFullscreen)
    SetFullscreen(win, isFullscreen);

  ShowWindow(win, SW_SHOW);

  i64 freq = GetPerfFrequency();
  appTime = 0;
  i64 appStart = GetPerfCounter();
  i64 frameStart = appStart;

  frameTime = 0;

  CreateFont(&regularFont, 14, "Consolas");

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

  ExitProcess(0);
}
