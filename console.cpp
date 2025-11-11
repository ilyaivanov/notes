#include <windows.h>

int len(char* str) {
  int res = 0;
  while (str[res])
    res++;
  return res;
}

void Print(char* str) {
  HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);

  DWORD bytesWritten;
  WriteFile(out, str, len(str), &bytesWritten, NULL);
}

void mainCRTStartup() {
  Print((char*)"Hello there\n");
}
