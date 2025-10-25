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

struct v2 {
  f32 x;
  f32 y;
};

v2 vec2(f32 x, f32 y) {
  return (v2){x, y};
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

struct Rect {
  i32 x;
  i32 y;
  i32 width;
  i32 height;
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

  int width = 800;
  int height = 800;

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

#ifdef USE_OPENGL
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
#endif

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

FileContent ReadMyFileImp(const char* path) {
  HANDLE file = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

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

// i32 abs(i32 a) {
//   if (a < 0)
//     return -a;
//   return a;
// }

f32 abs(f32 a) {
  if (a < 0)
    return -a;
  return a;
}

void Append(CharBuffer* buff, i32 val) {
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
    temp[templen++] = L'0' + abs(val % 10);
    val /= 10;
  }

  for (i32 i = templen - 1; i >= 0; i--)
    AddChar(buff, temp[i]);
}

void Append(CharBuffer* buff, f32 val) {
  if (val != val)
    Append(buff, L"NaN");
  else {
    Append(buff, (i32)val);
    AddChar(buff, L'.');
    Append(buff, (i32)((val - (i32)val) * 10));
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

void AppendLine(CharBuffer* buff, v2 vec) {
  Append(buff, vec);
  AddChar(buff, '\n');
}

void AppendLine(CharBuffer* buff, v3 vec) {
  Append(buff, vec);
  AddChar(buff, '\n');
}

void AppendLine(CharBuffer* buff, const wchar_t* str) {
  Append(buff, str);
  AddChar(buff, L'\n');
}

void AppendLine(CharBuffer* buff, f32 val) {
  if (val != val)
    Append(buff, L"NaN");
  else {
    Append(buff, (i32)val);
    AddChar(buff, L'.');
    Append(buff, (i32)((val - (i32)val) * 10));
  }
  AddChar(buff, '\n');
}

void AppendLine(CharBuffer* buff, i32 val) {
  Append(buff, val);
  AddChar(buff, L'\n');
}
