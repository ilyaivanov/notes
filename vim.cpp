#pragma once
#include "win32.cpp"
#include "anim.cpp"
// #include "drawing.cpp"

struct LineBreak {
  // Soft line break is a text overflow break due to a limited screen width.
  // Non-soft (hard) line break is via \n
  i8 isSoft;
  i32 textPos;
};

struct Buffer {
  c16 path[MAX_PATH];

  c16* text;
  i32 textLen;
  i32 textCapacity;

  LineBreak* lines;
  i32 linesLen;
  i32 linesCapacity;

  i32 selectionStart;
  i32 cursor;
  i32 desiredOffset;

  Spring offset;
  bool isModified;
};

struct Range {
  i32 from;
  i32 to;
};

i32 GetTextWidth(HDC dc, wchar_t* text, i32 from, i32 to) {
  SIZE s2;
  GetTextExtentPoint32W(dc, text + from, to - from, &s2);
  return s2.cx;
}

void InsertCharAt(Buffer& b, i32 at, c16 ch) {
  for (i32 i = b.textLen; i > at; i--) {
    b.text[i] = b.text[i - 1];
  }
  b.text[at] = ch;
  b.textLen++;
}

void InsertCharsAt(Buffer& b, i32 at, c16* text, i32 textLen) {
  for (i32 i = b.textLen; i > at; i--) {
    b.text[i + textLen - 1] = b.text[i - 1];
  }
  for (i32 i = 0; i < textLen; i++) {
    b.text[at + i] = text[i];
  }
  b.textLen += textLen;
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

i32 FindLineStartFrom(Buffer& b, i32 at, i32* lineIndex = 0) {
  for (i32 i = 0; i < b.linesLen - 1; i++) {
    i32 start = b.lines[i].textPos;
    i32 end = b.lines[i + 1].textPos;
    if (at >= start && at < end) {
      if (lineIndex)
        *lineIndex = i;
      return start;
    }
  }
  return 0;
}

i32 FindLineStart(Buffer& b, i32* lineIndex = 0) {
  return FindLineStartFrom(b, b.cursor, lineIndex);
}

i32 FindLineEndFrom(Buffer& b, i32 at, i32* lineIndex = 0) {
  for (i32 i = 0; i < b.linesLen - 1; i++) {
    i32 start = b.lines[i].textPos;
    i32 end = b.lines[i + 1].textPos;
    if (at >= start && at < end) {
      if (lineIndex)
        *lineIndex = i + 1;
      return end;
    }
  }
  return b.textLen;
}

i32 FindLineEnd(Buffer& b, i32* lineIndex = 0) {
  return FindLineEndFrom(b, b.cursor, lineIndex);
}

i32 FindLineStartv2(Buffer& b, i32 from) {
  for (i32 i = from - 1; i >= 0; i--) {
    if (b.text[i] == '\n')
      return i + 1;
  }
  return 0;
}

i32 FindLineEndv2(Buffer& b, i32 from) {
  for (i32 i = from; i < b.textLen; i++) {
    if (b.text[i] == '\n')
      return i;
  }
  return 0;
}

void UpdateDesiredOffset(Buffer& b, HDC dc) {
  b.desiredOffset = GetTextWidth(dc, b.text, FindLineStartv2(b, b.cursor), b.cursor);
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

i32 MoveDown(Buffer& b, i32 from, HDC dc) {
  i32 nextLineStart = from + 1;
  if (b.text[from] != '\n') {
    while (b.text[nextLineStart] != '\n' && nextLineStart < b.textLen)
      nextLineStart++;

    if (b.text[nextLineStart] == '\n')
      nextLineStart++;
  }

  i32 nextPos = ClampCursor(b, nextLineStart +
                                   FindLineOffsetByDistance(b, dc, nextLineStart, b.desiredOffset));
  if (nextPos != b.textLen - 1 || b.text[nextPos] == '\n')
    return nextPos;

  return -1;
}

i32 MoveUp(Buffer& b, i32 from, HDC dc) {
  i32 prevLineStart = FindLineStartv2(b, FindLineStartv2(b, from) - 1);

  return ClampCursor(b, prevLineStart +
                            FindLineOffsetByDistance(b, dc, prevLineStart, b.desiredOffset));
}

u32 IsAlphaNumeric(c16 ch) {
  return (ch >= L'0' && ch <= L'9') || (ch >= L'a' && ch <= L'z') || (ch >= L'A' && ch <= L'Z');
}

i32 IsWhitespace(c16 ch) {
  return ch == L' ' || ch == L'\n';
}

bool HasNewLine(c16* text) {
  int i = 0;
  while (text[i]) {
    if (text[i] == L'\n')
      return true;
    i++;
  }
  return false;
}

u32 IsPunctuation(c16 ch) {
  // use a lookup table
  const c16* punctuation = L"!\"#$%&'()*+,-./:;<=>?@[\\]^`{|}~";

  const c16* p = punctuation;
  while (*p) {
    if (ch == *p) {
      return 1;
    }
    p++;
  }
  return 0;
}

// one two three four five   six sever
i32 JumpWordForwardIgnorePunctuation(Buffer& b) {
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
  return ClampCursor(b, b.cursor);
}

i32 JumpWordBackwardIgnorePunctuation(Buffer& b) {
  b.cursor = ClampCursor(b, b.cursor - 1);

  if (IsWhitespace(b.text[b.cursor])) {
    while (IsWhitespace(b.text[b.cursor]) && b.cursor > 0)
      b.cursor = ClampCursor(b, b.cursor - 1);
  }

  while (!IsWhitespace(b.text[b.cursor]) && b.cursor > 0)
    b.cursor = ClampCursor(b, b.cursor - 1);

  if (b.cursor != 0)
    b.cursor++;

  return ClampCursor(b, b.cursor);
}

i32 JumpWordBackward(Buffer& buffer) {
  c16* text = buffer.text;
  i32 pos = Max(buffer.cursor - 1, 0);
  i32 isStartedAtWhitespace = IsWhitespace(text[pos]);

  while (pos > 0 && IsWhitespace(text[pos]))
    pos--;

  if (IsAlphaNumeric(text[pos])) {
    while (pos > 0 && IsAlphaNumeric(text[pos]))
      pos--;
  } else {
    while (pos > 0 && IsPunctuation(text[pos]))
      pos--;
  }
  pos++;

  if (!isStartedAtWhitespace) {
    while (pos > 0 && IsWhitespace(text[pos]))
      pos--;
  }

  return pos;
}

i32 JumpWordForward(Buffer& buffer) {
  c16* text = buffer.text;
  i32 pos = buffer.cursor;
  i32 size = buffer.textLen;
  if (IsWhitespace(text[pos])) {
    while (pos < size && IsWhitespace(text[pos]))
      pos++;
  } else {
    if (!IsPunctuation(text[pos]) && !IsWhitespace(text[pos])) {
      while (pos < size && !IsPunctuation(text[pos]) && !IsWhitespace(text[pos]))
        pos++;
    } else {
      while (pos < size && IsPunctuation(text[pos]))
        pos++;
    }
    while (pos < size && IsWhitespace(text[pos]))
      pos++;
  }

  return pos;
}

Range GetStringLocation(Buffer& buffer, i32 at) {
  Range res = {-1, -1};
  i32 lineStart = FindLineStart(buffer);
  i32 quoteLeft = -1;
  i32 quoteRight = -1;

  for (i32 i = at; i >= lineStart; i--) {
    if (buffer.text[i] == L'"') {
      quoteLeft = i;
      break;
    }
  }

  for (i32 i = at; i < buffer.textLen; i++) {
    if (buffer.text[i] == L'"') {
      quoteRight = i;
      break;
    }
  }

  if (quoteRight != -1) {
    if (quoteLeft == -1) {
      quoteLeft = quoteRight;

      for (i32 i = quoteLeft + 1; i < buffer.textLen; i++) {
        if (buffer.text[i] == L'"') {
          quoteRight = i;
          break;
        }
      }
    }

    if (quoteRight != -1) {
      res.from = quoteLeft;
      res.to = quoteRight;
      return res;
    }
  }

  return res;
}

i32 FindNext(Buffer& b, i32 from, c16* text) {
  i32 inText = 0;
  for (i32 i = from; i < b.textLen; i++) {
    if (b.text[i] == text[inText]) {
      inText++;
      if (text[inText] == L'\0') {
        return i - inText + 1;
      }
    } else {
      inText = 0;
    }
  }
  return -1;
}

i32 FindPrev(Buffer& b, i32 from, c16* text) {
  i32 strLen = wstrlen(text) - 1;
  i32 inText = strLen;

  for (i32 i = from; i >= 0; i--) {
    if (b.text[i] == text[inText]) {
      inText--;
      if (inText < 0) {
        return i + strLen + 1;
      }
    } else {
      inText = strLen;
    }
  }

  return -1;
}
