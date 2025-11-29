#pragma once
#include "item.cpp"

struct SearchEntrance {
  Item* item;
  i32 at;
};

c16 searchTerm[255];
c16 searchTermTemp[255];
i32 searchTermLen;

SearchEntrance searchResults[1024];
i32 searchResultsLen;
i32 currentSearchResult;
i32 currentSearchEntry;
i32 totalSearchEntries;

void UpdateSelection(Item* item);
void UpdateCursorPosWithDesiredOffset(i32 pos);

void ClearSearchTerm() {
  memset(searchTerm, 0, ArrayLength(searchTerm) * sizeof(searchTerm[0]));
  currentSearchEntry = 0;
  searchTermLen = 0;
}

bool HasUppercase() {
  memcpy(searchTermTemp, searchTerm, ArrayLength(searchTerm) * sizeof(searchTerm[0]));
  CharLowerW(searchTermTemp);
  return !StrEqual(searchTermTemp, searchTerm);
}

void UpdateSearchResults(Item* focused) {
  Item* stack[200];
  int stackLen = 0;

  stack[stackLen++] = focused;

  // this is used to lowercase the title, assumes title is < 4k
  // stupid fucking assumption, but hey, look at that 200 magic number above
  c16* temp = (c16*)valloc(1);
  i32 runningSearchEntry = 0;

  bool isCaseSensitive = HasUppercase();
  while (stackLen > 0) {
    Item* entry = stack[--stackLen];

    CopyItemTitle(temp, entry);
    if (!isCaseSensitive)
      CharLowerW(temp);

    i32 index = IndexOf(temp, entry->textLen, searchTerm);
    while (index >= 0) {
      if (runningSearchEntry == currentSearchEntry) {
        UpdateSelection(entry);
        UpdateCursorPosWithDesiredOffset(index);
      }
      runningSearchEntry++;
      index = IndexOfStartingFrom(temp, entry->textLen, searchTerm, index + 1);
    }

    if (IsItemOpenVisually(entry)) {
      for (i32 i = entry->childrenLen - 1; i >= 0; i--) {
        stack[stackLen++] = entry->children[i];
      }
    }
  }
  vfree(temp);
  totalSearchEntries = runningSearchEntry;
}
