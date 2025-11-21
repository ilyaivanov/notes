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
  res->text = (char*)text;
  res->textLen = len;
  AppendChild(parent, res);
  return res;
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
    if (file[i] == '\n') {
      int level = 0;
      while (file[lineStart + level] == ' ')
        level++;

      while (stack[stackLen - 1].level >= level)
        stackLen--;

      Item* res = CreateItem(stack[stackLen - 1].item, file + lineStart + level, i - lineStart);

      stack[stackLen++] = {res, level};

      lineStart = i + 1;
    }
  }
  CreateItem(root, file + lineStart, i - lineStart);

  return root;
}
