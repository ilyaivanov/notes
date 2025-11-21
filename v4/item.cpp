#pragma once
#include "../win32.cpp"

struct Item {
  char* text;
  int textLen;
  int textCapacity;

  Item** children;
  int childrenLen;
  int childrenCapacity;

  Item* parent;
  bool isOpen;
};

Item* CreateRoot() {
  Item* res = (Item*)valloc(sizeof(Item));
  return res;
}

void AppendChild(Item* parent, Item* child) {
  size_t ps = sizeof(parent->children[0]);

  if (parent->childrenCapacity == 0) {
    parent->childrenCapacity = 10;
    parent->children = (Item**)valloc(ps * parent->childrenCapacity);
    parent->isOpen = true;
  }

  if (parent->childrenLen == parent->childrenCapacity) {
    parent->childrenCapacity = parent->childrenCapacity * 2;
    Item** newChildren = (Item**)valloc(ps * parent->childrenCapacity);
    memcpy(newChildren, parent->children, parent->childrenLen * ps);
    vfree(parent->children);
    parent->children = newChildren;
  }

  parent->children[parent->childrenLen++] = child;
  child->parent = parent;
}

Item* CreateItem(Item* parent, const char* text, int len) {
  Item* res = (Item*)valloc(sizeof(Item));
  res->textCapacity = len + 10;
  res->textLen = len;
  res->text = (char*)valloc(res->textCapacity * sizeof(char));
  memcpy(res->text, text, len);
  AppendChild(parent, res);
  return res;
}

void RemoveChars(Item* item, i32 from, i32 to) {
  char* text = item->text;
  i32 len = item->textLen;
  for (i32 i = to; i < len - 1; i++) {
    text[i + from - to] = text[i + 1];
  }
  item->textLen -= to - from + 1;
}

void InsertCharAt(Item* item, i32 at, c8 ch) {
  if (item->textLen == item->textCapacity) {
    item->textCapacity *= 2;
    char* newStr = (char*)valloc(item->textCapacity * sizeof(char));
    memcpy(newStr, item->text, item->textLen);
    vfree(item->text);
    item->text = newStr;
  }

  char* text = item->text;
  i32 len = item->textLen;
  for (i32 i = len; i > at; i--) {
    text[i] = text[i - 1];
  }
  text[at] = ch;
  item->textLen++;
}

i32 IndexOf(Item* parent, Item* item) {
  for (i32 i = 0; i < parent->childrenLen; i++)
    if (parent->children[i] == item)
      return i;

  return -1;
}

bool IsRoot(Item* item) {
  return item->parent == nullptr;
}

bool IsLast(Item* item) {
  if (!item->parent)
    return true;
  i32 index = IndexOf(item->parent, item);
  return item->parent->childrenLen == index + 1;
}

Item* GetItemBelow(Item* item) {
  if (item->isOpen)
    return item->children[0];

  Item* parent = item;
  while (parent && IsLast(parent)) {
    parent = parent->parent;
  }

  if (parent) {
    i32 index = IndexOf(parent->parent, parent);
    if (index < parent->parent->childrenLen - 1)
      return parent->parent->children[index + 1];
  }

  return nullptr;
}

Item* GetItemAbove(Item* item) {
  Item* parent = item->parent;
  i32 index = IndexOf(parent, item);
  if (index == 0 && !IsRoot(parent))
    return parent;
  else if (index == 0 && IsRoot(parent))
    return nullptr;

  Item* prev = parent->children[index - 1];
  while (prev->isOpen) {
    prev = prev->children[prev->childrenLen - 1];
  }

  return prev;
}

Item* NextSibling(Item* item) {
  Item* parent = item->parent;
  i32 index = IndexOf(parent, item);
  if (index < parent->childrenLen)
    return parent->children[index + 1];
  return nullptr;
}

Item* PrevSibling(Item* item) {
  Item* parent = item->parent;
  i32 index = IndexOf(parent, item);
  if (index > 0)
    return parent->children[index - 1];
  return nullptr;
}

struct StackEntry {
  Item* item;
  int level;
};

Item* ParseFileIntoRoot(char* file, int fileLen) {
  Item* root = CreateRoot();

  StackEntry stack[200];
  i32 stackLen = 0;

  stack[stackLen++] = {root, -1};

  i32 lineStart = 0;
  i32 i = 0;
  for (; i < fileLen; i++) {
    if (file[i] == '\n' || file[i] == '\r' || i == fileLen - 1) {
      int level = 0;
      while (file[lineStart + level] == ' ')
        level++;

      while (stack[stackLen - 1].level >= level)
        stackLen--;

      i32 textStart = lineStart + level;
      Item* res = CreateItem(stack[stackLen - 1].item, file + textStart, i - textStart);

      stack[stackLen++] = {res, level};

      while (file[i] == '\n' || file[i] == '\r')
        i++;

      lineStart = i;
    }
  }

  return root;
}
