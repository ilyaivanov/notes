#pragma once
#include <windows.h>
#include "item.cpp"
#include "vimutils.cpp"
#include "commandBuffer.cpp"

#define EMPTY_ITEM_TEXT_CAPACITY 8

enum Mode { Normal, Insert, ReplaceChar, VisualLine, Visual, Modal };

struct Cursor {
  i32 pos;
  i32 desiredOffset;
};

HWND window;
Mode mode = Normal;
i32 fontSize;
Cursor cursor;
Item* root;
Item* selectedItem;
//

// these are the functions from the main which I don;t want to bring here
void UpdateCursorPosWithDesiredOffset(i32 pos);
void UpdateFontSize();
void EnterInsertMode();
void FindPositionBasedOnDesiredOffset();

void UpdateSelection(Item* item) {
  if (item) {
    selectedItem = item;
    FindPositionBasedOnDesiredOffset();
  }
}

void MoveToStart() {
  UpdateCursorPosWithDesiredOffset(0);
}

void MoveToEnd() {
  UpdateCursorPosWithDesiredOffset(selectedItem->textLen);
}

void JumpWordForwardA() {
  UpdateCursorPosWithDesiredOffset(JumpWordForward(selectedItem, cursor.pos));
}

void JumpWordForwardIgnorePunctuationA() {
  UpdateCursorPosWithDesiredOffset(JumpWordForwardIgnorePunctuation(selectedItem, cursor.pos));
}

void JumpWordBackwardA() {
  UpdateCursorPosWithDesiredOffset(JumpWordBackward(selectedItem, cursor.pos));
}

void JumpWordBackwardIgnorePunctuationA() {
  UpdateCursorPosWithDesiredOffset(JumpWordBackwardIgnorePunctuation(selectedItem, cursor.pos));
}

void MoveToStartAndEnterInsertMode() {
  MoveToStart();
  EnterInsertMode();
}

void MoveToEndAndEnterInsertMode() {
  MoveToEnd();
  EnterInsertMode();
}

void IncFontSize() {
  fontSize++;
  UpdateFontSize();
}

void DecFontSize() {
  fontSize--;
  UpdateFontSize();
}

void CreateItemAfter() {
  Item* newItem = CreateEmptyItem(EMPTY_ITEM_TEXT_CAPACITY);
  Item* newParent = selectedItem->parent;

  i32 index = IndexOf(selectedItem);
  i32 pos = index + 1;

  InsertChildAt(newParent, newItem, pos);
  selectedItem = newItem;
  UpdateCursorPosWithDesiredOffset(0);
  EnterInsertMode();
}

void CreateItemBefore() {
  Item* newItem = CreateEmptyItem(EMPTY_ITEM_TEXT_CAPACITY);
  Item* newParent = selectedItem->parent;

  i32 index = IndexOf(selectedItem);
  i32 pos = index;

  InsertChildAt(newParent, newItem, pos);
  selectedItem = newItem;
  UpdateCursorPosWithDesiredOffset(0);
  EnterInsertMode();
}

void CreateItemInside() {
  Item* newItem = CreateEmptyItem(EMPTY_ITEM_TEXT_CAPACITY);
  Item* newParent = selectedItem;

  newParent->isOpen = Open;
  i32 pos = 0;

  InsertChildAt(newParent, newItem, pos);
  selectedItem = newItem;
  UpdateCursorPosWithDesiredOffset(0);
  EnterInsertMode();
}

void GoDown() {
  UpdateSelection(GetItemBelow(selectedItem));
}

void GoUp() {
  UpdateSelection(GetItemAbove(selectedItem));
}

void GoLeft() {
  UpdateCursorPosWithDesiredOffset(cursor.pos - 1);
}

void GoRight() {
  UpdateCursorPosWithDesiredOffset(cursor.pos + 1);
}

void JumpDown() {
  UpdateSelection(NextSibling(selectedItem));
}

void JumpUp() {
  UpdateSelection(PrevSibling(selectedItem));
}

void JumpLeft() {
  if (!IsRoot(selectedItem->parent)) {
    UpdateSelection(selectedItem->parent);
  }
}

void JumpRight() {
  if (selectedItem->childrenLen > 0) {
    selectedItem->isOpen = Open;
    UpdateSelection(selectedItem->children[0]);
  }
}

void SwapDown() {
  MoveItemDown(selectedItem);
}

void SwapUp() {
  MoveItemUp(selectedItem);
}

void SwapLeft() {
  MoveItemLeft(selectedItem);
}

void SwapRight() {
  MoveItemRight(selectedItem);
}

void OpenSelected() {
  if (!selectedItem->isOpen && selectedItem->childrenLen > 0)
    selectedItem->isOpen = Open;
  else if (selectedItem->childrenLen > 0) {
    UpdateSelection(selectedItem->children[0]);
  }
}

