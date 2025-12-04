#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define u32 uint32_t
#define i32 int32_t
#define u64 uint64_t
#define i64 int64_t

#define f32 float

inline void* valloc(size_t size) {
  return VirtualAlloc(0, size, MEM_COMMIT, PAGE_READWRITE);
};

inline void vfree(void* ptr) {
  VirtualFree(ptr, 0, MEM_RELEASE);
};

#define KB(v) (v * 1024)

void PrintLastError() {
  DWORD errorMessageID = GetLastError();
  if (errorMessageID == 0) {
    printf("No error\n");
    return;
  }

  LPSTR messageBuffer = NULL;

  FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                     FORMAT_MESSAGE_IGNORE_INSERTS,
                 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                 (LPSTR)&messageBuffer, 0, NULL);

  if (messageBuffer) {
    printf("Error %lu: %s\n", errorMessageID, messageBuffer);
    LocalFree(messageBuffer);
  } else {
    printf("Error %lu: (Could not format message)\n", errorMessageID);
  }
}

HANDLE OpenMyFile(char* path) {
  return CreateFileA(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
                     FILE_ATTRIBUTE_NORMAL, NULL);
}

u64 GetWriteTime(HANDLE file) {
  FILETIME writeTime;
  GetFileTime(file, 0, 0, &writeTime);

  return (i64)writeTime.dwHighDateTime << 32 | (i64)writeTime.dwLowDateTime;
}

inline i64 GetPerfFrequency(void) {
  LARGE_INTEGER res;
  QueryPerformanceFrequency(&res);
  return res.QuadPart;
}

inline i64 GetPerfCounter(void) {
  LARGE_INTEGER res;
  QueryPerformanceCounter(&res);
  return res.QuadPart;
}

inline i64 PerfMs(i64 start, i64 end, i64 freq) {
  return (end - start) * 1000 / freq;
}

bool StrEqual(char* s1, char* s2) {
  while (*s1 && *s2) {
    if (*s1 != *s2)
      return false;
    ++s1;
    ++s2;
  }
  return *s1 == *s2;
}

bool IsArg(i32 argc, char** args, i32 at, char* str) {
  return argc > at && StrEqual(args[at], str);
}

typedef struct {
  char* text;
  i32 len;
  i32 capacity;
} CharBuffer;

// I'm never releasing this memory on purpose
// let the OS do the job, since this is a single run process
CharBuffer CreateCharBuffer(i32 capacity) {
  CharBuffer res = {};
  res.capacity = capacity;
  res.text = valloc(capacity);
  return res;
}

void Append(CharBuffer* buff, char* str) {
  while (*str) {
    buff->text[buff->len++] = *str;
    str++;
  }
  buff->text[buff->len] = '\0';
}

i32 Run(char* command) {
  printf("[i] %s\n", command);
  return system(command);
}

i32 RunRebuildCmd(char* command) {
#ifndef SILENT_REBUILDS
  return Run(command);
#else
  return system(command);
#endif
}

void RebuildIfOld(char* fileName) {
  char exeName[128];
  char oldExeName[128];
  char srcName[128];
  sprintf(exeName, "%s.exe", fileName);
  sprintf(oldExeName, "%s.exe.old", fileName);
  sprintf(srcName, "%s.c", fileName);

  HANDLE exeFile = OpenMyFile(exeName);
  HANDLE srcFile = OpenMyFile(srcName);

  i64 exeTime = GetWriteTime(exeFile);
  i64 srcTime = GetWriteTime(srcFile);

  CloseHandle(exeFile);
  CloseHandle(srcFile);

  if (exeTime < srcTime) {
    const char* source = exeName;
    const char* dest = oldExeName;

    if (!MoveFileEx(source, dest, MOVEFILE_REPLACE_EXISTING)) {
      PrintLastError();
    }

    char buff[128];
    sprintf(buff, "clang-cl %s", srcName);

    i32 res = RunRebuildCmd(buff);
    if (res > 0) {
      if (!MoveFileEx(dest, source, MOVEFILE_REPLACE_EXISTING)) {
        PrintLastError();
      }
      ExitProcess(1);
    } else {
      RunRebuildCmd(exeName);
    }

    ExitProcess(1);
  }
}
