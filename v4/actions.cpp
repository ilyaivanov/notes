#pragma once
#include <windows.h>
#include <shellapi.h>
#include "item.cpp"
#include "vimutils.cpp"
#include "commandBuffer.cpp"
#include "../anim.cpp"

#define EMPTY_ITEM_TEXT_CAPACITY 8

enum Mode { Normal, Insert, ReplaceChar, SearchLocal, VisualLine, Visual, Modal };

struct Cursor {
  i32 pos;
  i32 desiredOffset;
};

AppState appState;
Mode mode = Normal;
i32 fontSize;
f32 lineHeight;
Cursor cursor;
Item* root;
Item* selectedItem;
Item* itemFocused;
c16 errorMessage[255];

struct SearchEntrance {
  Item* item;
  i32 at;
};

c16 searchTerm[255];
i32 searchTermLen;

SearchEntrance searchResults[1024];
i32 searchResultsLen;
i32 currentSearchResult;

Spring scrollOffset;
v2 pagePadding = {20, 5};
f32 pageHeight;

// these are the functions from the main which I don;t want to bring here
void UpdateCursorPosWithDesiredOffset(i32 pos);
void UpdateFontSize();
void EnterInsertMode();
void FindPositionBasedOnDesiredOffset();

f32 GetFontHeight();

// TODO: this function should take rect as a param
f32 GetItemOffsetOnPage(Item* item) {
  v2 runningPos = pagePadding;
  StackEntry stack[200];
  int stackLen = 0;

  stack[stackLen++] = {root, -1};

  while (stackLen > 0) {
    StackEntry entry = stack[--stackLen];
    bool isClosed = !entry.item->isOpen && entry.item->childrenLen > 0;

    if (entry.item == item)
      return runningPos.y;

    if (entry.level >= 0) {
      runningPos.y += GetFontHeight();
    }
    if (!isClosed) {
      for (i32 i = entry.item->childrenLen - 1; i >= 0; i--) {
        stack[stackLen++] = {entry.item->children[i], entry.level + 1};
      }
    }
  }

  return -1;
}

f32 ClampScroll(f32 val) {
  if (val < 0)
    return 0;
  if (val > pageHeight - appState.size.y)
    return pageHeight - appState.size.y;
  return val;
}

void CenterOnItem() {
  scrollOffset.target = ClampScroll(GetItemOffsetOnPage(selectedItem) - appState.size.y / 2.0f);
}

void BottomOnItem() {
  scrollOffset.target =
      ClampScroll(GetItemOffsetOnPage(selectedItem) - appState.size.y + GetFontHeight());
}

void TopOnItem() {
  scrollOffset.target = ClampScroll(GetItemOffsetOnPage(selectedItem));
}

void CenterOnItemIfOutOfBounds() {
  f32 cursorY = GetItemOffsetOnPage(selectedItem);
  if (cursorY < scrollOffset.target)
    CenterOnItem();
  if (cursorY > scrollOffset.target + appState.size.y)
    CenterOnItem();
}