void CloseSelected() {
  if (selectedItem->isOpen)
    selectedItem->isOpen = Closed;
  else if (!IsRoot(selectedItem->parent))
    UpdateSelection(selectedItem->parent);
}

void RemoveCurrentChar() {
  if (cursor.pos < selectedItem->textLen) {
    RemoveChars(selectedItem, cursor.pos, cursor.pos);
  }
}

void DeleteCurrentItem() {
  Item* itemToSelect = GetItemToSelectAfterDeleting(selectedItem);
  DeleteItem(selectedItem);
  UpdateSelection(itemToSelect);
}

void SelectLastItem() {
  Item* mostNested = root;
  while (mostNested->isOpen) {
    mostNested = mostNested->children[mostNested->childrenLen - 1];
  }
  UpdateSelection(mostNested);
}

void SelectFirstItem() {
  UpdateSelection(root->children[0]);
}

void CopyLine() {
  ClipboardCopy(window, selectedItem->text, selectedItem->textLen);
}

void CopyItem() {
  i32 cap = MB(2);
  i32 bytesWritten;
  char* space = (char*)valloc(cap);
  SerializeRoot(selectedItem, space, &bytesWritten, cap);

  ClipboardCopy(window, space, bytesWritten);
  vfree(space);
}

void Paste(bool isAfter) {
  i32 size;
  c8* text = ClipboardPaste(window, &size);
  bool hasNewLine = IndexOf(text, size, '\n') >= 0;

  if (hasNewLine) {
    Item* tempRoot = ParseFileIntoRoot(text, size);

    i32 index = IndexOf(selectedItem);
    i32 pos = index;
    if (isAfter)
      pos++;

    Item* parent = selectedItem->parent;

    for (i32 i = 0; i < tempRoot->childrenLen; i++) {
      InsertChildAt(parent, tempRoot->children[0], pos);
      pos++;
    }

    DeleteItemWithoutChildren(tempRoot);
    UpdateSelection(parent->children[pos]);

  } else {
    InsertCharsAt(selectedItem, cursor.pos, text, size);
    UpdateCursorPosWithDesiredOffset(cursor.pos + size);
  }
  vfree(text);
}

void PasteAfter() {
  Paste(true);
}

void PasteBefore() {
  Paste(false);
}

void InitActions() {
  i32 i = 0;
  commands[i++] = {Key("w"), JumpWordForwardA};
  commands[i++] = {Key("W"), JumpWordForwardIgnorePunctuationA};
  commands[i++] = {Key("b"), JumpWordBackwardA};
  commands[i++] = {Key("B"), JumpWordBackwardIgnorePunctuationA};
  commands[i++] = {Key("0"), MoveToStart};
  commands[i++] = {Key("$"), MoveToEnd};
  commands[i++] = {Key("i"), EnterInsertMode};

  commands[i++] = {Key("I"), MoveToStartAndEnterInsertMode};
  commands[i++] = {Key("A"), MoveToEndAndEnterInsertMode};

  commands[i++] = {Ctrl("-"), DecFontSize};
  commands[i++] = {Ctrl("="), IncFontSize};
  commands[i++] = {Key("o"), CreateItemAfter};
  commands[i++] = {Key("O"), CreateItemBefore};
  commands[i++] = {Ctrl("o"), CreateItemInside};

  commands[i++] = {Key("j"), GoDown};
  commands[i++] = {Key("k"), GoUp};
  commands[i++] = {Key("h"), GoLeft};
  commands[i++] = {Key("l"), GoRight};

  commands[i++] = {Ctrl("j"), JumpDown};
  commands[i++] = {Ctrl("k"), JumpUp};
  commands[i++] = {Ctrl("h"), JumpLeft};
  commands[i++] = {Ctrl("l"), JumpRight};

  commands[i++] = {Alt("j"), SwapDown};
  commands[i++] = {Alt("k"), SwapUp};
  commands[i++] = {Alt("h"), SwapLeft};
  commands[i++] = {Alt("l"), SwapRight};

  commands[i++] = {Key("G"), SelectFirstItem};
  commands[i++] = {Key("gg"), SelectLastItem};

  commands[i++] = {Key("x"), RemoveCurrentChar};
  commands[i++] = {Key("di"), DeleteCurrentItem};

  commands[i++] = {Key("a"), CloseSelected};
  commands[i++] = {Key("s"), OpenSelected};

  commands[i++] = {Key("yl"), CopyLine};
  commands[i++] = {Key("yi"), CopyItem};
  commands[i++] = {Key("P"), PasteBefore};
  commands[i++] = {Key("p"), PasteAfter};

  commandsLen = i;
}
