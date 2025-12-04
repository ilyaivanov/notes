#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

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

void PrintLastError();

void RunAndForget(char* cmd) {
  PROCESS_INFORMATION pi;
  STARTUPINFO si = {sizeof(STARTUPINFO)};
  si.dwFlags = STARTF_USESTDHANDLES;

  if (!CreateProcess(NULL, cmd, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
    PrintLastError();
  }
}

void RunCommand(char* cmd, char* output, int* len) {
  HANDLE hRead, hWrite;
  SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};

  if (!CreatePipe(&hRead, &hWrite, &sa, KB(128)))
    return;

  // Ensure the read handle to the pipe is not inherited.
  SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0);

  PROCESS_INFORMATION pi;
  STARTUPINFO si = {sizeof(STARTUPINFO)};
  si.dwFlags = STARTF_USESTDHANDLES;
  si.hStdOutput = hWrite;
  si.hStdError = hWrite;
  si.hStdInput = NULL;

  if (!CreateProcess(NULL, cmd, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
    CloseHandle(hWrite);
    CloseHandle(hRead);
    return;
  }

  CloseHandle(hWrite);

  WaitForSingleObject(pi.hProcess, INFINITE);

  *len = 0;
  if (output && len) {
    DWORD bytesRead = 0, totalRead = 0;
    while (ReadFile(hRead, output + totalRead, 1, &bytesRead, NULL)) {
      totalRead += bytesRead;
    }
    output[totalRead] = '\0';
    *len = totalRead;
  }

  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);
  CloseHandle(hRead);
}

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
