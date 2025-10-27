#pragma once
#include "win32.cpp"
#include "drawing.cpp"

HFONT segoe;
v2 pos = {300, 300};

void Init() {
  segoe = CreateAppFont(L"Segoe UI", FW_NORMAL, 40);
}

void OnKeyPress(u32 code) {
  if (code == 'W')
    pos.x += 20;
}

void Draw() {
  v3 white = {1, 1, 1};
  PaintRect(20, 20, 200, 200, white);
  PrintText(segoe, pos.x, pos.y, (wchar_t*)L"Hello", 5);
}
