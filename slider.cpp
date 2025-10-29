#pragma once
#include "win32.cpp"

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

void PaintRect(MyBitmap* bitmap, i32 x, i32 y, i32 width, i32 height, u32 color) {
  for (i32 i = x; i < x + width; i++) {
    for (i32 j = y; j < y + height; j++) {
      if (i >= 0 && i < (i32)bitmap->width && j < (i32)bitmap->height && j >= 0)
        bitmap->pixels[j * bitmap->width + i] = color;
    }
  }
}

void PaintRect(MyBitmap* bitmap, i32 x, i32 y, i32 width, i32 height, v3 color) {
  u32 r = (u32)(color.x * 255.0);
  u32 g = (u32)(color.y * 255.0);
  u32 b = (u32)(color.z * 255.0);

  u32 c = r << 16 | g << 8 | b << 0;
  for (i32 i = x; i < x + width; i++) {
    for (i32 j = y; j < y + height; j++) {
      if (i >= 0 && i < (i32)bitmap->width && j < (i32)bitmap->height && j >= 0)
        bitmap->pixels[j * bitmap->width + i] = c;
    }
  }
}

void PaintRectA(MyBitmap* bitmap, i32 x, i32 y, i32 width, i32 height, u32 color, float alpha) {
  for (i32 i = x; i < x + width; i++) {
    for (i32 j = y; j < y + height; j++) {
      if (i >= 0 && i < (i32)bitmap->width && j < (i32)bitmap->height && j >= 0) {
        i32 p = j * bitmap->width + i;
        bitmap->pixels[p] = AlphaBlendColors(bitmap->pixels[p], color, alpha);
      }
    }
  }
}

u32 FromRGB(u32 r, u32 g, u32 b) {
  return u32(r) << 16 | u32(g) << 8 | u32(b) << 0;
}

v3 ToRGB(u32 color) {
  v3 res = {};
  res.x = (f32)((color & 0xff0000) >> 16);
  res.y = (f32)((color & 0x00ff00) >> 8);
  res.z = (f32)((color & 0x0000ff) >> 0);
  return res;
}

inline void SetPixel(MyBitmap* canvas, i32 pixelX, i32 pixelY, u32 color, i32 pixelSize) {
  for (i32 y = 0; y < pixelSize; y++)
    for (i32 x = 0; x < pixelSize; x++)
      canvas->pixels[pixelX + x + canvas->width * (pixelY + y)] = color;
}

inline v3 GetPixel(MyBitmap* canvas, i32 pixelX, i32 pixelY) {
  return ToRGB(canvas->pixels[pixelX + canvas->width * (pixelY)]);
}

i32 wstrlen(wchar_t* str) {
  i32 res = 0;
  while (str[res] != L'\0')
    res++;
  return res;
}

i32 round(f32 f) {
  return (i32)(f + 0.5);
}

f32 clamp(f32 v, f32 min, f32 max) {
  if (v < min)
    return min;
  if (v > max)
    return max;
  return v;
}

i32 clamp(i32 v, i32 min, i32 max) {
  if (v < min)
    return min;
  if (v > max)
    return max;
  return v;
}

float Map(float value, float fromMin, float fromMax, float toMin, float toMax) {
  return clamp(toMin + (value - fromMin) * (toMax - toMin) / (fromMax - fromMin), toMin, toMax);
}

void PaintSquareCentered(MyBitmap* canvas, i32 x, i32 y, f32 size, u32 color) {
  PaintRect(canvas, x - size / 2.0, y - size / 2.0, size, size, color);
}

inline bool InPointInsideRect(v2 point, v2 rectPos, v2 rectSize) {
  return (point.x >= rectPos.x && point.x <= rectPos.x + rectSize.x && point.y >= rectPos.y &&
          point.y <= rectPos.y + rectSize.y);
}
