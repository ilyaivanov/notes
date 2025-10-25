#pragma once

#include "win32.cpp"
#include <wingdi.h>
#ifdef USE_OPENGL
#include <gl/gl.h>
#endif
#include <windows.h>

#define MAX_CHAR_CODE 200

typedef struct FontData {
  MyBitmap textures[MAX_CHAR_CODE];
#ifdef USE_OPENGL
  GLuint openglTextureIds[MAX_CHAR_CODE];
#endif

  // assumes monospaced font
  i32 charWidth;

  i32 charHeight;

  TEXTMETRIC textMetric;
  v3 foreground;
  v3 background;
} FontData;

#define TRANSPARENT_R 0x0
#define TRANSPARENT_G 0x0
#define TRANSPARENT_B 0x0

// takes dimensions of destinations, reads rect from source at (0,0)
inline void CopyRectTo(MyBitmap* sourceT, MyBitmap* destination) {
  u32* row = (u32*)destination->pixels + destination->width * (destination->height - 1);
  u32* source = (u32*)sourceT->pixels + sourceT->width * (sourceT->height - 1);
  for (u32 y = 0; y < destination->height; y += 1) {
    u32* pixel = row;
    u32* sourcePixel = source;
    for (u32 x = 0; x < destination->width; x += 1) {
      u8 r = (*sourcePixel & 0xff0000) >> 16;
      u8 g = (*sourcePixel & 0x00ff00) >> 8;
      u8 b = (*sourcePixel & 0x0000ff) >> 0;
      u32 alpha = (r == TRANSPARENT_R && g == TRANSPARENT_G && b == TRANSPARENT_B) ? 0x00000000
                                                                                   : 0xff000000;
      *pixel = *sourcePixel | alpha;
      sourcePixel += 1;
      pixel += 1;
    }
    source -= sourceT->width;
    row -= destination->width;
  }
}

HFONT CreateAppFont(HDC dc, const wchar_t* name, i32 weight, i32 fontSize) {
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
                           CLEARTYPE_QUALITY,

                           DEFAULT_PITCH, name);
  return font;
}

void FillVals(i32* vals, const wchar_t* fontName, i32 fontSize) {
  HDC deviceContext = CreateCompatibleDC(0);

  HFONT font = CreateAppFont(deviceContext, fontName, FW_DONTCARE, fontSize);

  BITMAPINFO info = {};
  u32 textureSize = 30;
  info.bmiHeader.biSize = sizeof(info.bmiHeader);
  info.bmiHeader.biBitCount = 32;
  info.bmiHeader.biWidth = textureSize;
  info.bmiHeader.biHeight = textureSize;
  info.bmiHeader.biPlanes = 1;
  info.bmiHeader.biCompression = BI_RGB;

  void* bits;
  HBITMAP bitmap = CreateDIBSection(deviceContext, &info, DIB_RGB_COLORS, &bits, 0, 0);
  MyBitmap fontCanvas = {
      .width = textureSize, .height = textureSize, .bytesPerPixel = 4, .pixels = (u32*)bits};

  SelectObject(deviceContext, bitmap);
  SelectObject(deviceContext, font);

  // TRANSPARENT still leaves 00 as alpha value for non-trasparent pixels. I
  // love GDI SetBkColor(deviceContext, TRANSPARENT);

  // SIZE size;
  for (i32 i = 0; i <= 255; i++) {
    SetBkColor(deviceContext, RGB(0, 0, i));
    SetTextColor(deviceContext, RGB(255, 255, 255));
    // GetTextExtentPoint32W(deviceContext, &ch, len, &size);

    TextOutW(deviceContext, 0, 0, L"l", 1);
    u32 color = fontCanvas.pixels[1 + 6 * fontCanvas.width];
    [[maybe_unused]] u32 r = (color & 0xff0000) >> 16;
    [[maybe_unused]] u32 g = (color & 0x00ff00) >> 8;
    [[maybe_unused]] u32 b = (color & 0x0000ff) >> 0;
    vals[i] = b;
  }

  DeleteObject(bitmap);
  DeleteObject(font);
  DeleteDC(deviceContext);
}

void InitFont(FontData* fontData, int fontSize, const wchar_t* fontName, DWORD weight,
              v3 foreground, v3 background) {
  fontData->foreground = foreground;
  fontData->background = background;

  HDC deviceContext = CreateCompatibleDC(0);

  HFONT font = CreateAppFont(deviceContext, fontName, weight, fontSize);

  BITMAPINFO info = {};
  u32 textureSize = 512;
  info.bmiHeader.biSize = sizeof(info.bmiHeader);
  info.bmiHeader.biBitCount = 32;
  info.bmiHeader.biWidth = textureSize;
  info.bmiHeader.biHeight = textureSize;
  info.bmiHeader.biPlanes = 1;
  info.bmiHeader.biCompression = BI_RGB;

  void* bits;
  HBITMAP bitmap = CreateDIBSection(deviceContext, &info, DIB_RGB_COLORS, &bits, 0, 0);
  MyBitmap fontCanvas = {
      .width = textureSize, .height = textureSize, .bytesPerPixel = 4, .pixels = (u32*)bits};

  SelectObject(deviceContext, bitmap);
  SelectObject(deviceContext, font);

  foreground *= 255.0;
  background *= 255.0;
  // TRANSPARENT still leaves 00 as alpha value for non-trasparent pixels. I
  // love GDI SetBkColor(deviceContext, TRANSPARENT);
  SetBkColor(deviceContext,
             RGB(background.x, background.y,
                 background.z)); // RGB(TRANSPARENT_R, TRANSPARENT_G, TRANSPARENT_B));
  SetTextColor(deviceContext, RGB(foreground.x, foreground.y, foreground.z));

  SIZE size;
  for (wchar_t ch = 32; ch < MAX_CHAR_CODE; ch += 1) {
    int len = 1;
    GetTextExtentPoint32W(deviceContext, &ch, len, &size);

    TextOutW(deviceContext, 0, 0, &ch, len);

    MyBitmap* texture = &fontData->textures[ch];
    texture->width = size.cx;
    texture->height = size.cy;
    texture->bytesPerPixel = 4;

    if (texture->pixels)
      vfree(texture->pixels);

    texture->pixels = (u32*)valloc(texture->height * texture->width * texture->bytesPerPixel);

    CopyRectTo(&fontCanvas, texture);
  }

  GetTextMetrics(deviceContext, &fontData->textMetric);

  fontData->charHeight = fontData->textMetric.tmHeight;
  fontData->charWidth = fontData->textures['W'].width;
  DeleteObject(bitmap);
  DeleteObject(font);
  DeleteDC(deviceContext);
}

#ifdef USE_OPENGL
void CreateFontTexturesForOpenGl(FontData* font) {
  glGenTextures(MAX_CHAR_CODE, font->openglTextureIds);
  for (u32 i = ' '; i < MAX_CHAR_CODE; i++) {
    MyBitmap* bit = &font->textures[i];
    glBindTexture(GL_TEXTURE_2D, font->openglTextureIds[i]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, bit->width, bit->height, 0, GL_BGRA_EXT,
                 GL_UNSIGNED_BYTE, bit->pixels);
  }
}
#endif

void CreateFont(FontData* font, i32 size, const wchar_t* name, DWORD weight, v3 foreground,
                v3 background) {
  InitFont(font, size, name, weight, foreground, background);
#ifdef USE_OPENGL
  CreateFontTexturesForOpenGl(font);
#endif
}
