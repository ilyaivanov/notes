#pragma once
#include "..//win32.cpp"
#include "item.cpp"

enum KeyFlags : u8 { Ctrl = 1, Alt = 2, Win = 4 };

struct Key {
  u8 flags;
  u32 code;
};

struct CommandBuffer {
  i32 len;
  Key keys[255];
};

CommandBuffer command = {};
CommandBuffer lastCommand = {};

void ClearCommand() {
  memcpy(&lastCommand, &command, sizeof(CommandBuffer));
  command.len = 0;
}

void HandleCommand() {
  if (command.len > 0) {
    if (command.keys[command.len - 1].code == VK_ESCAPE)
      ClearCommand();
  }
}

// mode, selectedItem, cursor
void AppendChar(u32 ch) {
  command.keys[command.len].code = ch;
  if (IsKeyPressed(VK_CONTROL))
    command.keys[command.len].flags |= Ctrl;
  if (IsKeyPressed(VK_MENU))
    command.keys[command.len].flags |= Alt;
  if (IsKeyPressed(VK_LWIN))
    command.keys[command.len].flags |= Win;

  command.len++;

  HandleCommand();
}

//
//
//
//
//
//

bool IsLetter(char ch) {
  return (ch >= L'a' && ch <= L'z') || (ch >= L'A' && ch <= L'Z');
}

char ToLower(char ch) {
  if (ch >= L'A' && ch <= L'Z')
    return ch + ('a' - 'A');
  return ch;
}

u32 IsAlphaNumeric(c16 ch) {
  return (ch >= L'0' && ch <= L'9') || (ch >= L'a' && ch <= L'z') || (ch >= L'A' && ch <= L'Z');
}

i32 IsWhitespace(c16 ch) {
  return ch == L' ' || ch == L'\n';
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

// // one two three four five   six sever
i32 JumpWordForwardIgnorePunctuation(Item* item, i32 from) {
  c8* text = item->text;
  i32 textLen = item->textLen;
  i32 cursor = from;
  if (IsWhitespace(text[cursor])) {
    while (IsWhitespace(text[cursor]) && cursor <= textLen)
      cursor++;
  } else {
    while (!IsWhitespace(text[cursor]) && cursor <= textLen)
      cursor++;
  }
  if (text[cursor] == '\n')
    cursor++;

  while (text[cursor] == ' ' && cursor <= textLen)
    cursor++;
  return cursor;
}

i32 JumpWordBackwardIgnorePunctuation(Item* item, i32 from) {
  c8* text = item->text;
  i32 textLen = item->textLen;
  i32 cursor = Max(from - 1, 0);

  if (IsWhitespace(text[cursor])) {
    while (IsWhitespace(text[cursor]) && cursor > 0)
      cursor--;
  }

  while (!IsWhitespace(text[cursor]) && cursor > 0)
    cursor--;

  if (cursor != 0)
    cursor++;

  return cursor;
}

i32 JumpWordBackward(Item* item, i32 from) {
  c8* text = item->text;
  i32 pos = Max(from - 1, 0);
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
  if (pos != 0)
    pos++;

  if (!isStartedAtWhitespace) {
    while (pos > 0 && IsWhitespace(text[pos]))
      pos--;
  }

  return pos;
}

i32 JumpWordForward(Item* item, i32 from) {
  c8* text = item->text;
  i32 pos = from;
  i32 size = item->textLen;
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
