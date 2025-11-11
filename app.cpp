#pragma once
#include "win32.cpp"
#include "drawing.cpp"
#include "vim.cpp"
#include "sincos.cpp"
#include "anim.cpp"
#include "utils.cpp"
#include "modal.cpp"
#include <winuser.h>

wchar_t* path = (c16*)L"main.cpp";
const wchar_t* libPath = L"lib.dll";

enum Mode { Normal, Insert, VisualLine, Modal };

Mode mode = Normal;
i32 fontSize = 14;
f32 hardLineHeight = 1.1;
f32 softLineHeight = 0.9;

HFONT segoe;
HFONT consolas;
v2 pos = {300, 300};
Buffer buffer = {};
v3 bg = {0.2, 0.4, 0.2};

v3 white = {1, 1, 1};
v3 red = {1, 0, 0};
v3 grey = {0.4, 0.4, 0.4};
v3 lightGrey = {0.5, 0.5, 0.5};
v3 black = {0, 0, 0};
Spring scrollOffset;

i64 buildTime;
i64 formatTime;
Rect textArea;
v2 pagePadding = {60, 10};
f32 pageHeight;

f32 timeToCursorBlink = 1000;
f32 cursorBlinkStart = 0;

char* out;
u32 len = 0;

bool isBuildFailed;
char* buildOut;
u32 buildOutLen;

void AddHardLineBreak(i32 at) {
  buffer.lines[buffer.linesLen].textPos = at;
  buffer.lines[buffer.linesLen].isSoft = 0;
  buffer.linesLen += 1;
}

void AddSoftLineBreak(i32 at) {
  buffer.lines[buffer.linesLen].textPos = at;
  buffer.lines[buffer.linesLen].isSoft = 1;
  buffer.linesLen += 1;
}

void RebuildLines() {
  i32 wordStart = 0;
  i32 lineStart = 0;
  buffer.linesLen = 0;
  AddHardLineBreak(0);
  c16* text = buffer.text;
  // i32 isStartingFromLineStart = 1;

  f32 maxWidth = textArea.width - pagePadding.x * 2.0f;

  for (i32 i = 0; i < buffer.textLen; i++) {
    if (text[i] == '\n') {
      if (GetTextWidth(text, lineStart, i) > maxWidth) {
        AddSoftLineBreak(wordStart);
        lineStart = wordStart;
      }

      AddHardLineBreak(i + 1);
      lineStart = i + 1;

      wordStart = i + 1;
    } else if (text[i] == ' ') {
      if (GetTextWidth(text, lineStart, i) > maxWidth) {
        AddSoftLineBreak(wordStart);
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
      runningCursor.y += fontHeight * softLineHeight;
    else
      runningCursor.y += fontHeight * hardLineHeight;
  }
  return cursorPos;
}

void OnResize(AppState& app) {
  textArea.x = 0;
  textArea.y = 0;
  textArea.width = app.size.x;
  textArea.height = app.size.y;
  // textArea = ShrinkRect(textArea, pagePadding);
  RebuildLines();
}

HMODULE libModule;

typedef void RenderApp(MyBitmap* bitmap, AppState* app);
RenderApp* render;

void LoadFile(c16* path) {
  i64 size = GetMyFileSize(path);
  c8* file = (c8*)valloc(size);
  ReadFileInto(path, size, file);
  i32 wideCharsCount = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, file, size, 0, 0);
  c16* text = (c16*)valloc(wideCharsCount * sizeof(c16));
  MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, file, size, text, wideCharsCount);

  i32 bufferPos = 0;
  for (i32 i = 0; i < wideCharsCount; i++) {
    if (text[i] != '\r') {
      buffer.text[bufferPos] = text[i];
      bufferPos++;
    }
  }
  buffer.textLen = bufferPos;

  RebuildLines();
}

