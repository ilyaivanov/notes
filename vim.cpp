#pragma once
#include "win32.cpp"
#include "drawing.cpp"

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
  i32 desiredOffset;
};

void InsertCharAt(Buffer& b, i32 at, c16 ch) {
  for (i32 i = b.textLen; i > at; i--) {
    b.text[i] = b.text[i - 1];
  }
  b.text[at] = ch;
  b.textLen++;
}

// one two
void RemoveCharAt(Buffer& b, i32 at) {
  for (i32 i = at; i < b.textLen - 1; i++) {
    b.text[i] = b.text[i + 1];
  }
  b.textLen--;
}

void RemoveChars(Buffer& b, i32 from, i32 to) {
  for (i32 i = to; i < b.textLen - 1; i++) {
    b.text[i + from - to] = b.text[i + 1];
  }
  b.textLen -= to - from + 1;
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

i32 FindLineEnd(Buffer& b, i32* lineIndex = 0) {
  for (i32 i = 0; i < b.linesLen - 1; i++) {
    i32 start = b.lines[i].textPos;
    i32 end = b.lines[i + 1].textPos;
    if (b.cursor >= start && b.cursor < end) {
      if (lineIndex)
        *lineIndex = i + 1;
      return end;
    }
  }
  return -1;
}

void UpdateDesiredOffset(Buffer& b) {
  b.desiredOffset = GetTextWidth(b.text, FindLineStart(b), b.cursor);
}

i32 ClampCursor(Buffer& b, i32 pos) {
  i32 lastPos = b.textLen;
  if (b.text[b.textLen - 1] == '\n')
    lastPos--;
  return clamp(pos, 0, lastPos);
}

void MoveRight(Buffer& b) {
  b.cursor = ClampCursor(b, b.cursor + 1);
}

void MoveLeft(Buffer& b) {
  b.cursor = ClampCursor(b, b.cursor - 1);
}

i32 FindLineOffsetByDistance(Buffer& b, i32 lineStart, f32 distanceFromLineStart) {
  i32 offset = 0;
  for (i32 i = lineStart; i <= b.textLen; i++) {
    if (b.text[i] == '\n')
      break;
    if (GetTextWidth(b.text, lineStart, i) >= distanceFromLineStart) {
      if (abs(GetTextWidth(b.text, lineStart, i) - distanceFromLineStart) >
          abs(GetTextWidth(b.text, lineStart, i - 1) - distanceFromLineStart))
        offset--;

      break;
    }
    offset++;
  }
  return offset;
}

void MoveDown(Buffer& b) {
  i32 currentLineIndex = 0;
  FindLineStart(b, &currentLineIndex);
  i32 nextLineStart = b.lines[currentLineIndex + 1].textPos;

  if (currentLineIndex < b.linesLen - 2)
    b.cursor =
        ClampCursor(b, nextLineStart + FindLineOffsetByDistance(b, nextLineStart, b.desiredOffset));
}

void MoveUp(Buffer& b) {
  i32 currentLineIndex = 0;
  FindLineStart(b, &currentLineIndex);
  i32 prevLineStart = 0;
  if (currentLineIndex > 0)
    prevLineStart = b.lines[currentLineIndex - 1].textPos;

  b.cursor =
      ClampCursor(b, prevLineStart + FindLineOffsetByDistance(b, prevLineStart, b.desiredOffset));
}

i32 IsWhitespace(c16 ch) {
  return ch == L' ' || ch == L'\n';
}

// one two three four five   six sever
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

void JumpWordBackward(Buffer& b) {
  b.cursor = ClampCursor(b, b.cursor - 1);

  if (IsWhitespace(b.text[b.cursor])) {
    while (IsWhitespace(b.text[b.cursor]) && b.cursor > 0)
      b.cursor = ClampCursor(b, b.cursor - 1);
  }

  while (!IsWhitespace(b.text[b.cursor]) && b.cursor > 0)
    b.cursor = ClampCursor(b, b.cursor - 1);

  if (b.cursor != 0)
    b.cursor++;

  b.cursor = ClampCursor(b, b.cursor);
}
