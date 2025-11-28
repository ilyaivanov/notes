#pragma once
#include "../win32.cpp"

enum IsOpen { Open = 1, Closed = 0, ClosedInFile = -1 };
struct Item {
  c16* text;
  int textLen;
  int textCapacity;

  Item** children;
  int childrenLen;
  int childrenCapacity;

  Item* parent;
  IsOpen isOpen;
};

Item* CreateRoot() {
  Item* res = (Item*)valloc(sizeof(Item));
  res->isOpen = Open;
  return res;
}

void CheckCapacityBeforeInsert(Item* parent) {
  size_t ps = sizeof(parent->children[0]);

  if (parent->childrenCapacity == 0) {
    parent->childrenCapacity = 10;
    parent->children = (Item**)valloc(ps * parent->childrenCapacity);
  }

  if (parent->childrenLen == parent->childrenCapacity) {
    parent->childrenCapacity = parent->childrenCapacity * 2;
    Item** newChildren = (Item**)valloc(ps * parent->childrenCapacity);
    memcpy(newChildren, parent->children, parent->childrenLen * ps);
    vfree(parent->children);
    parent->children = newChildren;
  }
}
void AppendChild(Item* parent, Item* child) {
  CheckCapacityBeforeInsert(parent);

  parent->children[parent->childrenLen++] = child;
  child->parent = parent;
}

void InsertChildAt(Item* parent, Item* child, i32 at) {
  CheckCapacityBeforeInsert(parent);

  Item** children = parent->children;
  i32 len = parent->childrenLen;
  for (i32 i = len; i > at; i--) {
    children[i] = children[i - 1];
  }

  children[at] = child;
  child->parent = parent;
  parent->childrenLen++;
}

Item* CreateEmptyItem(i32 textCapacity) {
  Item* res = (Item*)valloc(sizeof(Item));
  res->textCapacity = textCapacity;
  res->textLen = 0;
  if (res->textCapacity > 0)
    res->text = (c16*)valloc(res->textCapacity * sizeof(c16));

  return res;
}

Item* CreateItem(Item* parent, const c16* text, int len) {
  Item* res = CreateEmptyItem(len + 10);
  res->textLen = len;
  memcpy(res->text, text, len * sizeof(c16));
  AppendChild(parent, res);
  return res;
}

void RemoveChars(Item* item, i32 from, i32 to) {
  c16* text = item->text;
  i32 len = item->textLen;
  for (i32 i = to; i < len - 1; i++) {
    text[i + from - to] = text[i + 1];
  }
  item->textLen -= to - from + 1;
}

bool IsRoot(Item* item) {
  return item->parent == nullptr;
}

i32 GetItemLevel(Item* item) {
  Item* parent = item->parent;
  i32 level = 0;
  while (!IsRoot(parent)) {
    parent = parent->parent;
    level++;
  }
  return level;
}

bool IsAChildOf(Item* potentialParent, Item* child) {
  if (IsRoot(potentialParent))
    return true;

  Item* parent = child->parent;
  while (!IsRoot(parent)) {
    if (parent == potentialParent)
      return true;
    parent = parent->parent;
  }
  return false;
}

void CheckItemTextCapacity(Item* item, i32 charsToInsert) {
  if (item->textLen + charsToInsert > item->textCapacity) {
    item->textCapacity = item->textCapacity * 2 + charsToInsert;
    c16* newStr = (c16*)valloc(item->textCapacity * sizeof(c16));
    memcpy(newStr, item->text, item->textLen * sizeof(c16));
    vfree(item->text);
    item->text = newStr;
  }
}

void InsertCharAt(Item* item, i32 at, c16 ch) {
  CheckItemTextCapacity(item, 1);

  c16* text = item->text;
  i32 len = item->textLen;
  for (i32 i = len; i > at; i--) {
    text[i] = text[i - 1];
  }
  text[at] = ch;
  item->textLen++;
}

void InsertCharsAt(Item* item, i32 at, c16* text, i32 textLen) {
  CheckItemTextCapacity(item, textLen);

  for (i32 i = item->textLen; i > at; i--) {
    item->text[i + textLen - 1] = item->text[i - 1];
  }
  for (i32 i = 0; i < textLen; i++) {
    item->text[at + i] = text[i];
  }
  item->textLen += textLen;
}

i32 IndexOf(Item* item) {
  for (i32 i = 0; i < item->parent->childrenLen; i++)
    if (item->parent->children[i] == item)
      return i;

  return -1;
}

void RemoveChildAt(Item* item, i32 at) {
  for (i32 i = at; i < item->childrenLen - 1; i++) {
    item->children[i] = item->children[i + 1];
  }
  item->childrenLen--;
  if (item->childrenLen == 0)
    item->isOpen = Closed;
}

Item* GetItemToSelectAfterDeleting(Item* item) {
  Item* parent = item->parent;
  Item* itemToSelectNext = parent;
  i32 index = IndexOf(item);

  if (index < parent->childrenLen - 1) {
    itemToSelectNext = parent->children[index + 1];
  } else if (index != 0) {
    itemToSelectNext = parent->children[index - 1];
  }
  return itemToSelectNext;
}

void DeleteItemWithoutChildren(Item* item) {
  if (item->children)
    vfree(item->children);
  if (item->text)
    vfree(item->text);
  vfree(item);
}

bool IsItemOpenVisually(Item* item);
bool IsFocused(Item* item);

