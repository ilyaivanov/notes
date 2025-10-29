#pragma once
#include "win32.cpp"

struct Rect {
  f32 x;
  f32 y;
  f32 width;
  f32 height;
};

Rect ShrinkRect(Rect rect, v2 dim) {
  Rect shrinked = {rect.x + dim.x, rect.y + dim.y, rect.width - dim.x * 2, rect.height - dim.y * 2};

  return shrinked;
}

u32 ColorFromVec(v3 color) {
  u32 r = (u32)(color.x * 255.0);
  u32 g = (u32)(color.y * 255.0);
  u32 b = (u32)(color.z * 255.0);

  return r << 16 | g << 8 | b << 0;
}

u32 ColorFromVec(v4 color) {
  v3 c = {color.x, color.y, color.z};
  return ColorFromVec(c);
}

MyBitmap* currentBitmap;
HDC currentDc;
void PaintRect(i32 x, i32 y, i32 width, i32 height, v3 color) {
  u32 c = ColorFromVec(color);
  for (i32 i = x; i < x + width; i++) {
    for (i32 j = y; j < y + height; j++) {
      if (i >= 0 && i < (i32)currentBitmap->width && j < (i32)currentBitmap->height && j >= 0)
        currentBitmap->pixels[j * currentBitmap->width + i] = c;
    }
  }
}

void PaintRect(Rect rect, v3 color) {
  PaintRect(rect.x, rect.y, rect.width, rect.height, color);
}

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

void PaintRect(Rect rect, v4 c) {
  i32 x = rect.x;
  i32 y = rect.y;
  i32 width = rect.width;
  i32 height = rect.height;

  f32 alpha = c.w;
  u32 color = ColorFromVec(c);
  for (i32 i = x; i < x + width; i++) {
    for (i32 j = y; j < y + height; j++) {
      if (i >= 0 && i < (i32)currentBitmap->width && j < (i32)currentBitmap->height && j >= 0) {
        i32 p = j * currentBitmap->width + i;
        currentBitmap->pixels[p] = AlphaBlendColors(currentBitmap->pixels[p], color, alpha);
      }
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

i32 GetTextWidth(wchar_t* text, i32 from, i32 to) {
  SIZE s2;
  GetTextExtentPoint32W(currentDc, text + from, to - from, &s2);
  return s2.cx;
}

void SetColors(v3 fg, v3 bg) {
  fg *= 255.0f;
  bg *= 255.0f;
  SetBkColor(currentDc, RGB(bg.x, bg.y, bg.z));
  SetTextColor(currentDc, RGB(fg.x, fg.y, fg.z));
}

void SetAlign(u32 align) {
  SetTextAlign(currentDc, align);
}

void UseFont(HFONT font) {
  SelectFont(currentDc, font);
}

void PrintText(i32 x, i32 y, wchar_t* text, i32 size) {
  TextOutW(currentDc, x, y, text, size);
}

f32 GetFontHeight() {
  TEXTMETRIC textMetric;
  GetTextMetrics(currentDc, &textMetric);
  return textMetric.tmAscent + textMetric.tmDescent;
}
