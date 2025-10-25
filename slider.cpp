#pragma once
#include "win32.cpp"

//
// Math
//

f32 lerp(f32 from, f32 to, f32 v) {
  return (1 - v) * from + to * v;
}

f32 Min(f32 a, f32 b) {
  return a < b ? a : b;
}

f32 Max(f32 a, f32 b) {
  return a > b ? a : b;
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

inline void DrawAppTexture(MyBitmap* canvas, MyBitmap* sourceT, i32 offsetX, i32 offsetY) {
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

// void PrintLine(MyBitmap* canvas, FontData* font, i32 x, i32 y, wchar_t* text, i32 len,
//                i32 pixelSize) {
//   for (i32 i = 0; i < len; i++) {
//     wchar_t ch = text[i];
//     MyBitmap chTexture = font->textures[ch];
//     for (i32 textureY = 0; textureY < (i32)chTexture.height; textureY++) {
//       for (i32 textureX = 0; textureX < (i32)chTexture.width; textureX++) {
//         u32 color = chTexture.pixels[textureX + chTexture.width * (chTexture.height - textureY)];
//         if (color != 0)
//           SetPixel(canvas, x + textureX * pixelSize, y + textureY * pixelSize, color, pixelSize);
//       }
//     }
//     x += chTexture.width * pixelSize;
//   }
// }
//
// void PrintLineEmulated(MyBitmap* canvas, FontData* font, i32 x, i32 y, wchar_t* text, i32 len,
//                        i32 pixelSize) {
//   for (i32 i = 0; i < len; i++) {
//     wchar_t ch = text[i];
//     MyBitmap chTexture = font->textures[ch];
//     for (i32 textureY = 0; textureY < (i32)chTexture.height; textureY++) {
//       for (i32 textureX = 0; textureX < (i32)chTexture.width; textureX++) {
//         u32 color = chTexture.pixels[textureX + chTexture.width * (chTexture.height - textureY)];
//         if (color != 0)
//           SetPixel(canvas, x + textureX * pixelSize, y + textureY * pixelSize, color, pixelSize);
//       }
//     }
//     x += chTexture.width * pixelSize;
//   }
// }
//
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

float Map(float value, float fromMin, float fromMax, float toMin, float toMax) {
  return clamp(toMin + (value - fromMin) * (toMax - toMin) / (fromMax - fromMin), toMin, toMax);
}

void PaintSquareCentered(MyBitmap* canvas, i32 x, i32 y, f32 size, u32 color) {
  PaintRect(canvas, x - size / 2.0, y - size / 2.0, size, size, color);
}

//
//
// Color Picker
//

#define SLIDER_WIDTH 500
#define SLIDER_HEIGHT 300
#define SLIDER_PADDING 20
#define PICKER_HEIGHT 15
#define DRAG_HANDLE_SIZE 30

struct Slider {
  v2 pos;
  v3* color;
  wchar_t* label;
};

struct MouseState {
  v2 pos;
  v2 pressedAt;
  i32 isDown;
};

inline bool InPointInsideRect(v2 point, v2 rectPos, v2 rectSize) {
  return (point.x >= rectPos.x && point.x <= rectPos.x + rectSize.x && point.y >= rectPos.y &&
          point.y <= rectPos.y + rectSize.y);
}

// inline void DrawSlider(MyBitmap* canvas, Slider& slider, FontData* font, MouseState* mouse) {
//   PaintRectA(canvas, slider.pos.x, slider.pos.y, SLIDER_WIDTH, SLIDER_HEIGHT, 0xffffff, 0.1);
//   f32 sliderPart = SLIDER_HEIGHT / 4.0f;
//
//   PrintLine(canvas, font, slider.pos.x + SLIDER_WIDTH / 2.0f - 50, slider.pos.y, slider.label,
//             wstrlen(slider.label), 1);
//
//   for (i32 i = 0; i < 3; i++) {
//     f32* sliderColor;
//     if (i == 0)
//       sliderColor = &slider.color->x;
//     if (i == 1)
//       sliderColor = &slider.color->y;
//     if (i == 2)
//       sliderColor = &slider.color->z;
//
//     v2 sliderSize = vec2(SLIDER_WIDTH - SLIDER_PADDING * 2, PICKER_HEIGHT);
//     v2 sliderStart = vec2(slider.pos.x + SLIDER_PADDING,
//                           slider.pos.y + sliderPart * (i + 1) - PICKER_HEIGHT / 2.0f);
//
//     f32 sliderXMin = sliderStart.x;
//     f32 sliderXMax = sliderStart.x + SLIDER_WIDTH - SLIDER_PADDING * 2;
//
//     f32 sliderXPos;
//
//     if (mouse->isDown && InPointInsideRect(mouse->pressedAt, sliderStart, sliderSize))
//       *sliderColor = Map(mouse->pos.x, sliderXMin, sliderXMax, 0, 255.0f);
//
//     sliderXPos = Map(*sliderColor, 0, 255, sliderXMin, sliderXMax);
//
//     for (i32 y = sliderStart.y; y < sliderStart.y + sliderSize.y; y++) {
//       for (i32 x = sliderStart.x; x < sliderStart.x + sliderSize.x; x++) {
//         u32 color = Map(x, sliderStart.x, sliderStart.x + sliderSize.x, 0, 255);
//
//         u32 heightColor;
//         if (i == 0)
//           heightColor = FromRGB(color, slider.color->y, slider.color->z);
//         if (i == 1)
//           heightColor = FromRGB(slider.color->x, color, slider.color->z);
//         if (i == 2)
//           heightColor = FromRGB(slider.color->x, slider.color->y, color);
//
//         SetPixel(canvas, x, y, heightColor, 1);
//       }
//     }
//
//     u32 dragHandleColor = FromRGB(slider.color->x, slider.color->y, slider.color->z);
//     PaintSquareCentered(canvas, sliderXPos, sliderStart.y + sliderSize.y / 2.0, DRAG_HANDLE_SIZE,
//                         dragHandleColor);
//
//     CharBuffer buff = {};
//     Append(&buff, (i32)*sliderColor);
//
//     PrintLine(canvas, font, sliderStart.x + SLIDER_WIDTH - SLIDER_PADDING + 10,
//               sliderStart.y + sliderSize.y / 2.0f - font->charHeight / 2.0f, buff.content,
//               buff.len, 1);
//   }
// }
