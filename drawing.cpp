#pragma once
#include "win32.cpp"

MyBitmap* currentBitmap;
HDC currentDc;
void PaintRect(i32 x, i32 y, i32 width, i32 height, v3 color) {
  u32 r = (u32)(color.x * 255.0);
  u32 g = (u32)(color.y * 255.0);
  u32 b = (u32)(color.z * 255.0);

  u32 c = r << 16 | g << 8 | b << 0;
  for (i32 i = x; i < x + width; i++) {
    for (i32 j = y; j < y + height; j++) {
      if (i >= 0 && i < (i32)currentBitmap->width && j < (i32)currentBitmap->height && j >= 0)
        currentBitmap->pixels[j * currentBitmap->width + i] = c;
    }
  }
}

HFONT CreateAppFont(const wchar_t* name, i32 weight, i32 fontSize) {
  int h = -MulDiv(fontSize, GetDeviceCaps(currentDc, LOGPIXELSY), USER_DEFAULT_SCREEN_DPI);
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

i32 GetTextWidth(HDC dc, wchar_t* text, i32 from, i32 to) {
  SIZE s2;
  GetTextExtentPoint32W(dc, text + from, to - from, &s2);
  return s2.cx;
}

void PrintText(HFONT font, i32 x, i32 y, wchar_t* text, i32 size) {
  SetBkColor(currentDc, RGB(0, 0, 0));
  SetTextColor(currentDc, RGB(255, 255, 255));
  SelectFont(currentDc, font);
  TextOutW(currentDc, x, y, text, size);
}
