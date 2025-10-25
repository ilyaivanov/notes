#pragma once
#include "win32.cpp"
#include "text.cpp"

struct LineBreak {
  // Soft line break is a text overflow break due to a limited screen width.
  // Non-soft (hard) line break is via \n
  i8 isSoft;
  i32 textPos;
};

struct Buffer {
  wchar_t* text;
  i32 textLen;
  i32 textCapacity;

  LineBreak* lines;
  i32 linesLen;
  i32 linesCapacity;

  i32 cursor;
  f32 desiredOffset;
};

i32 FindLineStart(Buffer& b, HDC dc, i32* lineIndex = 0) {
  for (i32 i = 0; i < b.linesLen - 1; i++) {
    i32 start = b.lines[i].textPos;
    i32 end = b.lines[i + 1].textPos;
    if (b.cursor >= start && b.cursor < end) {
      if (lineIndex)
        *lineIndex = i;
      return start;
    }
  }
  return 0;
}

void UpdateDesiredOffset(Buffer& b, HDC dc) {
  b.desiredOffset = GetTextWidth(dc, b.text, FindLineStart(b, dc), b.cursor);
}

void MoveRight(Buffer& b, HDC dc) {
  b.cursor++;
  UpdateDesiredOffset(b, dc);
}

void MoveLeft(Buffer& b, HDC dc) {
  b.cursor--;
  UpdateDesiredOffset(b, dc);
}

i32 FindLineOffsetByDistance(Buffer& b, HDC dc, i32 lineStart, f32 distanceFromLineStart) {
  i32 offset = 0;
  for (i32 i = lineStart; i <= b.textLen; i++) {
    if (b.text[i] == '\n')
      break;
    if (GetTextWidth(dc, b.text, lineStart, i) >= distanceFromLineStart) {
      if (abs(GetTextWidth(dc, b.text, lineStart, i) - distanceFromLineStart) >
          abs(GetTextWidth(dc, b.text, lineStart, i - 1) - distanceFromLineStart))
        offset--;

      break;
    }
    offset++;
  }
  return offset;
}

void MoveDown(Buffer& b, HDC dc) {
  i32 currentLineIndex = 0;
  FindLineStart(b, dc, &currentLineIndex);
  i32 nextLineStart = b.lines[currentLineIndex + 1].textPos;

  b.cursor = nextLineStart + FindLineOffsetByDistance(b, dc, nextLineStart, b.desiredOffset);
}

void MoveUp(Buffer& b, HDC dc) {
  i32 currentLineIndex = 0;
  FindLineStart(b, dc, &currentLineIndex);
  i32 prevLineStart = 0;
  if (currentLineIndex > 0)
    prevLineStart = b.lines[currentLineIndex - 1].textPos;

  b.cursor = prevLineStart + FindLineOffsetByDistance(b, dc, prevLineStart, b.desiredOffset);
}
