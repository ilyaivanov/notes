#pragma once
#include "win32.cpp"
#include "vim.cpp"

bool isNumber(char ch) {
  return ch >= '0' && ch <= '9';
}

i32 atoi(char* str) {
  i32 res = 0;
  i32 pos = 0;
  while (isNumber(str[pos])) {
    res *= 10;
    res += str[pos] - '0';
    pos++;
  }

  return res;
}

i64 RunClangFormat(c16* path, Buffer& buffer) {
  i64 formatStart = GetPerfCounter();

  CharBuffer strBuffer = {};
  Append(&strBuffer, L"cmd /c clang-format ");

  Append(&strBuffer, path);
  Append(&strBuffer, L" --cursor=");
  Append(&strBuffer, buffer.cursor);
  AppendLine(&strBuffer);

  char* output = (char*)valloc(KB(100));
  u32 outputLen = 0;
  char buff[256] = {};
  for (i32 i = 0; i < strBuffer.len; i++) {
    buff[i] = (u8)strBuffer.content[i];
  }

  RunCommand((char*)buff, output, &outputLen);

  if (outputLen > 0) {
    i32 start = 0;
    while (output[start] != '\n')
      start++;
    start++;
    i32 newLen = outputLen - start;
    c8* fileStart = output + start;

    // 12 here is the index of new cursor position in JSON response { "Cursor": 36
    // I don't want to parse entire JSON yet
    int newCursor = atoi(output + 12);
    buffer.cursor = newCursor;
    i32 wideCharsCount = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, fileStart, newLen, 0, 0);
    MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, fileStart, newLen, buffer.text, wideCharsCount);
    buffer.textLen = newLen;
    vfree(output);

    i64 formatTimeMs =
        round(f32(GetPerfCounter() - formatStart) * 1000.0f / (f32)GetPerfFrequency());
    return formatTimeMs;
  }
  return 0;
}
