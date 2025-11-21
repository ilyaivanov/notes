#include "windows.h"

int len(char* str) {
  int res = 0;
  while (str[res] != '\0')
    res++;
  return res;
}

void mainCRTStartup() {
  HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
  char* str = "hello world\r\n";
  DWORD written;
  WriteFile(handle, str, len(str), &written, 0);
  WriteFile(handle, str, len(str), &written, 0);

  ExitProcess(0);
}
