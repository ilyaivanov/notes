#pragma once
#include <gl/gl.h>
#include "glFlags.h"

// https://registry.khronos.org/EGL/api/KHR/khrplatform.h
// https://registry.khronos.org/OpenGL/api/GL/glcorearb.h

typedef void GlGenBuffers(GLsizei n, GLuint* buffers);
typedef void GlBindBuffer(GLenum target, GLuint buffer);
typedef void GlBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
typedef GLuint GlCreateShader(GLenum type);
typedef void GlCompileShader(GLuint shader);
typedef void GlShaderSource(GLuint shader, GLsizei count, const GLchar* const* string,
                            const GLint* length);
typedef void GlGetShaderiv(GLuint shader, GLenum pname, GLint* params);
typedef void GlGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog);

typedef GLuint GlCreateProgram(void);
typedef void GlGetProgramiv(GLuint program, GLenum pname, GLint* params);
typedef void GlGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
typedef void GlAttachShader(GLuint program, GLuint shader);
typedef void GlLinkProgram(GLuint program);
typedef void GlUseProgram(GLuint program);
typedef void GlDeleteShader(GLuint shader);

typedef void GlBindVertexArray(GLuint array);
typedef void GlVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized,
                                   GLsizei stride, const void* pointer);
typedef void GlGenVertexArrays(GLsizei n, GLuint* arrays);
typedef void GlEnableVertexAttribArray(GLuint index);
typedef GLint GlGetUniformLocation(GLuint program, const GLchar* name);
typedef void GlUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void GlGenerateMipmap(GLenum target);
typedef void GlActiveTexture(GLenum texture);

typedef void GlUniform1f(GLint location, GLfloat v0);
typedef void GlUniform2f(GLint location, GLfloat v0, GLfloat v1);
typedef void GlUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void GlUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void GlUniform1i(GLint location, GLint v0);
typedef void GlUniform2i(GLint location, GLint v0, GLint v1);
typedef void GlUniform3i(GLint location, GLint v0, GLint v1, GLint v2);
typedef void GlUniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
typedef void GlUniform1fv(GLint location, GLsizei count, const GLfloat* value);
typedef void GlUniform2fv(GLint location, GLsizei count, const GLfloat* value);
typedef void GlUniform3fv(GLint location, GLsizei count, const GLfloat* value);
typedef void GlUniform4fv(GLint location, GLsizei count, const GLfloat* value);
typedef void GlUniform1iv(GLint location, GLsizei count, const GLint* value);
typedef void GlUniform2iv(GLint location, GLsizei count, const GLint* value);
typedef void GlUniform3iv(GLint location, GLsizei count, const GLint* value);
typedef void GlUniform4iv(GLint location, GLsizei count, const GLint* value);
typedef void GlUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose,
                                const GLfloat* value);
typedef void GlUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose,
                                const GLfloat* value);
typedef void GlUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose,
                                const GLfloat* value);

typedef void GlUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose,
                                const GLfloat* value);

typedef void GLSwapControl(int interval);

GlGenBuffers* glGenBuffers;
GlBindBuffer* glBindBuffer;
GlBufferData* glBufferData;
GlCreateShader* glCreateShader;
GlCompileShader* glCompileShader;
GlShaderSource* glShaderSource;
GlGetShaderiv* glGetShaderiv;
GlGetShaderInfoLog* glGetShaderInfoLog;

GlCreateProgram* glCreateProgram;
GlAttachShader* glAttachShader;
GlLinkProgram* glLinkProgram;
GlGetProgramiv* glGetProgramiv;

GlGetProgramInfoLog* glGetProgramInfoLog;

GlUseProgram* glUseProgram;
GlDeleteShader* glDeleteShader;
GlBindVertexArray* glBindVertexArray;
GlVertexAttribPointer* glVertexAttribPointer;
GlGenVertexArrays* glGenVertexArrays;
GlEnableVertexAttribArray* glEnableVertexAttribArray;

GlGetUniformLocation* glGetUniformLocation;
GlGenerateMipmap* glGenerateMipmap;
GlActiveTexture* glActiveTexture;

