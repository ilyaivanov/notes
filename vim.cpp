#pragma once
#include "slider.cpp"
#include "win32.cpp"
#include "text.cpp"

struct LineBreak {
  // Soft line break is a text overflow break due to a limited screen width.
  // Non-soft (hard) line break is via \n
  i8 isSoft;
  i32 textPos;
};

struct Buffer {
  c16* text;
  i32 textLen;
  i32 textCapacity;

  LineBreak* lines;
  i32 linesLen;
  i32 linesCapacity;

  i32 cursor;
  f32 desiredOffset;
};

void InsertCharAt(Buffer& b, i32 at, c16 ch) {

  for (i32 i = b.textLen; i > at; i--) {
    b.text[i] = b.text[i - 1];
  }
  b.text[at] = ch;
  b.textLen++;
}

i32 FindLineStart(Buffer& b, i32* lineIndex = 0) {
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
  b.desiredOffset = GetTextWidth(dc, b.text, FindLineStart(b), b.cursor);
}

i32 ClampCursor(Buffer& b, i32 pos) {
  return clamp(pos, 0, b.textLen);
}

void MoveRight(Buffer& b, HDC dc) {
  b.cursor = ClampCursor(b, b.cursor + 1);
  UpdateDesiredOffset(b, dc);
}

void MoveLeft(Buffer& b, HDC dc) {
  b.cursor = ClampCursor(b, b.cursor - 1);
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
  FindLineStart(b, &currentLineIndex);
  i32 nextLineStart = b.lines[currentLineIndex + 1].textPos;

  if (currentLineIndex < b.linesLen - 2)
    b.cursor = ClampCursor(b, nextLineStart +
                                  FindLineOffsetByDistance(b, dc, nextLineStart, b.desiredOffset));
}

void MoveUp(Buffer& b, HDC dc) {
  i32 currentLineIndex = 0;
  FindLineStart(b, &currentLineIndex);
  i32 prevLineStart = 0;
  if (currentLineIndex > 0)
    prevLineStart = b.lines[currentLineIndex - 1].textPos;

  b.cursor = ClampCursor(b, prevLineStart +
                                FindLineOffsetByDistance(b, dc, prevLineStart, b.desiredOffset));
}

i32 IsWhitespace(c16 ch) {
  return ch == L' ' || ch == L'\n';
}

void JumpWordForward(Buffer& b) {
  if (IsWhitespace(b.text[b.cursor])) {
    while (IsWhitespace(b.text[b.cursor]) && b.cursor <= b.textLen)
      b.cursor++;
  } else {
    while (!IsWhitespace(b.text[b.cursor]) && b.cursor <= b.textLen)
      b.cursor++;
  }
  if (b.text[b.cursor] == '\n')
    b.cursor++;

  while (b.text[b.cursor] == ' ' && b.cursor <= b.textLen)
    b.cursor++;
  b.cursor = ClampCursor(b, b.cursor);
}
