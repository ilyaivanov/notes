#include <windows.h>

void GetFoo(char* buffer);

int len(char* str) {
  int res = 0;
  while (str[res] != '\0')
    res++;
  return res;
}

int main() {
  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

  DWORD charsWritten;

  // SetConsoleOutputCP(CP_UTF8);

  char buffer[256] = {0};

  GetFoo(buffer);
  WriteConsoleA(hConsole, buffer, len(buffer), &charsWritten, 0);

  return 0;
}
