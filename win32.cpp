#pragma once

#include <math.h>
#include <dwmapi.h>
#include <stdint.h>
#include <windows.h>
#include <windowsx.h>

extern "C" int _fltused = 0x9875;
typedef char c8;
typedef wchar_t c16;

typedef int8_t i8;
typedef uint8_t u8;

typedef int16_t i16;
typedef uint16_t u16;

typedef int32_t i32;
typedef uint32_t u32;

typedef int64_t i64;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

void fail() {
  int* a = 0;
  *a = 1;
}

struct v2 {
  f32 x;
  f32 y;
};

v2 vec2(f32 x, f32 y) {
  v2 res = {x, y};
  return res;
}

v2 operator-(const v2& a, const v2& b) {
  v2 res = {a.x - b.x, a.y - b.y};
  return res;
}

v2 operator+(const v2& a, const v2& b) {
  v2 res = {a.x + b.x, a.y + b.y};
  return res;
}

v2& operator+=(v2& a, const v2& b) {
  a = a + b;
  return a;
}

struct vi2 {
  i32 x;
  i32 y;
};

struct v3 {
  f32 x;
  f32 y;
  f32 z;
};

v3 vec3(f32 x, f32 y, f32 z) {
  v3 res = {x, y, z};
  return res;
}

bool operator==(v3& a, v3& b) {
  return a.x == b.x && a.y == b.y && a.z == b.z;
}

v3 operator*(v3 v, f32 s) {
  v3 res = {v.x * s, v.y * s, v.z * s};
  return res;
}
v3 operator*(f32 s, v3 v) {
  v3 res = {v.x * s, v.y * s, v.z * s};
  return res;
}

v3& operator*=(v3& v, f32 s) {
  v = s * v;
  return v;
}

bool operator!=(v3& a, v3& b) {
  return a.x != b.x || a.y != b.y || a.z != b.z;
}

i32 Max(i32 a, i32 b) {
  if (a > b)
    return a;
  return b;
}

i32 Min(i32 a, i32 b) {
  if (a < b)
    return a;
  return b;
}

f32 lerp(f32 from, f32 to, f32 v) {
  return (1 - v) * from + to * v;
}

f32 clamp(f32 val, f32 min, f32 max) {
  if (val < min)
    return min;
  if (val > max)
    return max;
  return val;
}

i32 clamp(i32 v, i32 min, i32 max) {
  if (v < min)
    return min;
  if (v > max)
    return max;
  return v;
}

struct v4 {
  union {
    struct {
      f32 x;
      f32 y;
      f32 z;
      f32 w;
    };
  };
};

struct AppState {
  HDC dc;
  HWND window;
  bool isFullscreen;
  bool isRunning;
  f32 appTimeMs;
  f32 lastFrameTimeMs;
  v2 size;
};

typedef struct MyBitmap {
  u32 width;
  u32 height;
  u32 bytesPerPixel;
  u32* pixels;
} MyBitmap;

#define ArrayLength(array) (i32)(sizeof(array) / sizeof(array[0]))
#define KB(v) (1024 * v)
#define MB(v) (KB(1024 * v))

inline void* valloc(size_t size) {
  return VirtualAlloc(0, size, MEM_COMMIT, PAGE_READWRITE);
};

inline void vfree(void* ptr) {
  VirtualFree(ptr, 0, MEM_RELEASE);
};

#pragma function(memcpy)
void* memcpy(void* dest, void* src, size_t n) {
  char* csrc = (char*)src;
  char* cdest = (char*)dest;

  for (size_t i = 0; i < n; i++)
    cdest[i] = csrc[i];

  return dest;
}

// Increasing Read Bandwidth with SIMD Instructions
// https://www.computerenhance.com/p/increasing-read-bandwidth-with-simd

#pragma function(memset)
void* memset(void* dest, int c, size_t count) {
  char* bytes = (char*)dest;
  while (count--) {
    *bytes++ = (char)c;
  }
  return dest;
}

