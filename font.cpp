#pragma once

#include "win32.cpp"
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

void InitFont(FontData* fontData, int fontSize, const char* fontName) {
  HDC deviceContext = CreateCompatibleDC(0);

  int h = -MulDiv(fontSize, GetDeviceCaps(deviceContext, LOGPIXELSY), USER_DEFAULT_SCREEN_DPI);
  HFONT font = CreateFontA(h, 0, 0, 0,
                           FW_DONTCARE, // Weight
                           0,           // Italic
                           0,           // Underline
                           0,           // Strikeout
                           DEFAULT_CHARSET, OUT_TT_ONLY_PRECIS, CLIP_DEFAULT_PRECIS,

                           // I've experimented with the Chrome and it doesn't render LCD
                           // quality for fonts above 32px
                           // ANTIALIASED_QUALITY,
                           // I'm experiencing troubles with LCD font rendering and
                           // setting a custom color for a shader
                           CLEARTYPE_QUALITY,

                           DEFAULT_PITCH, fontName);

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

  // TRANSPARENT still leaves 00 as alpha value for non-trasparent pixels. I
  // love GDI SetBkColor(deviceContext, TRANSPARENT);
  SetBkColor(deviceContext, RGB(TRANSPARENT_R, TRANSPARENT_G, TRANSPARENT_B));
  SetTextColor(deviceContext, RGB(255, 255, 255));

  SIZE size;
  for (wchar_t ch = 32; ch < MAX_CHAR_CODE; ch += 1) {
    int len = 1;
    GetTextExtentPoint32W(deviceContext, &ch, len, &size);

    TextOutW(deviceContext, 0, 0, &ch, len);

    MyBitmap* texture = &fontData->textures[ch];
    texture->width = size.cx;
    texture->height = size.cy;
    texture->bytesPerPixel = 4;

    texture->pixels = (u32*)valloc(texture->height * texture->width * texture->bytesPerPixel);

    CopyRectTo(&fontCanvas, texture);
  }

  GetTextMetrics(deviceContext, &fontData->textMetric);

  fontData->charHeight = fontData->textMetric.tmHeight;
  fontData->charWidth = fontData->textures['W'].width;
  DeleteObject(bitmap);
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

void CreateFont(FontData* font, i32 size, const char* name) {
  InitFont(font, size, name);
#ifdef USE_OPENGL
  CreateFontTexturesForOpenGl(font);
#endif
}
