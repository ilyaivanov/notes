#pragma once
#include "..//win32.cpp"

enum KeyFlags : u8 { CtrlKey = 1, AltKey = 2, WinKey = 4 };

struct Key {
  u8 flags;
  u32 code;
};

struct CommandBuffer {
  i32 len;
  Key keys[255];
};

typedef void CommandHandler();

struct Shortcut {
  Key keys[3];
  i32 len;
};

struct Command {
  Shortcut shortcut;
  CommandHandler* handler;
};

CommandBuffer command = {};
CommandBuffer lastCommand = {};

Command commands[255];
i32 commandsLen = 0;

Shortcut Key(const char* ch);
Shortcut Ctrl(const char* ch);
Shortcut Alt(const char* ch);

void ClearCommand() {
  memcpy(&lastCommand, &command, sizeof(CommandBuffer));
  command.len = 0;
}

inline Shortcut Key(const char* ch) {
  Shortcut res = {};
  i32 i = 0;
  while (*ch) {
    res.keys[i++].code = *ch;
    ch++;
  }
  res.len = i;
  return res;
}

inline Shortcut Ctrl(const char* ch) {
  Shortcut res = Key(ch);
  for (i32 i = 0; i < res.len; i++)
    res.keys[i].flags = CtrlKey;
  return res;
}

inline Shortcut Alt(const char* ch) {
  Shortcut res = Key(ch);
  for (i32 i = 0; i < res.len; i++)
    res.keys[i].flags = AltKey;
  return res;
}

void HandleCommand() {
  if (command.len > 0) {
    if (command.keys[command.len - 1].code == VK_ESCAPE)
      ClearCommand();

    bool commandNeedToBeDiscarded = true;
    for (i32 i = 0; i < commandsLen; i++) {
      Shortcut shortcut = commands[i].shortcut;
      bool ismatched = true;

      i32 min = Min(shortcut.len, command.len);
      i32 k = 0;
      bool hasMatchedAtLeastOne = false;
      for (; k < min; k++) {
        if (shortcut.keys[k].code != command.keys[k].code ||
            shortcut.keys[k].flags != command.keys[k].flags) {
          ismatched = false;

          break;
        } else {
          hasMatchedAtLeastOne = true;
        }
      }

      if (shortcut.len == command.len && ismatched) {
        commands[i].handler();
        ClearCommand();
        break;
      }

      if (command.len < shortcut.len && hasMatchedAtLeastOne) {
        commandNeedToBeDiscarded = false;
      }
    }

    if (commandNeedToBeDiscarded)
      command.len = 0;
  }
}

void AppendChar(u32 ch) {
  command.keys[command.len].code = ch;
  command.keys[command.len].flags = 0;

  if (IsKeyPressed(VK_CONTROL))
    command.keys[command.len].flags |= CtrlKey;
  if (IsKeyPressed(VK_MENU))
    command.keys[command.len].flags |= AltKey;
  if (IsKeyPressed(VK_LWIN))
    command.keys[command.len].flags |= WinKey;

  command.len++;

  HandleCommand();
}