typedef LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HWND OpenWindow(WindowProc* proc) {
  HINSTANCE instance = GetModuleHandle(0);
  WNDCLASSW windowClass = {};
  windowClass.hInstance = instance;
  windowClass.lpfnWndProc = proc;
  windowClass.lpszClassName = L"MyClassName";
  windowClass.style = CS_VREDRAW | CS_HREDRAW;
  windowClass.hCursor = LoadCursor(0, IDC_ARROW);
  windowClass.hbrBackground = CreateSolidBrush(0);

  RegisterClassW(&windowClass);

  HDC screenDc = GetDC(0);

  int screenWidth = GetDeviceCaps(screenDc, HORZRES);
  //  int screenHeight = GetDeviceCaps(screenDc, VERTRES);

  int width = 1200;
  int height = 1600;

  HWND win = CreateWindowW(windowClass.lpszClassName, L"Notes", WS_OVERLAPPEDWINDOW,
                           screenWidth - width - 1200, 0, width, height, 0, 0, instance, 0);

  BOOL USE_DARK_MODE = TRUE;
  SUCCEEDED(DwmSetWindowAttribute(win, DWMWA_USE_IMMERSIVE_DARK_MODE, &USE_DARK_MODE,
                                  sizeof(USE_DARK_MODE)));
  return win;
}

