#define WIN32_LEAN_AND_MEAN
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <shellapi.h>
#include "build.h"

int main() {
  char* output = valloc(KB(128));
  int len;

  HANDLE exeFile = OpenMyFile("build.exe");
  HANDLE srcFile = OpenMyFile("build.c");

  if (GetWriteTime(exeFile) < GetWriteTime(srcFile)) {
    const char* source = "build.exe";

    const char* dest = "build.exe.old";

    CloseHandle(exeFile);
    // Sleep(100);
    if (!MoveFileEx(source, dest, MOVEFILE_REPLACE_EXISTING)) {
      PrintLastError();
    }

    // Sleep(1000);
    printf("[i] clang-cl build.c\n");
    // TODO: get status code from the CreateProcess
    RunCommand("clang-cl build.c", output, &len);
    printf("%s", output);
    if (len > 0) {
      if (!MoveFileEx(dest, source, MOVEFILE_REPLACE_EXISTING)) {
        PrintLastError();
      }
      return 1;
    } else {
      printf("[i] build.exe\n");
      RunCommand("build.exe", output, &len);
      printf("%s", output);
      return 0;
    }

  } else {
    CloseHandle(exeFile);
    printf("[i] clang-cl main.c\n");
    RunCommand("clang-cl main.c", output, &len);
    printf("%s", output);

    printf("[i] main.exe\n");
    system("main.exe");
    // ShellExecute(NULL, "open", "main.exe", NULL, NULL, SW_NORMAL);
    // RunAndForget("main.exe");
  }

  // fosda jfl;askdjf ;lasddkjf; laskk
  CloseHandle(srcFile);
}
