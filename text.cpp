#pragma once
#include "win32.cpp"

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

i32 GetTextWidth(HDC dc, wchar_t* text, i32 from, i32 to) {
  SIZE s2;
  GetTextExtentPoint32W(dc, text + from, to - from, &s2);
  return s2.cx;
}