void UpdateSelection(Item* item) {
  if (item) {
    selectedItem = item;
    FindPositionBasedOnDesiredOffset();
    CenterOnItemIfOutOfBounds();
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

void RemoveUntilEnd() {
  RemoveChars(selectedItem, cursor.pos, selectedItem->textLen - 1);
}

void ReplaceUntilEnd() {
  RemoveUntilEnd();
  EnterInsertMode();
}

void RemoveUntilStart() {
  RemoveChars(selectedItem, 0, cursor.pos - 1);
  UpdateCursorPosWithDesiredOffset(0);
}

void CopyUntilStart() {
  ClipboardCopy(appState.window, selectedItem->text, cursor.pos);
}

void ReplaceUntilStart() {
  RemoveUntilStart();
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

void IncLineHeight() {
  lineHeight += 0.1f;
  UpdateFontSize();
}

void DecLineHeight() {
  lineHeight -= 0.1f;
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
  Item* mostNested = itemFocused;
  while (mostNested->isOpen) {
    mostNested = mostNested->children[mostNested->childrenLen - 1];
  }
  UpdateSelection(mostNested);
}

void SelectFirstItem() {
  if (IsRoot(itemFocused))
    UpdateSelection(itemFocused->children[0]);
  else
    UpdateSelection(itemFocused);
}

void CopyLine() {
  ClipboardCopy(appState.window, selectedItem->text, selectedItem->textLen);
}

void CopyUntilEnd() {
  ClipboardCopy(appState.window, selectedItem->text + cursor.pos,
                selectedItem->textLen - cursor.pos);
}

void CopyItem() {
  i32 cap = MB(2);
  i32 bytesWritten;
  c16* space = (c16*)valloc(cap);
  SerializeRoot(selectedItem, space, &bytesWritten, cap);

  ClipboardCopy(appState.window, space, bytesWritten);
  vfree(space);
}

void Paste(bool isAfter) {
  i32 size;
  c16* text = ClipboardPaste(appState.window, &size);
  bool hasNewLine = IndexOf(text, size, L'\n') >= 0;

  if (hasNewLine) {
    Item* tempRoot = ParseFileIntoRoot(text, size);

    i32 index = IndexOf(selectedItem);
    i32 pos = index;
    if (isAfter)
      pos++;

    Item* parent = selectedItem->parent;

    for (i32 i = 0; i < tempRoot->childrenLen; i++) {
      InsertChildAt(parent, tempRoot->children[i], pos);
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

// I need this bullshit because on ShellExecute I will receive pending WM_CHAR event while
// processing WM_KEYDOWN thus I need to "flush" all events after WM_KEYDOWN by adding new event and
// processing it separatelly
#define WM_LAUNCH_URL_UNDER_CURSOR (WM_APP + 1)

void PostLaunchUrlEvent() {
  PostMessage(appState.window, WM_LAUNCH_URL_UNDER_CURSOR, 0, 0);
}

const c16* validPrefixes[] = {L"http:", L"https:", L"file:"};
void OpenUrlUnderCursor() {
  c16* url = (c16*)valloc(KB(1024));
  i32 current = cursor.pos;
  if (selectedItem->text[current] == L' ' || current == selectedItem->textLen)
    current--;

  i32 start = current;
  i32 end = current;

  while (selectedItem->text[start] != L' ' && start > 0)
    start--;

  if (selectedItem->text[start] == L' ')
    start++;

  while (selectedItem->text[end] != L' ' && end < selectedItem->textLen - 1)
    end++;

  if (selectedItem->text[end] == L' ')
    end--;

  c16* urlStart = selectedItem->text + start;
  i32 urlLen = end - start + 1;

  bool isKnownUrl = false;
  for (i32 i = 0; i < ArrayLength(validPrefixes); i++) {
    if (StartsWith(urlStart, urlLen, (c16*)validPrefixes[i])) {
      isKnownUrl = true;
      break;
    }
  }

  c16* runningUrl = url;
  if (!isKnownUrl) {
    const c16* defaultPrefix = L"https://";
    i32 prefixLen = wstrlen((c16*)defaultPrefix);
    memcpy(url, defaultPrefix, prefixLen * sizeof(c16));
    runningUrl += prefixLen;
  }

  memcpy(runningUrl, urlStart, urlLen * sizeof(c16));

  HINSTANCE res = ShellExecuteW(NULL, L"open", url, NULL, NULL, SW_SHOWNORMAL);
  if ((u64)res <= 32) {
    CharBuffer error = {};
    Append(&error, L"Can't open: '");
    Append(&error, url);
    Append(&error, L"'");

    for (i32 i = 0; i < error.len; i++)
      errorMessage[i] = error.content[i];

    errorMessage[error.len] = L'\0';
  }

  vfree(url);
}

void JumpHalfPageDown() {
  i32 times = round(appState.size.y / 2.0f / GetFontHeight());
  for (i32 i = 0; i < times; i++) {
    GoDown();
  }
}

void JumpHalfPageUp() {
  i32 times = round(appState.size.y / 2.0f / GetFontHeight());
  for (i32 i = 0; i < times; i++) {
    GoUp();
  }
}

void EnterSearchLocal() {
  mode = SearchLocal;
  searchTermLen = 0;
}

void UpdateSearchResults() {
  Item* center = root->children[9];
  scrollOffset.target = GetItemOffsetOnPage(center) - appState.size.y / 2.0f;
}

void FocusOnCurrentItem() {

  itemFocused = selectedItem;
}

void FocusOnParent() {
  if (itemFocused->parent) {
    itemFocused = itemFocused->parent;
  }
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

  commands[i++] = {Ctrl("d"), JumpHalfPageDown};
  commands[i++] = {Ctrl("u"), JumpHalfPageUp};
  commands[i++] = {Key("zz"), CenterOnItem};
  commands[i++] = {Key("zt"), TopOnItem};
  commands[i++] = {Key("zb"), BottomOnItem};

  commands[i++] = {Alt("j"), SwapDown};
  commands[i++] = {Alt("k"), SwapUp};
  commands[i++] = {Alt("h"), SwapLeft};
  commands[i++] = {Alt("l"), SwapRight};

  commands[i++] = {Key("gg"), SelectFirstItem};
  commands[i++] = {Key("G"), SelectLastItem};

  commands[i++] = {Key("x"), RemoveCurrentChar};
  commands[i++] = {Key("dn"), DeleteCurrentItem};

  commands[i++] = {Key("a"), CloseSelected};
  commands[i++] = {Key("s"), OpenSelected};

  commands[i++] = {Key("yl"), CopyLine};
  commands[i++] = {Key("yn"), CopyItem};
  commands[i++] = {Key("P"), PasteBefore};
  commands[i++] = {Key("p"), PasteAfter};

  commands[i++] = {Key("gl"), PostLaunchUrlEvent};

  commands[i++] = {Key("/"), EnterSearchLocal};

  // Typography
  commands[i++] = {Ctrl("-"), DecFontSize};
  commands[i++] = {Ctrl("="), IncFontSize};
  commands[i++] = {Ctrl("_"), DecLineHeight};
  commands[i++] = {Ctrl("+"), IncLineHeight};

  // One two three. Four five six. Seven eight. Nine.

  // now this is where combinatorics will kick in.
  // I will start from naive approach by enumerating all possible combinations for action + motion,
  // but I will need a more flexible solution later
  commands[i++] = {Key("d0"), RemoveUntilStart};
  commands[i++] = {Key("D"), RemoveUntilEnd};

  commands[i++] = {Key("c0"), ReplaceUntilStart};
  commands[i++] = {Key("C"), ReplaceUntilEnd};

  commands[i++] = {Key("y0"), CopyUntilStart};
  commands[i++] = {Key("Y"), CopyUntilEnd};

  commands[i++] = {Ctrl("f"), FocusOnCurrentItem};
  commands[i++] = {Alt("f"), FocusOnParent};

  commandsLen = i;
}