void Init(AppState& app) {
  InitAnimations();

  out = (char*)valloc(2000);
  buildOut = (char*)valloc(2000);

  segoe = CreateAppFont(L"Segoe UI", FW_NORMAL, 14, CLEARTYPE_QUALITY);
  consolas = CreateAppFont(L"Consolas", FW_NORMAL, 14, ANTIALIASED_QUALITY);
  // consolasAliased = CreateAppFont(L"Consolas", FW_NORMAL, 14, ANTIALIASED_QUALITY);

  UseFont(consolas);

  buffer.linesCapacity = 1024;
  buffer.lines = (LineBreak*)valloc(buffer.linesCapacity * sizeof(LineBreak));

  buffer.textCapacity = MB(2) / sizeof(c16);
  buffer.text = (c16*)valloc(buffer.textCapacity * sizeof(c16));

  LoadFile(path);

  OnResize(app);
}

void SaveFile() {
  i32 utf8Count = WideCharToMultiByte(CP_UTF8, 0, buffer.text, buffer.textLen, 0, 0, 0, 0);

  c8* text = (c8*)valloc(utf8Count * sizeof(c8));
  WideCharToMultiByte(CP_UTF8, 0, buffer.text, buffer.textLen, text, utf8Count, 0, 0);
  WriteMyFile(path, text, utf8Count);
}

