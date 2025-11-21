#include "win32.cpp"
#include <gl/gl.h>

#include "glFlags.h"
#include "gl.cpp"

GLuint CompileShader(GLuint shaderEnum, const char* source) {
  GLuint shader = glCreateShader(shaderEnum);
  glShaderSource(shader, 1, &source, NULL);

  glCompileShader(shader);

  int success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

  const char* shaderName = shaderEnum == GL_VERTEX_SHADER ? "Vertex" : "Fragmment";
  if (success) {
    OutputDebugStringA(shaderName);
    OutputDebugStringA("Shader Compiled\n");
  } else {
    OutputDebugStringA(shaderName);
    OutputDebugStringA("Shader Errors\n");

    char infoLog[512];
    glGetShaderInfoLog(shader, 512, NULL, infoLog);
    OutputDebugStringA(infoLog);
    OutputDebugStringA("\n");
  }
  return shader;
}

GLuint CreateProgram(const char* vertexShaderPath, const char* fragmentShaderPath) {
  FileContent vertexFile = ReadMyFileImp(vertexShaderPath);
  FileContent fragmentFile = ReadMyFileImp(fragmentShaderPath);

  GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexFile.content);
  GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentFile.content);

  GLuint program = glCreateProgram();
  glAttachShader(program, vertexShader);
  glAttachShader(program, fragmentShader);

  glLinkProgram(program);
  GLint success = 0;
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (success)
    OutputDebugStringA("Program Linked\n");
  else {
    char infoLog[512];
    glGetProgramInfoLog(program, 512, NULL, infoLog);
    OutputDebugStringA("Error during linking: \n");
    OutputDebugStringA(infoLog);
    OutputDebugStringA("\n");
  }

  // TODO: there is no error checking, just learning stuff, not writing prod code
  vfree(fragmentFile.content);
  vfree(vertexFile.content);
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
  return program;
}

typedef BOOL PFNWGLCHOOSEPIXELFORMATARBPROC(HDC hdc, const int* piAttribIList,
                                            const FLOAT* pfAttribFList, UINT nMaxFormats,
                                            int* piFormats, UINT* nNumFormats);

typedef const char* GETExt(HDC hdc);
void Win32InitOpenGL2(HDC dc) {
  WNDCLASSA wc = {};
  wc.style = CS_OWNDC; // <-- IMPORTANT: ensures window keeps its DC
  wc.lpfnWndProc = DefWindowProcA;
  wc.hInstance = GetModuleHandle(0);
  wc.lpszClassName = "Dummy_WGL";

  RegisterClassA(&wc);

  HWND dummyWnd = CreateWindowA("Dummy_WGL", "Dummy OpenGL Window", WS_OVERLAPPEDWINDOW, 0, 0, 1, 1,
                                NULL, NULL, GetModuleHandle(0), NULL);

  // ------------------------------------------------------------
  // 2. Get the dummy DC
  // ------------------------------------------------------------

  HDC dummyDC = GetDC(dummyWnd);

  // ------------------------------------------------------------
  // 3. Set a BASIC pixel format (old-style) for the dummy window
  // ------------------------------------------------------------

  PIXELFORMATDESCRIPTOR pfd = {};
  pfd.nSize = sizeof(pfd);
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 32;
  pfd.cDepthBits = 24;
  pfd.cStencilBits = 8;

  int pf = ChoosePixelFormat(dummyDC, &pfd);
  SetPixelFormat(dummyDC, pf, &pfd);

  // ------------------------------------------------------------
  // 4. Create dummy OpenGL context
  // ------------------------------------------------------------

  HGLRC dummyRC = wglCreateContext(dummyDC);
  wglMakeCurrent(dummyDC, dummyRC);

  // ------------------------------------------------------------
  // NOW you can load WGL extension functions
  // ------------------------------------------------------------

  auto wglChoosePixelFormatARB =
      (PFNWGLCHOOSEPIXELFORMATARBPROC*)(void*)wglGetProcAddress("wglChoosePixelFormatARB");

  // Step 3: choose multisample pixel format for real window
  int pixelFormat = 0;
  UINT numFormats = 0;

  // clang-format off
  int attribs[] = {
    WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
    WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
    WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
    WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
    WGL_COLOR_BITS_ARB, 32,
    // WGL_DEPTH_BITS_ARB, 24,
    // WGL_STENCIL_BITS_ARB, 8,
 
    WGL_SAMPLE_BUFFERS_ARB, GL_TRUE, // <-- enable MSAA
    WGL_SAMPLES_ARB, 4, // <-- 4x MSAA
    0
  };
  // clang-format on

  wglChoosePixelFormatARB(dc, attribs, NULL, 1, &pixelFormat, &numFormats);

  // Step 4: delete dummy context
  wglMakeCurrent(NULL, NULL);
  wglDeleteContext(dummyRC);

  // Step 5: set pixel format for real window
  SetPixelFormat(dc, pixelFormat, &pfd);

  // Step 6: create real GL context
  HGLRC rc = wglCreateContext(dc);
  wglMakeCurrent(dc, rc);

  // Step 7: enable MSAA
  glEnable(GL_MULTISAMPLE);
}
