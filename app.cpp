#pragma once
#include "win32.cpp"
#include "drawing.cpp"
#include "vim.cpp"
#include "sincos.cpp"

const wchar_t* path = L"sample.txt";

enum Mode { Normal, Insert };
Mode mode = Normal;
i32 fontSize = 14;
f32 lineHeight = 1.1;

HFONT segoe;
v2 pos = {300, 300};
Buffer buffer = {};
v3 bg = {0.2, 0.4, 0.2};

v3 white = {1, 1, 1};
v3 red = {1, 0, 0};
v3 grey = {0.4, 0.4, 0.4};
v3 black = {0, 0, 0};

Rect textArea;
v2 pagePadding = {40, 10};

f32 timeToCursorBlink = 1000;
f32 cursorBlinkStart = 0;

void AddLine(i32 isSoft, i32 pos) {
  buffer.lines[buffer.linesLen].isSoft = isSoft;
  buffer.lines[buffer.linesLen].textPos = pos;
  buffer.linesLen += 1;
}

void RebuildLines() {
  i32 wordStart = 0;
  i32 lineStart = 0;
  buffer.linesLen = 0;
  buffer.lines[0] = (LineBreak){.isSoft = 0, .textPos = 0};
  buffer.linesLen = 1;
  c16* text = buffer.text;
  // i32 isStartingFromLineStart = 1;

  f32 maxWidth = textArea.width;

  for (i32 i = 0; i < buffer.textLen; i++) {
    if (text[i] == '\n') {
      if (GetTextWidth(text, lineStart, i) > maxWidth) {
        buffer.lines[buffer.linesLen++] = (LineBreak){.isSoft = 1, .textPos = wordStart};
        lineStart = wordStart;
      }

      buffer.lines[buffer.linesLen++] = (LineBreak){.isSoft = 0, .textPos = i + 1};
      lineStart = i + 1;

      wordStart = i + 1;
    } else if (text[i] == ' ') {
      if (GetTextWidth(text, lineStart, i) > maxWidth) {
        buffer.lines[buffer.linesLen++] = (LineBreak){.isSoft = 1, .textPos = wordStart};
        lineStart = wordStart;
      }
      wordStart = i + 1;
    }
  }

  // buffer.lines[buffer.linesLen++].textPos = buffer.textLen;
}

v2 GetCursorPos() {
  f32 fontHeight = GetFontHeight();
  v2 runningCursor = {textArea.x, textArea.y};
  v2 cursorPos = {};

  for (i32 i = 0; i < buffer.linesLen - 1; i++) {
    i32 start = buffer.lines[i].textPos;
    i32 end = (i == buffer.linesLen - 1) ? buffer.textLen : buffer.lines[i + 1].textPos;

    i32 isCursorVisibleOnLine = ((buffer.cursor >= start && buffer.cursor < end) ||
                                 (buffer.cursor == buffer.textLen && i == buffer.linesLen - 1));

    if (isCursorVisibleOnLine) {
      cursorPos.x = runningCursor.x + GetTextWidth(buffer.text, start, buffer.cursor);
      cursorPos.y = runningCursor.y;
      break;
    }

    if (buffer.lines[i + 1].isSoft)
      runningCursor.y += fontHeight;
    else
      runningCursor.y += fontHeight * lineHeight;
  }
  return cursorPos;
}

void OnResize(AppState& app) {
  textArea.x = 0;
  textArea.y = 0;
  textArea.width = app.size.x;
  textArea.height = app.size.y;
  textArea = ShrinkRect(textArea, pagePadding);
  RebuildLines();
}

void Init(AppState& app) {
  segoe = CreateAppFont(L"Segoe UI", FW_NORMAL, 14);
  UseFont(segoe);

  i64 size = GetMyFileSize(path);
  c8* file = (c8*)valloc(size);
  ReadFileInto(path, size, file);
  i32 wideCharsCount = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, file, size, 0, 0);
  c16* text = (c16*)valloc(wideCharsCount * sizeof(c16));
  MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, file, size, text, wideCharsCount);

  buffer.linesCapacity = 1024;
  buffer.lines = (LineBreak*)valloc(buffer.linesCapacity * sizeof(LineBreak));

  buffer.textCapacity = MB(2) / sizeof(c16);
  buffer.text = (c16*)valloc(buffer.textCapacity * sizeof(c16));
  i32 bufferPos = 0;
  for (i32 i = 0; i < wideCharsCount; i++) {
    if (text[i] != '\r') {
      buffer.text[bufferPos] = text[i];
      bufferPos++;
    }
  }
  buffer.textLen = bufferPos;

  OnResize(app);
}

void OnCursorUpdated(AppState& app) {
  timeToCursorBlink = 600;
  cursorBlinkStart = app.appTimeMs + timeToCursorBlink;
}