GlUniform1f* glUniform1f;
GlUniform2f* glUniform2f;
GlUniform3f* glUniform3f;
GlUniform4f* glUniform4f;
GlUniform1i* glUniform1i;
GlUniform2i* glUniform2i;
GlUniform3i* glUniform3i;
GlUniform4i* glUniform4i;
GlUniform1fv* glUniform1fv;
GlUniform2fv* glUniform2fv;
GlUniform3fv* glUniform3fv;
GlUniform4fv* glUniform4fv;
GlUniform1iv* glUniform1iv;
GlUniform2iv* glUniform2iv;
GlUniform3iv* glUniform3iv;
GlUniform4iv* glUniform4iv;
GlUniformMatrix2fv* glUniformMatrix2fv;
GlUniformMatrix3fv* glUniformMatrix3fv;
GlUniformMatrix4fv* glUniformMatrix4fv;

GLSwapControl* glSwapControl;

// GlPolygonMode* glPolygonMode;

void* getProc(const char* addr) {
  return (void*)wglGetProcAddress(addr);
}

void InitFunctions() {
  glGenBuffers = (GlGenBuffers*)getProc("glGenBuffers");
  glBindBuffer = (GlBindBuffer*)getProc("glBindBuffer");
  glBufferData = (GlBufferData*)getProc("glBufferData");
  glCreateShader = (GlCreateShader*)getProc("glCreateShader");
  glCompileShader = (GlCompileShader*)getProc("glCompileShader");
  glShaderSource = (GlShaderSource*)getProc("glShaderSource");
  glGetShaderiv = (GlGetShaderiv*)getProc("glGetShaderiv");
  glGetShaderInfoLog = (GlGetShaderInfoLog*)getProc("glGetShaderInfoLog");
  glCreateProgram = (GlCreateProgram*)getProc("glCreateProgram");
  glAttachShader = (GlAttachShader*)getProc("glAttachShader");
  glLinkProgram = (GlLinkProgram*)getProc("glLinkProgram");
  glGetProgramiv = (GlGetProgramiv*)getProc("glGetProgramiv");
  glGetProgramInfoLog = (GlGetProgramInfoLog*)getProc("glGetProgramInfoLog");

  glUseProgram = (GlUseProgram*)getProc("glUseProgram");
  glDeleteShader = (GlDeleteShader*)getProc("glDeleteShader");

  glBindVertexArray = (GlBindVertexArray*)getProc("glBindVertexArray");
  glVertexAttribPointer = (GlVertexAttribPointer*)getProc("glVertexAttribPointer");
  glGenVertexArrays = (GlGenVertexArrays*)getProc("glGenVertexArrays");
  glEnableVertexAttribArray = (GlEnableVertexAttribArray*)getProc("glEnableVertexAttribArray");

  glGetUniformLocation = (GlGetUniformLocation*)getProc("glGetUniformLocation");
  glUniform4f = (GlUniform4f*)getProc("glUniform4f");
  glGenerateMipmap = (GlGenerateMipmap*)getProc("glGenerateMipmap");
  glActiveTexture = (GlActiveTexture*)getProc("glActiveTexture");

  glUniform1f = (GlUniform1f*)getProc("glUniform1f");
  glUniform2f = (GlUniform2f*)getProc("glUniform2f");
  glUniform3f = (GlUniform3f*)getProc("glUniform3f");
  glUniform4f = (GlUniform4f*)getProc("glUniform4f");
  glUniform1i = (GlUniform1i*)getProc("glUniform1i");
  glUniform2i = (GlUniform2i*)getProc("glUniform2i");
  glUniform3i = (GlUniform3i*)getProc("glUniform3i");
  glUniform4i = (GlUniform4i*)getProc("glUniform4i");
  glUniform1fv = (GlUniform1fv*)getProc("glUniform1fv");
  glUniform2fv = (GlUniform2fv*)getProc("glUniform2fv");
  glUniform3fv = (GlUniform3fv*)getProc("glUniform3fv");
  glUniform4fv = (GlUniform4fv*)getProc("glUniform4fv");
  glUniform1iv = (GlUniform1iv*)getProc("glUniform1iv");
  glUniform2iv = (GlUniform2iv*)getProc("glUniform2iv");
  glUniform3iv = (GlUniform3iv*)getProc("glUniform3iv");
  glUniform4iv = (GlUniform4iv*)getProc("glUniform4iv");
  glUniformMatrix2fv = (GlUniformMatrix2fv*)getProc("glUniformMatrix2fv");
  glUniformMatrix3fv = (GlUniformMatrix3fv*)getProc("glUniformMatrix3fv");
  glUniformMatrix4fv = (GlUniformMatrix4fv*)getProc("glUniformMatrix4fv");

  glSwapControl = (GLSwapControl*)getProc("wglSwapIntervalEXT");
}