// https://devblogs.microsoft.com/oldnewthing/20100412-00/?p=14353
WINDOWPLACEMENT prevWindowDimensions = {};
void SetFullscreen(HWND window, i32 isFullscreen) {
  prevWindowDimensions.length = sizeof(prevWindowDimensions);
  DWORD style = GetWindowLong(window, GWL_STYLE);
  if (isFullscreen) {
    MONITORINFO monitorInfo = {};
    monitorInfo.cbSize = sizeof(monitorInfo);
    if (GetWindowPlacement(window, &prevWindowDimensions) &&
        GetMonitorInfo(MonitorFromWindow(window, MONITOR_DEFAULTTOPRIMARY), &monitorInfo)) {
      SetWindowLong(window, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);

      SetWindowPos(window, HWND_TOP, monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top,
                   monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
                   monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
                   SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
  } else {
    SetWindowLong(window, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
    SetWindowPlacement(window, &prevWindowDimensions);
    SetWindowPos(window, NULL, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
  }
}

void Win32InitOpenGL(HDC dc) {
  PIXELFORMATDESCRIPTOR DesiredPixelFormat = {};
  DesiredPixelFormat.nSize = sizeof(DesiredPixelFormat);
  DesiredPixelFormat.nVersion = 1;
  DesiredPixelFormat.iPixelType = PFD_TYPE_RGBA;
  DesiredPixelFormat.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
  DesiredPixelFormat.cColorBits = 32;
  DesiredPixelFormat.cAlphaBits = 8;
  DesiredPixelFormat.iLayerType = PFD_MAIN_PLANE;

  int SuggestedPixelFormatIndex = ChoosePixelFormat(dc, &DesiredPixelFormat);

  PIXELFORMATDESCRIPTOR SuggestedPixelFormat;
  DescribePixelFormat(dc, SuggestedPixelFormatIndex, sizeof(SuggestedPixelFormat),
                      &SuggestedPixelFormat);
  SetPixelFormat(dc, SuggestedPixelFormatIndex, &SuggestedPixelFormat);

  HGLRC OpenGLRC = wglCreateContext(dc);
  wglMakeCurrent(dc, OpenGLRC);
}

typedef BOOL PFNWGLCHOOSEPIXELFORMATARBPROC(HDC hdc, const int* piAttribIList,
                                            const FLOAT* pfAttribFList, UINT nMaxFormats,
                                            int* piFormats, UINT* nNumFormats);

inline BOOL IsKeyPressed(u32 code) {
  return (GetKeyState(code) >> 15) & 1;
}

i64 GetMyFileSize(const wchar_t* path) {
  HANDLE file = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

  LARGE_INTEGER size = {};
  GetFileSizeEx(file, &size);

  CloseHandle(file);
  return (i64)size.QuadPart;
}

void ReadFileInto(const wchar_t* path, u32 fileSize, char* buffer) {
  HANDLE file = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

  DWORD bytesRead;
  ReadFile(file, buffer, fileSize, &bytesRead, 0);
  CloseHandle(file);
}

void WriteMyFile(const wchar_t* path, char* content, int size) {
  HANDLE file = CreateFileW(path, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);

  DWORD bytesWritten;
  WriteFile(file, content, size, &bytesWritten, 0);
  CloseHandle(file);
}

typedef struct FileContent {
  char* content;
  i32 size;
} FileContent;

FileContent ReadMyFileImp(const wchar_t* path) {
  HANDLE file = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

  LARGE_INTEGER size;
  GetFileSizeEx(file, &size);

  u32 fileSize = (u32)size.QuadPart;

  void* buffer = valloc(fileSize);

  DWORD bytesRead;
  ReadFile(file, buffer, fileSize, &bytesRead, 0);
  CloseHandle(file);

  FileContent res = {};
  res.content = (char*)buffer;
  res.size = bytesRead;
  return res;
}

inline i64 GetPerfFrequency() {
  LARGE_INTEGER res;
  QueryPerformanceFrequency(&res);
  return res.QuadPart;
}

inline i64 GetPerfCounter() {
  LARGE_INTEGER res;
  QueryPerformanceCounter(&res);
  return res.QuadPart;
}

//
//
//

struct CharBuffer {
  wchar_t content[512];
  i32 len;
};

inline void AddChar(CharBuffer* buff, wchar_t ch) {
  buff->content[buff->len++] = ch;
}

void Append(CharBuffer* buff, const wchar_t* str) {
  while (*str) {
    AddChar(buff, *str);
    str++;
  }
}

i32 abs(i32 a) {
  if (a < 0)
    return -a;
  return a;
}

f32 abs(f32 a) {
  if (a < 0)
    return -a;
  return a;
}

i32 round(f32 a) {
  return i32(a + 0.5);
}

void Append(CharBuffer* buff, i64 val) {
  if (val < 0) {
    AddChar(buff, L'-');
    val = -val;
  }

  if (val == 0)
    AddChar(buff, L'0');

  u32 templen = 0;
  wchar_t temp[32];
  while (val != 0) {
    // abs because -val above can produce negative result for INT_MIN
    i32 rem = val % 10;
    i32 s = abs(rem);
    temp[templen++] = L'0' + s;
    val /= 10;
  }

  for (i32 i = templen - 1; i >= 0; i--)
    AddChar(buff, temp[i]);
}

void Append(CharBuffer* buff, i32 val) {
  Append(buff, (i64)val);
}

void Append(CharBuffer* buff, f32 val) {
  if (val != val)
    Append(buff, L"NaN");
  else {
    Append(buff, (i64)val);
    AddChar(buff, L'.');
    Append(buff, (i64)((val - (i64)val) * 10));
  }
}

void Append(CharBuffer* buff, v2 vec) {
  Append(buff, vec.x);
  Append(buff, L",");
  Append(buff, vec.y);
}

void Append(CharBuffer* buff, v3 vec) {
  Append(buff, vec.x);
  Append(buff, L",");
  Append(buff, vec.y);
  Append(buff, L",");
  Append(buff, vec.z);
}

inline void AppendLine(CharBuffer* buff) {
  AddChar(buff, L'\n');
}

void AppendLine(CharBuffer* buff, v2 vec) {
  Append(buff, vec);
  AppendLine(buff);
}

void AppendLine(CharBuffer* buff, v3 vec) {
  Append(buff, vec);
  AppendLine(buff);
}

void AppendLine(CharBuffer* buff, const wchar_t* str) {
  Append(buff, str);
  AppendLine(buff);
}

void AppendLine(CharBuffer* buff, f32 val) {
  if (val != val)
    Append(buff, L"NaN");
  else {
    Append(buff, (i32)val);
    AddChar(buff, L'.');
    Append(buff, (i32)((val - (i32)val) * 10));
  }
  AppendLine(buff);
}

void AppendLine(CharBuffer* buff, i32 val) {
  Append(buff, val);
  AppendLine(buff);
}

DWORD RunCommand(char* cmd, char* output, u32* len) {
  *len = 0;
  HANDLE hRead, hWrite;
  SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};

  if (!CreatePipe(&hRead, &hWrite, &sa, KB(128)))
    return 0;

  // Ensure the read handle to the pipe is not inherited.
  SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0);

  PROCESS_INFORMATION pi;
  STARTUPINFOA si = {};
  si.cb = sizeof(STARTUPINFOA);
  si.dwFlags = STARTF_USESTDHANDLES;
  si.hStdOutput = hWrite;
  si.hStdError = hWrite;
  si.hStdInput = NULL;

  if (!CreateProcessA(NULL, cmd, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
    CloseHandle(hWrite);
    CloseHandle(hRead);
    return 0;
  }

  CloseHandle(hWrite);

  WaitForSingleObject(pi.hProcess, INFINITE);

  DWORD bytesRead = 0, totalRead = 0;
  while (ReadFile(hRead, output + totalRead, 1, &bytesRead, NULL) && totalRead < *len - 1) {
    totalRead += bytesRead;
  }

  output[totalRead] = '\0';
  *len = totalRead;

  DWORD statusCode = 0;
  GetExitCodeProcess(pi.hProcess, &statusCode);

  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);
  CloseHandle(hRead);
  return statusCode;
}

i32 strlen(char* str) {
  i32 res = 0;
  while (str[res])
    res++;
  return res;
}

bool EndsWith(char* str, i32 len, char* substr) {
  i32 substrLen = strlen(substr);
  i32 currentSubstr = substrLen - 1;
  for (i32 i = len - 1; i >= 0; i--) {
    if (str[len - substrLen + currentSubstr] == substr[currentSubstr]) {
      currentSubstr--;
      if (currentSubstr == -1)
        return true;
    } else {
      return false;
    }
  }
  return false;
}

i32 wstrlen(c16* str) {
  i32 res = 0;
  while (str[res] != L'\0')
    res++;
  return res;
}

#pragma function(memcpy)
extern "C" void* memcpy(void* dst, const void* src, size_t n) {
  unsigned char* d = (unsigned char*)dst;
  const unsigned char* s = (const unsigned char*)src;
  while (n--)
    *d++ = *s++;
  return dst;
}

c16* ClipboardPaste(HWND window, i32* size) {
  OpenClipboard(window);
  HANDLE hClipboardData = GetClipboardData(CF_UNICODETEXT);
  c16* pchData = (c16*)GlobalLock(hClipboardData);
  c16* res = NULL;
  if (pchData) {
    i32 len = wstrlen(pchData);
    res = (c16*)valloc(len * sizeof(c16));
    memcpy(res, pchData, len * sizeof(c16));
    GlobalUnlock(hClipboardData);
    *size = len;
  } else {
    OutputDebugStringA("Failed to capture clipboard\n");
  }
  CloseClipboard();
  return res;
}

// https://www.codeproject.com/Articles/2242/Using-the-Clipboard-Part-I-Transferring-Simple-Tex
void ClipboardCopy(HWND window, wchar_t* text, i32 len) {
  if (OpenClipboard(window)) {
    EmptyClipboard();

    HGLOBAL hClipboardData = GlobalAlloc(GMEM_DDESHARE, (len + 1) * sizeof(c16));

    c16* pchData = (c16*)GlobalLock(hClipboardData);

    for (i32 i = 0; i < len; i++) {
      pchData[i] = text[i];
    }
    pchData[len] = L'\0';

    GlobalUnlock(hClipboardData);

    SetClipboardData(CF_UNICODETEXT, hClipboardData);

    CloseClipboard();
  }
}
