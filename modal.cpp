#pragma once
#include "win32.cpp"
#include "drawing.cpp"

CharBuffer search;

struct FileInfo {
  c16 path[MAX_PATH];
  i32 len;
};

FileInfo files[40];
i32 filesLen;
i32 selectedFile;

bool StrEqual(c16* a, c16* b) {
  if (a == b)
    return true;
  if (!a || !b)
    return false;

  while (*a != L'\0' && *b != L'\0') {
    if (*a != *b)
      return false;
    ++a;
    ++b;
  }

  return *a == *b;
}

i32 IndexOf(c16* str, c16* substr) {
  if (!str || !substr)
    return -1;

  for (i32 i = 0; str[i]; ++i) {
    i32 j = 0;
    while (substr[j] && str[i + j] == substr[j]) {
      j++;
    }
    if (substr[j] == L'\0')
      return i; // found match
  }

  return -1; // not found
}

bool EndsWith(c16* str, i32 len, c16* end) {
  if (!str || !end)
    return false;

  i32 endLen = 0;
  for (c16* p = end; *p; ++p)
    endLen++;

  if (endLen > len)
    return false;

  c16* strEnd = str + (len - endLen);
  return IndexOf(strEnd, end) == 0;
}

void AddFile(WIN32_FIND_DATAW& info) {
  if (StrEqual(info.cFileName, (c16*)L".") || StrEqual(info.cFileName, (c16*)L"..") ||
      (!EndsWith(info.cFileName, wstrlen(info.cFileName), (c16*)L".cpp") &&
       !EndsWith(info.cFileName, wstrlen(info.cFileName), (c16*)L".txt") &&
       !EndsWith(info.cFileName, wstrlen(info.cFileName), (c16*)L".bat"))) {
    return;
  }

  if (search.len > 0 && IndexOf(info.cFileName, search.content) == -1) {
    return;
  }

  files[filesLen].len = wstrlen(info.cFileName);
  memcpy(files[filesLen].path, info.cFileName, files[filesLen].len * sizeof(c16));
  files[filesLen].path[files[filesLen].len] = L'\0';
  filesLen++;
}

void ClampSelectedFile() {
  selectedFile = clamp(selectedFile, 0, filesLen - 1);
}

void FindAllFiles() {
  filesLen = 0;
  WIN32_FIND_DATAW info;
  HANDLE handle = FindFirstFileW(L".\\*", &info);

  AddFile(info);

  while (FindNextFileW(handle, &info) && filesLen < ArrayLength(files))
    AddFile(info);

  FindClose(handle);

  selectedFile = 0;
}

void InitModal() {
  FindAllFiles();
}

void AddCharToModal(c16 ch) {
  search.content[search.len++] = ch;
  FindAllFiles();
}

void ModalGoDown() {
  selectedFile++;
  ClampSelectedFile();
}

void ModalGoUp() {
  selectedFile--;
  ClampSelectedFile();
}

void RemoveCharFromModal() {
  search.len = Max(search.len - 1, 0);
  search.content[search.len] = '\0';
  FindAllFiles();
}

void RenderModal(AppState& app) {
  Rect rect;
  i32 width = 800;
  i32 height = 1200;
  rect.x = app.size.x / 2.0f - width / 2.0f;
  rect.y = app.size.y / 2.0f - height / 2.0f;
  rect.width = width;
  rect.height = height;
  v3 bg = {0.1, 0.1, 0.1};
  PaintRect(rect, bg);

  v3 white = {1, 1, 1};
  v3 grey = {0.8, 0.8, 0.8};
  v3 green = {0.3, 0.5, 0.3};

  SetAlign(TA_LEFT);
  f32 y = rect.y + 5;
  TextOutW(app.dc, rect.x + 5, round(y), search.content, search.len);

  y += 40;
  for (i32 i = 0; i < filesLen; i++) {
    if (i == selectedFile)
      SetColors(white, green);
    else
      SetColors(grey, bg);

    TextOutW(app.dc, rect.x + 5, round(y), files[i].path, wstrlen(files[i].path));
    y += 25;
  }
}