void DeleteItem(Item* item) {
  if (IsFocused(item))
    return;

  Item* parent = item->parent;
  i32 index = IndexOf(item);

  RemoveChildAt(parent, index);

  Item* stack[200];
  i32 stackLen = 0;
  stack[stackLen++] = item;

  while (stackLen > 0) {
    Item* itemToRemove = stack[--stackLen];

    for (i32 i = itemToRemove->childrenLen - 1; i >= 0; i--) {
      stack[stackLen++] = itemToRemove->children[i];
    }
    DeleteItemWithoutChildren(itemToRemove);
    // TODO: check for memory leaks
  }
}

bool IsLast(Item* item) {
  if (!item->parent)
    return true;
  i32 index = IndexOf(item);
  return item->parent->childrenLen == index + 1;
}

Item* GetItemBelow(Item* item) {
  if (IsItemOpenVisually(item))
    return item->children[0];

  Item* parent = item;
  while (parent && IsLast(parent)) {
    parent = parent->parent;
  }

  if (parent) {
    i32 index = IndexOf(parent);
    if (index < parent->parent->childrenLen - 1)
      return parent->parent->children[index + 1];
  }

  return nullptr;
}

Item* GetItemAbove(Item* item) {
  Item* parent = item->parent;
  i32 index = IndexOf(item);
  if (index == 0 && !IsRoot(parent))
    return parent;
  else if (index == 0 && IsRoot(parent))
    return nullptr;

  Item* prev = parent->children[index - 1];
  while (IsItemOpenVisually(prev)) {
    prev = prev->children[prev->childrenLen - 1];
  }

  return prev;
}

Item* NextSibling(Item* item) {
  Item* parent = item->parent;
  i32 index = IndexOf(item);
  if (index < parent->childrenLen)
    return parent->children[index + 1];
  return nullptr;
}

Item* PrevSibling(Item* item) {
  Item* parent = item->parent;
  i32 index = IndexOf(item);
  if (index > 0)
    return parent->children[index - 1];
  return nullptr;
}

void MoveItemRight(Item* item) {
  Item* parent = item->parent;
  i32 index = IndexOf(item);
  if (index != 0) {
    RemoveChildAt(parent, index);
    Item* newParent = parent->children[index - 1];
    AppendChild(newParent, item);
    newParent->isOpen = Open;
  }
}

void MoveItemLeft(Item* item) {
  Item* parent = item->parent;
  if (!IsFocused(parent)) {
    int index = IndexOf(parent);
    RemoveChildAt(parent, IndexOf(item));
    Item* newParent = parent->parent;
    InsertChildAt(newParent, item, index + 1);
  }
}

void MoveItemDown(Item* item) {
  Item* parent = item->parent;
  i32 index = IndexOf(item);
  if (index < parent->childrenLen - 1) {
    RemoveChildAt(parent, IndexOf(item));
    InsertChildAt(parent, item, index + 1);
  }
}

void MoveItemUp(Item* item) {
  Item* parent = item->parent;
  i32 index = IndexOf(item);
  if (index > 0) {
    RemoveChildAt(parent, IndexOf(item));
    InsertChildAt(parent, item, index - 1);
  }
}

#define isNewLine(val) (val == L'\n' || val == L'\r')

void CheckForFileFlags(Item* item) {
  if (item->isOpen == ClosedInFile)
    item->isOpen = Closed;
  else if (item->childrenLen > 0)
    item->isOpen = Open;
}

struct StackEntry {
  Item* item;
  int level;
};

Item* ParseFileIntoRoot(c16* file, int fileLen) {
  Item* root = CreateRoot();

  StackEntry stack[200];
  i32 stackLen = 0;

  stack[stackLen++] = {root, -1};

  i32 lineStart = 0;
  for (i32 i = 0; i < fileLen; i++) {
    if (isNewLine(file[i]) || i == fileLen - 1) {
      int level = 0;
      while (file[lineStart + level] == ' ')
        level++;

      while (stack[stackLen - 1].level >= level) {
        CheckForFileFlags(stack[stackLen - 1].item);

        stackLen--;
      }

      i32 textStart = lineStart + level;
      i32 textEnd = i;

      bool isClosed = EndsWith(file + textStart, textEnd - textStart, (c16*)L" /c");
      if (isClosed) {
        textEnd -= 3;
      }

      Item* res = CreateItem(stack[stackLen - 1].item, file + textStart, textEnd - textStart);

      if (isClosed)
        res->isOpen = ClosedInFile;

      stack[stackLen++] = {res, level};

      while (isNewLine(file[i]))
        i++;

      lineStart = i;
    }
  }

  while (stackLen > 0) {
    CheckForFileFlags(stack[stackLen - 1].item);
    stackLen--;
  }

  return root;
}

void SerializeRoot(Item* root, void* space, i32* bytesWritten, i32 capacity) {
  StackEntry stack[200];
  i32 stackLen = 0;

  i32 startLevel = -1;
  if (!IsRoot(root))
    startLevel = 0;

  stack[stackLen++] = {root, startLevel};

  c16* text = (c16*)space;
  i32 len = 0;
  *bytesWritten = 0;
  while (stackLen > 0) {
    StackEntry entry = stack[--stackLen];
    Item* item = entry.item;

    for (i32 i = item->childrenLen - 1; i >= 0; i--) {
      stack[stackLen++] = {item->children[i], entry.level + 1};
    }

    if (len + item->textLen > capacity)
      fail();

    if (!IsRoot(item)) {
      i32 tabSize = 2;
      for (i32 i = 0; i < entry.level * tabSize; i++)
        text[len++] = L' ';

      memcpy(text + len, item->text, item->textLen * sizeof(c16));
      len += item->textLen;

      if (!item->isOpen && item->childrenLen > 0) {
        memcpy(text + len, L" /c", 3 * sizeof(c16));
        len += 3;
      }

      text[len++] = L'\n';
    }
  }
  *bytesWritten = len;
}