void Teardown([[maybe_unused]] AppState& app) {
  SaveFile();
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

c16 currentCommand[255];
i32 currentCommandLen;

// first attempt to discard command if it doesn't partially match any of the existing commands
i32 isPartialMatch;

bool IsCommand(const c16* c) {
  if (currentCommandLen == 0)
    return false;

  i32 len = 0;
  while (c[len] != L'\0' && c[len] == currentCommand[len]) {
    len++;
  }

  bool res = len == currentCommandLen && c[len] == '\0';
  if (res) {
    currentCommandLen = 0;
    isPartialMatch = false;
  }

  if (len >= currentCommandLen)
    isPartialMatch = true;

  return res;
}

bool IsCtrlCommand(c16 ch) {
  bool res = currentCommandLen == 1 && currentCommand[0] == ch && GetKeyState(VK_CONTROL);
  if (res) {
    currentCommandLen = 0;
  }
  return res;
}

bool IsCommand(c16 ch) {
  bool res = currentCommandLen == 1 && currentCommand[0] == ch;
  if (res) {
    currentCommandLen = 0;
  }
  return res;
}

f32 ClampScrollOffset(f32 offset) {
  if (offset < 0)
    return 0;
  return offset;
}

void ScrollIntoCursor() {
  v2 p = GetCursorPos();

  f32 cursorY = p.y;

  scrollOffset.target =
      ClampScrollOffset(cursorY - textArea.height / 2.0f + GetFontHeight() / 2.0f);
}

void FormatCode() {
  SaveFile();
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

void HandleMotions(AppState& app) {
  if (IsCommand(L"j")) {
    MoveDown(buffer);
    OnCursorUpdated(app);
  }

  if (IsCommand(L"k")) {
    MoveUp(buffer);
    OnCursorUpdated(app);
  }

  if (IsCommand(L"w")) {
    buffer.cursor = JumpWordForward(buffer);
    OnCursorUpdated(app);
    UpdateDesiredOffset(buffer);
  }

  if (IsCommand(L"b")) {
    buffer.cursor = JumpWordBackward(buffer);
    UpdateDesiredOffset(buffer);
    OnCursorUpdated(app);
  }

  if (IsCommand(L"W")) {
    buffer.cursor = JumpWordForwardIgnorePunctuation(buffer);
    OnCursorUpdated(app);
    UpdateDesiredOffset(buffer);
  }

  if (IsCommand(L"B")) {
    buffer.cursor = JumpWordBackwardIgnorePunctuation(buffer);
    UpdateDesiredOffset(buffer);
    OnCursorUpdated(app);
  }

  if (IsCommand(L"l")) {
    MoveRight(buffer);
    UpdateDesiredOffset(buffer);
    OnCursorUpdated(app);
  }

  if (IsCommand(L"h")) {
    MoveLeft(buffer);
    UpdateDesiredOffset(buffer);
    OnCursorUpdated(app);
  }
}

void Rebuild() {
  if (libModule) {
    FreeLibrary(libModule);
    libModule = NULL;
    render = NULL;
  }
  // need to wait untill dll is released, otherwise compiler complains it can't remove dll file
  Sleep(40);

  buildOutLen = 0;
  char* cmd = (char*)"clang-cl /GR- /Zi /Od /FC /GS- /LDd /Felib.dll main.cpp /link /nodefaultlib "
                     "-EXPORT:RenderApp "
                     "user32.lib kernel32.lib gdi32.lib winmm.lib shell32.lib dwmapi.lib uuid.lib "
                     "ole32.lib";

  DWORD status = RunCommand(cmd, buildOut, &buildOutLen);

  isBuildFailed = status != 0;
  // RunCommand((char*)"cl main.cpp /link /nodefaultlib /subsystem:console kernel32.lib",
  // buildOut, &buildOutLen);

  if (!isBuildFailed) {
    libModule = LoadLibrary(libPath);

    if (libModule) {
      render = (RenderApp*)(void*)GetProcAddress(libModule, "RenderApp");
    }
  }
}

void OnKeyPress(u32 code, AppState& app) {
  if (mode == Modal) {

    if (code == VK_ESCAPE) {
      mode = Normal;
    } else if (code == VK_BACK)
      RemoveCharFromModal();
    else if (code == L'N' && IsKeyPressed(VK_CONTROL))
      ModalGoDown();
    else if (code == L'P' && IsKeyPressed(VK_CONTROL))
      ModalGoUp();
    else if (code == VK_RETURN) {
      SaveFile();
      if (filesLen > 0) {

        path = files[selectedFile].path;
        LoadFile(path);
        buffer.cursor = 0;
        mode = Normal;
      }
    } else if (IsAlphaNumeric(code))
      AddCharToModal(code);

  } else if (mode == VisualLine) {

    isPartialMatch = false;
    currentCommand[currentCommandLen++] = code;

    if (code == VK_ESCAPE) {
      mode = Normal;
    }

    HandleMotions(app);

    if (IsCommand(L"d")) {
      i32 selStart = Min(buffer.cursor, buffer.selectionStart);
      i32 selEnd = Max(buffer.cursor, buffer.selectionStart);
      i32 selStartLine = FindLineStartFrom(buffer, selStart);
      i32 selEndLine = FindLineEndFrom(buffer, selEnd);
      RemoveChars(buffer, selStartLine, selEndLine - 1);
      buffer.cursor = ClampCursor(buffer, selStartLine - 1);
      mode = Normal;
      RebuildLines();
    }

    if (IsCommand(L"c")) {
      i32 selStart = Min(buffer.cursor, buffer.selectionStart);
      i32 selEnd = Max(buffer.cursor, buffer.selectionStart);
      i32 selStartLine = FindLineStartFrom(buffer, selStart);
      i32 selEndLine = FindLineEndFrom(buffer, selEnd);
      RemoveChars(buffer, selStartLine, selEndLine - 2);
      buffer.cursor = selStartLine;
      mode = Insert;
      RebuildLines();
    }

    if (IsCommand(L"y")) {
      i32 selStart = Min(buffer.cursor, buffer.selectionStart);
      i32 selEnd = Max(buffer.cursor, buffer.selectionStart);
      i32 selStartLine = FindLineStartFrom(buffer, selStart);
      i32 selEndLine = FindLineEndFrom(buffer, selEnd);
      ClipboardCopy(app.window, buffer.text + selStartLine, selEndLine - selStartLine);
      mode = Normal;
    }

    if (!isPartialMatch) {
      currentCommandLen = 0;
    }

    if (code == VK_ESCAPE) {
      currentCommandLen = 0;
    }
  } else if (mode == Insert) {
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
    isPartialMatch = false;
    currentCommand[currentCommandLen++] = code;

    if (IsCommand(L"V")) {
      mode = VisualLine;
      buffer.selectionStart = buffer.cursor;
    }

    if (IsCommand(L" f")) {
      app.isFullscreen = !app.isFullscreen;
      SetFullscreen(app.window, app.isFullscreen);
    }

    if (IsCommand(L"gg")) {
      buffer.cursor = FindLineOffsetByDistance(buffer, 0, buffer.desiredOffset);
    }

    if (IsCommand(L"p")) {
      i32 len;
      c16* text = ClipboardPaste(app.window, &len);

      i32 at = buffer.cursor;
      i32 cursorPos = at + len;
      bool hasNewLine = HasNewLine(text);

      if (hasNewLine) {
        at = FindLineEndFrom(buffer, at);
        cursorPos = at;
      }

      InsertCharsAt(buffer, at, text, len);
      vfree(text);
      buffer.cursor = cursorPos;
      RebuildLines();
    }

    if (IsCommand(L"r")) {
      SaveFile();

      i64 start = GetPerfCounter();
      Rebuild();
      // RunCommand((char*)"./main.exe", out, &len);
      buildTime = (i64)round((GetPerfCounter() - start) / (f32)GetPerfFrequency() * 1000.0f);
    }

    if (IsCommand(L"f")) {
      SaveFile();
      formatTime = RunClangFormat((c16*)path, buffer);
      RebuildLines();
    }

    if (IsCommand(L"zz")) {
      ScrollIntoCursor();
    }

    if (IsCommand(L"G")) {
      i32 line = buffer.lines[buffer.linesLen - 2].textPos;
      buffer.cursor = line + FindLineOffsetByDistance(buffer, line, buffer.desiredOffset);
    }

    if (IsCommand(L"I")) {
      buffer.cursor = FindLineStart(buffer);
      mode = Insert;
      OnCursorUpdated(app);
    }
    if (IsCommand(L"A")) {
      buffer.cursor = FindLineEnd(buffer) - 1;
      mode = Insert;
      OnCursorUpdated(app);
    }
    if (IsCommand(L"i")) {
      mode = Insert;
      OnCursorUpdated(app);
    }

    if (IsCommand(L"dd") || IsCommand(L"dl")) {
      i32 lineStart = FindLineStart(buffer);
      i32 lineEnd = FindLineEnd(buffer);
      RemoveChars(buffer, lineStart, lineEnd - 1);
      buffer.cursor = ClampCursor(buffer, lineStart);
      OnCursorUpdated(app);
      RebuildLines();
    }

    if (IsCommand(L"ci\"") || IsCommand(L"cis")) {
      Range stringRange = GetStringLocation(buffer, buffer.cursor);
      if (stringRange.from != -1) {
        RemoveChars(buffer, stringRange.from + 1, stringRange.to - 1);
        buffer.cursor = ClampCursor(buffer, stringRange.from + 1);
        OnCursorUpdated(app);
        RebuildLines();
        mode = Insert;
        UpdateDesiredOffset(buffer);
      }
    }

    if (IsCommand(L"ca\"") || IsCommand(L"cas")) {
      Range stringRange = GetStringLocation(buffer, buffer.cursor);
      if (stringRange.from != -1) {
        RemoveChars(buffer, stringRange.from, stringRange.to);
        buffer.cursor = ClampCursor(buffer, stringRange.from);
        OnCursorUpdated(app);
        RebuildLines();
        mode = Insert;
        UpdateDesiredOffset(buffer);
      }
    }

    if (IsCommand(L"}")) {
      i32 next = FindNext(buffer, buffer.cursor + 1, (c16*)L"\n\n");
      if (next != -1) {
        buffer.cursor = next + 1;
      }
    }

    if (IsCommand(L"{")) {
      i32 prev = FindPrev(buffer, buffer.cursor - 1, (c16*)L"\n\n");
      if (prev != -1) {
        buffer.cursor = prev - 1;
      }
    }

    if (IsCommand(L"cc") || IsCommand(L"cl")) {
      i32 lineStart = FindLineStart(buffer);
      i32 lineEnd = FindLineEnd(buffer);
      RemoveChars(buffer, lineStart, lineEnd - 2);
      buffer.cursor = ClampCursor(buffer, lineStart);
      OnCursorUpdated(app);
      RebuildLines();
      UpdateDesiredOffset(buffer);

      mode = Insert;
    }

    if (IsCommand(L"C")) {
      i32 lineEnd = FindLineEnd(buffer);
      RemoveChars(buffer, buffer.cursor, lineEnd - 2);
      OnCursorUpdated(app);
      RebuildLines();

      mode = Insert;
    }
    if (IsCommand(L"D")) {
      i32 lineEnd = FindLineEnd(buffer);
      RemoveChars(buffer, buffer.cursor, lineEnd - 2);
      OnCursorUpdated(app);
      RebuildLines();
    }

    if (IsCommand(L"q")) {
      app.isRunning = false;
    }

    if (IsCommand(L"l")) {
      MoveRight(buffer);
      UpdateDesiredOffset(buffer);
      OnCursorUpdated(app);
    }

    if (IsCommand(L"h")) {
      MoveLeft(buffer);
      UpdateDesiredOffset(buffer);
      OnCursorUpdated(app);
    }

    if (IsCommand(L'J')) {
      scrollOffset.target += GetFontHeight() * 3;
    }

    if (IsCommand(L'K')) {
      scrollOffset.target -= GetFontHeight() * 3;
    }

    if (IsCommand(L"j")) {
      MoveDown(buffer);
      OnCursorUpdated(app);
    }

    if (IsCommand(L"k")) {
      MoveUp(buffer);
      OnCursorUpdated(app);
    }

    if (IsCommand(L"w")) {
      buffer.cursor = JumpWordForward(buffer);
      OnCursorUpdated(app);
      UpdateDesiredOffset(buffer);
    }

    if (IsCommand(L"b")) {
      buffer.cursor = JumpWordBackward(buffer);
      UpdateDesiredOffset(buffer);
      OnCursorUpdated(app);
    }
    if (IsCommand(L"W")) {
      buffer.cursor = JumpWordForwardIgnorePunctuation(buffer);
      OnCursorUpdated(app);
      UpdateDesiredOffset(buffer);
    }

    if (IsCommand(L"B")) {
      buffer.cursor = JumpWordBackwardIgnorePunctuation(buffer);
      UpdateDesiredOffset(buffer);
      OnCursorUpdated(app);
    }

    if (IsCommand(L"x")) {
      if (buffer.cursor < buffer.textLen - 1) {
        RemoveCharAt(buffer, buffer.cursor);
        RebuildLines();
      }
    }
    if (IsCommand(VK_BACK)) {
      RemoveCharFromLeft(app);
    }
    if (IsCommand(L"\r"))
      AddCharAtCursor(app, '\n');

    if (IsCommand(L"O")) {
      i32 target = 0;
      target = FindLineStart(buffer);

      InsertCharAt(buffer, target, '\n');
      buffer.cursor = target;

      mode = Insert;

      OnCursorUpdated(app);
      RebuildLines();
      UpdateDesiredOffset(buffer);
    }
    if (IsCommand(L"o")) {
      i32 target = 0;
      target = FindLineEnd(buffer);

      InsertCharAt(buffer, target, '\n');
      buffer.cursor = target;

      mode = Insert;

      OnCursorUpdated(app);
      RebuildLines();
      UpdateDesiredOffset(buffer);
    }

    if (IsCommand(L" sf")) {
      mode = Modal;
      InitModal();
    }

    if (!isPartialMatch) {
      currentCommandLen = 0;
    }

    if (code == VK_ESCAPE) {
      currentCommandLen = 0;
    }
  }
}

void Draw(AppState& app) {
  UseFont(consolas);

  timeToCursorBlink -= app.lastFrameTimeMs;
  f32 fontHeight = GetFontHeight();

  v2 running = {textArea.x + pagePadding.x, textArea.y - scrollOffset.current + pagePadding.y};
  CharBuffer lineBuff = {};
  i32 line = 1;

  i32 selStart = Min(buffer.cursor, buffer.selectionStart);
  i32 selEnd = Max(buffer.cursor, buffer.selectionStart);
  i32 selStartLine = FindLineStartFrom(buffer, selStart);
  i32 selEndLine = FindLineEndFrom(buffer, selEnd);
  for (i32 i = 0; i < buffer.linesLen - 1; i++) {
    i32 start = buffer.lines[i].textPos;
    i32 end = (i == buffer.linesLen - 1) ? buffer.textLen : buffer.lines[i + 1].textPos;
    if (!buffer.lines[i].isSoft) {
      lineBuff.len = 0;
      Append(&lineBuff, line);

      SetColors(grey, black);
      SetAlign(TA_RIGHT);

      PrintText(running.x - 10, round(running.y), lineBuff.content, lineBuff.len);
      line++;
    }

    i32 lineStart = start;
    while (buffer.text[lineStart] == ' ')
      lineStart++;
    if (buffer.text[lineStart] == L'/' && buffer.text[lineStart + 1] == L'/')
      SetColors(lightGrey, black);
    else
      SetColors(white, black);

    SetAlign(TA_LEFT);

    if (mode == VisualLine) {

      i32 isLineInRange =
          (selStartLine <= start && start < selEndLine) || (selStartLine < end && end < selEndLine);

      if (isLineInRange)
        SetColors(white, grey);
    }
    PrintText(running.x, round(running.y), buffer.text + start, end - start);

    if (buffer.lines[i + 1].isSoft)
      running.y += fontHeight * softLineHeight;
    else
      running.y += fontHeight * hardLineHeight;
  }

  pageHeight = running.y + pagePadding.y + scrollOffset.current;

  if (pageHeight > textArea.height) {
    f32 scrollbarWidth = 10;
    f32 scrollbarHeight = textArea.height * textArea.height / pageHeight;
    f32 maxOffset = pageHeight - textArea.height;
    f32 maxScrollY = textArea.height - scrollbarHeight;
    f32 scrollY = lerp(0, maxScrollY, scrollOffset.current / maxOffset);

    Rect scrollbar = {app.size.x - scrollbarWidth, scrollY, scrollbarWidth, scrollbarHeight};
    PaintRect(scrollbar, vec3(0.3, 0.3, 0.3));
  }

  f32 cursorHeight = fontHeight * 1.1;
  v2 cursorPos = GetCursorPos();

  f32 sin;
  f32 cos;
  SinCos((app.appTimeMs - cursorBlinkStart) / 300.0f, &sin, &cos);

  f32 cursorAlpha = 1;
  if (timeToCursorBlink <= 0)
    cursorAlpha = abs(cos);

  Rect cursorRect = {cursorPos.x + pagePadding.x,
                     cursorPos.y + pagePadding.y - scrollOffset.current, 2.0f, cursorHeight};
  cursorRect.x -= cursorRect.width / 2.0f;
  v4 cursorColor = {1, 1, 1, cursorAlpha};
  if (mode == Insert)
    cursorColor = {1, 0.2, 0.2, cursorAlpha};

  PaintRect(cursorRect, cursorColor);

  CharBuffer buff = {};
  Append(&buff, L"FPS: ");
  Append(&buff, i32(1000.0f / app.lastFrameTimeMs));
  Append(&buff, L" Desired: ");
  Append(&buff, buffer.desiredOffset);
  Append(&buff, L" Len: ");
  Append(&buff, buffer.textLen);
  Append(&buff, L" Capacity: ");
  Append(&buff, buffer.textCapacity);
  Append(&buff, L" Build: ");
  Append(&buff, buildTime);
  Append(&buff, L"ms");
  Append(&buff, L" Fmt: ");
  Append(&buff, formatTime);
  Append(&buff, L"ms");

  SetColors(white, black);
  SetAlign(TA_RIGHT);
  PrintText(app.size.x - 10, app.size.y - fontHeight - 8, buff.content, buff.len);

  if (len > 0) {
    SetAlign(TA_RIGHT);
    i32 lineStart = 0;
    f32 y = 20;

    for (u32 i = 0; i < len; i++) {
      if (out[i] == '\n' || i == len - 1) {
        PrintText(app.size.x - 20, y, out + lineStart, i - lineStart + 1);
        lineStart = i + 1;
        y += fontHeight * hardLineHeight;
      }
    }
  }

  SetAlign(TA_RIGHT);
  PrintText(app.size.x - 10, app.size.y - fontHeight * 2 - 8, currentCommand, currentCommandLen);

  if (mode == Modal) {
    RenderModal(app);
  }
  if (isBuildFailed) {
    PrintParagraph(app.dc, app.size.x - 10, 10, buildOut, buildOutLen);
  } else if (render) {
    SetColors(white, black);
    render(currentBitmap, &app);
  }

  UpdateSpring(&scrollOffset, app.lastFrameTimeMs / 1000.0f);
}