void RemoveCharFromLeft(AppState& app) {
  if (buffer.cursor > 0) {
    RemoveCharAt(buffer, buffer.cursor - 1);
    buffer.cursor -= 1;
    RebuildLines();
    OnCursorUpdated(app);
    UpdateDesiredOffset(buffer);
  }
}

void AddCharAtCursor(AppState& app, u32 ch) {
  InsertCharAt(buffer, buffer.cursor, ch);
  buffer.cursor++;

  OnCursorUpdated(app);
  RebuildLines();
  UpdateDesiredOffset(buffer);
}

void OnKeyPress(u32 code, AppState& app) {
  if (mode == Insert) {
    if (code == VK_ESCAPE) {
      mode = Normal;
      OnCursorUpdated(app);
    } else if (code == '\r')
      AddCharAtCursor(app, '\n');
    else if (code == VK_BACK)
      RemoveCharFromLeft(app);
    else
      AddCharAtCursor(app, code);
  } else {
    if (code == 'i') {
      mode = Insert;
      OnCursorUpdated(app);
    }
    if (code == 'q') {
      app.isRunning = false;
    }
    if (code == 'l') {
      MoveRight(buffer);
      UpdateDesiredOffset(buffer);
      OnCursorUpdated(app);
    }

    if (code == 'h') {
      MoveLeft(buffer);
      UpdateDesiredOffset(buffer);
      OnCursorUpdated(app);
    }

    if (code == 'j') {
      MoveDown(buffer);
      OnCursorUpdated(app);
    }

    if (code == 'k') {
      MoveUp(buffer);
      OnCursorUpdated(app);
    }

    if (code == 'w') {
      JumpWordForward(buffer);
      OnCursorUpdated(app);
      UpdateDesiredOffset(buffer);
    }

    if (code == 'b') {
      JumpWordBackward(buffer);
      UpdateDesiredOffset(buffer);
      OnCursorUpdated(app);
    }

    if (code == 'x') {
      if (buffer.cursor < buffer.textLen - 1) {
        RemoveCharAt(buffer, buffer.cursor);
        RebuildLines();
      }
    }
    if (code == VK_BACK) {
      RemoveCharFromLeft(app);
    }
    if (code == '\r')
      AddCharAtCursor(app, '\n');

    if (code == 'o' || code == 'O') {
      i32 target = 0;
      if (code == 'O')
        target = FindLineStart(buffer);
      else
        target = FindLineEnd(buffer);

      InsertCharAt(buffer, target, '\n');
      buffer.cursor = target;

      mode = Insert;

      OnCursorUpdated(app);
      RebuildLines();
      UpdateDesiredOffset(buffer);
    }
  }
}

void Draw(AppState& app) {
  timeToCursorBlink -= app.lastFrameTimeMs;
  f32 fontHeight = GetFontHeight();

  v2 running = {textArea.x, textArea.y};
  CharBuffer lineBuff = {};
  i32 line = 1;
  for (i32 i = 0; i < buffer.linesLen - 1; i++) {
    i32 start = buffer.lines[i].textPos;
    i32 end = (i == buffer.linesLen - 1) ? buffer.textLen : buffer.lines[i + 1].textPos;
    if (!buffer.lines[i].isSoft) {
      lineBuff.len = 0;
      Append(&lineBuff, line);

      SetColors(grey, black);
      SetAlign(TA_RIGHT);
      PrintText(running.x - 10, running.y, lineBuff.content, lineBuff.len);
      line++;
    }

    SetColors(white, black);
    SetAlign(TA_LEFT);
    PrintText(running.x, running.y, buffer.text + start, end - start);

    if (buffer.lines[i + 1].isSoft)
      running.y += fontHeight;
    else
      running.y += fontHeight * lineHeight;
  }

  f32 cursorHeight = fontHeight * 1.1;
  v2 cursorPos = GetCursorPos();

  f32 sin;
  f32 cos;
  SinCos((app.appTimeMs - cursorBlinkStart) / 300.0f, &sin, &cos);

  f32 cursorAlpha = 1;
  if (timeToCursorBlink <= 0)
    cursorAlpha = abs(cos);

  Rect cursorRect = {cursorPos.x, cursorPos.y, 2.0f, cursorHeight};
  cursorRect.x -= cursorRect.width / 2.0f;
  v4 cursorColor = {1, 1, 1, cursorAlpha};
  if (mode == Insert)
    cursorColor = {1, 0.2, 0.2, cursorAlpha};

  PaintRect(cursorRect, cursorColor);

  CharBuffer buff = {};
  Append(&buff, L"Offset: ");
  Append(&buff, GetTextWidth(buffer.text, FindLineStart(buffer), buffer.cursor));
  Append(&buff, L" Desired: ");
  Append(&buff, buffer.desiredOffset);
  PrintText(10, app.size.y - fontHeight - 8, buff.content, buff.len);
}
