#pragma once
#include "win32.cpp"
#define _USE_MATH_DEFINES
#include <math.h>

v3 vec3(f32 x, f32 y, f32 z) {
  return (v3){x, y, z};
}

v4 vec4(f32 x, f32 y, f32 z, f32 w) {
  return (v4){x, y, z, w};
}
struct mat4 {
  f32 values[16];
};

inline mat4 Mat4Identity() {
  // clang-format off
    return (mat4){
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1,
    };
  // clang-format on
}

inline mat4 Mult(mat4 m1, mat4 m2) {
  mat4 result;

  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      result.values[i * 4 + j] = 0.0f;
      for (int k = 0; k < 4; ++k) {
        result.values[i * 4 + j] += m1.values[i * 4 + k] * m2.values[k * 4 + j];
      }
    }
  }

  return result;
}

inline mat4 Mat4RotateZ(mat4 mat, f32 rads) {

  f32 sin = sinf(rads);
  f32 cos = cosf(rads);

  // clang-format off
  mat4 rotation = {
    cos,  sin, 0, 0,
    -sin, cos, 0, 0,
    0,    0,   1, 0,
    0,    0,   0, 1,
  };
  // clang-format on
  return Mult(rotation, mat);
}

inline mat4 Mat4Translate(mat4 mat, float x, float y, float z) {
  // clang-format off
  mat4 trans = {
    1, 0, 0, x,
    0, 1, 0, y,
    0, 0, 1, z,
    0, 0, 0, 1,
  };
  // clang-format on

  return Mult(trans, mat);
}

inline mat4 Mat4ScaleV3f(mat4 mat, v3 v) {
  // clang-format off
  mat4 scale = {
    v.x, 0,   0,   0,
    0,   v.y, 0,   0,
    0,   0,   v.z, 0,
    0,   0,   0,   1,
  };
  // clang-format on
  return Mult(scale, mat);
}

inline mat4 Mat4TranslateAndScale(v2 t, v2 s) {
  // clang-format off
  mat4 res = {
    s.x, 0,   0,   t.x,
    0,   s.y, 0,   t.y,
    0,   0,   1,   0,
    0,   0,   0,   1,
  };
  // clang-format on
  return res;
}

inline mat4 CreateScreenProjection(f32 width, f32 height) {
  // allows me to set vecrtex coords as 0..width/height, instead of -1..+1
  // 0,0 is bottom left, not top left
  // matrix in code != matrix in math notation, details at https://youtu.be/kBuaCqaCYwE?t=3084
  // in short: rows in math are columns in code
  // clang-format off
    return (mat4){
        2.0f / width, 0, 0,
        -1, 0, 2.0f / height, 0, -1,
        0, 0, 1, 0,
        0, 0, 0, 1,
    };
  // clang-format on
}

f32 rads(f32 degress) {
  return degress / 180.0f * M_PI;
}

// taken from https://registry.khronos.org/OpenGL-Refpages/gl2.1/xhtml/gluPerspective.xml algo
inline mat4 CreatePerspective(float fovyDeg, f32 width, f32 height, float zNear, float zFar) {
  float aspect = width / height;
  float f = 1.0f / (tan(rads(fovyDeg / 2)));

  // clang-format off
  return (mat4){
      f / aspect, 0, 0, 0,
      0, f, 0, 0,
      0, 0, (zFar + zNear) / (zNear - zFar), (2 * zFar * zNear) / (zNear - zFar),
      0, 0, -1, 0,
  };
  // clang-format on
}

inline mat4 RotateAroundZ(mat4 base, float degrees) {
  float rads = degrees * M_PI / 180.0f;
  // clang-format off
    mat4 rotat = {
        cosf(rads), -sinf(rads), 0, 0,
        sinf(rads), cosf(rads), 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1,
    };
  // clang-format on
  return Mult(base, rotat);
}

v3 Norm(v3 v) {
  float len = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
  if (len == 0)
    return vec3(0, 0, 0);
  return (v3){v.x / len, v.y / len, v.z / len};
}

v2 Norm(v2 v) {
  float len = sqrtf(v.x * v.x + v.y * v.y);
  if (len == 0)
    return (v2){0, 0};
  return (v2){v.x / len, v.y / len};
}

v2 Round(v2 v) {
  v2 res = v;
  res.x = roundf(res.x);
  res.y = roundf(res.y);
  return res;
}

v2 Rotate(v2 v, float rads) {
  float c = cosf(rads);
  float s = sinf(rads);
  return (v2){v.x * c - v.y * s, v.x * s + v.y * c};
}

f32 Mag(v2 v) {
  return sqrtf(v.x * v.x + v.y * v.y);
}

float Dist(v2 a, v2 b) {
  float dx = a.x - b.x;
  float dy = a.y - b.y;
  return sqrtf(dx * dx + dy * dy);
}

float DistSquared(v2 a, v2 b) {
  float dx = a.x - b.x;
  float dy = a.y - b.y;
  return (dx * dx + dy * dy);
}

v2 SetMag(v2 v, f32 mag) {
  float scale = mag / Mag(v);
  v.x *= scale;
  v.y *= scale;
  return v;
}

v2 Limit(v2 v, f32 maxMag) {
  float magSq = v.x * v.x + v.y * v.y;
  float maxSq = maxMag * maxMag;

  if (magSq > maxSq) {
    float scale = maxMag / sqrtf(magSq);
    v.x *= scale;
    v.y *= scale;
  }
  return v;
}

v3 Cross(v3 v1, v3 v2) {
  v3 result;

  result.x = (v1.y * v2.z) - (v1.z * v2.y);
  result.y = (v1.z * v2.x) - (v1.x * v2.z);
  result.z = (v1.x * v2.y) - (v1.y * v2.x);

  return result;
}

v3 Sub(v3 v1, v3 v2) {
  return (v3){v1.x - v2.x, v1.y - v2.y, v1.z - v2.z};
}

v2 Sub(v2 a, v2 b) {
  return (v2){a.x - b.x, a.y - b.y};
}

v3 Add(v3 v1, v3 v2) {
  return (v3){v1.x + v2.x, v1.y + v2.y, v1.z + v2.z};
}

v3 MultScalar(v3 v, f32 s) {
  return (v3){v.x * s, v.y * s, v.z * s};
}

float Dot(v3 v1, v3 v2) {
  return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

// taken from \glm\ext\matrix_transform.inl:99 (lookAtRH)
// https://github.com/g-truc/glm/blob/master/glm/ext/matrix_transform.inl#L153

// NEED TO TRANSPOSE RESULTING MATRIX WHEN PASSING TO OPENGL
inline mat4 LookAt(v3 eye, v3 center, v3 up) {
  v3 f = Norm(Sub(center, eye));
  v3 s = Norm(Cross(f, up));
  v3 u = Cross(s, f);

  // clang-format off
    mat4 res = {
        s.x, u.x, -f.x, 0,
        s.y, u.y, -f.y, 0,
        s.z, u.z, -f.z, 0,
        -Dot(s, eye), -Dot(u, eye), Dot(f, eye), 1};

  // clang-format on
  return res;
}

f32 Clamp(f32 v, f32 min, f32 max) {
  if (v < min)
    return min;
  if (v > max)
    return max;
  return v;
}

f32 Abs(f32 v) {
  return (f32)(int)(v + 0.5f);
}

// V4 Operators

v4 operator+(v4 a, f32 v) {
  return v4{a.x + v, a.y + v, a.z + v, a.w + v};
}

v4 operator-(v4 a, f32 v) {
  return v4{a.x - v, a.y - v, a.z - v, a.w - v};
}

v4 operator+(v4 a, v4 b) {
  return v4{a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
}

v4 operator-(v4 a, v4 b) {
  return v4{a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
}

v4 operator*(v4 a, v4 b) {
  return v4{a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w};
}
v4 operator*(v4 a, f32 v) {
  return v4{a.x * v, a.y * v, a.z * v, a.w * v};
}
// V3 Operators

v3 operator+(v3 a, f32 v) {
  return v3{a.x + v, a.y + v, a.z + v};
}

v3 operator+(v3 a, v3 b) {
  return v3{a.x + b.x, a.y + b.y, a.z + b.z};
}

v3& operator+=(v3& a, v3 b) {
  a = a + b;
  return a;
}

v3 operator-(v3 a, v3 b) {
  return v3{a.x - b.x, a.y - b.y, a.z - b.z};
}

v3 operator-(v3 a, f32 v) {
  return v3{a.x - v, a.y - v, a.z - v};
}

v3& operator-=(v3& a, v3 b) {
  a = a - b;
  return a;
}

v3 operator*(v3 a, v3 b) {
  return v3{a.x * b.x, a.y * b.y, a.z * b.z};
}

v3 operator*(v3 a, f32 v) {
  return v3{a.x * v, a.y * v, a.z * v};
}

v3& operator*=(v3& a, v3 b) {
  a = a * b;
  return a;
}

// V2 Operators

v2 vec2(f32 x, f32 y) {
  return (v2){x, y};
}

v2 operator*(v2 a, v2 b) {
  return v2{a.x * b.x, a.y * b.y};
}

v2 operator*(v2 a, f32 c) {
  return v2{a.x * c, a.y * c};
}

v2 operator*(f32 c, v2 a) {
  return a * c;
}

v2& operator*=(v2& b, f32 a) {
  b = a * b;
  return b;
}

v2 operator/(v2 a, v2 b) {
  return v2{a.x / b.x, a.y / b.y};
}

v2 operator/(v2 a, f32 c) {
  return v2{a.x / c, a.y / c};
}

v2 operator/(f32 c, v2 a) {
  return a / c;
}

v2& operator/=(v2& b, f32 a) {
  b = a / b;

  return b;
}

v2 operator+(v2 a, v2 b) {
  return v2{a.x + b.x, a.y + b.y};
}

v2 operator+(v2 a, f32 c) {
  return v2{a.x + c, a.y + c};
}

v2 operator+(f32 c, v2 a) {
  return a + c;
}

v2& operator+=(v2& a, f32 b) {
  a = a + b;

  return a;
}

v2& operator+=(v2& a, v2& b) {
  a = a + b;

  return a;
}

v2 operator-(v2 a, v2 b) {
  return v2{a.x - b.x, a.y - b.y};
}

v2 operator-(v2 a, f32 c) {
  return v2{a.x - c, a.y - c};
}

v2 operator-(f32 c, v2 a) {
  return a - c;
}

v2& operator-=(v2& b, f32 a) {
  b = a - b;

  return b;
}
v2& operator-=(v2& a, v2& b) {
  a = a - b;

  return a;
}

bool operator==(v2& a, v2& b) {
  return a.x == b.x && a.y == b.y;
}

bool operator!=(v2& a, v2& b) {
  return a.x != b.x && a.y != b.y;
}

// Easings
// taken from https://easings.net/

float Map(float value, float fromMin, float fromMax, float toMin, float toMax) {
  return toMin + (value - fromMin) * (toMax - toMin) / (fromMax - fromMin);
}

float MapToNorm(float value, float fromMin, float fromMax) {
  return Map(value, fromMin, fromMax, 0.0, 1.0);
}

float MapFromNorm(float value, float toMin, float toMax) {
  return Map(value, 0.0, 1.0, toMin, toMax);
}

f32 EaseInCubic(f32 x) {
  return x * x * x;
}

f32 EaseInQuad(f32 x) {
  return x * x;
}

f32 EaseLinear(f32 x) {
  return x;
}

f32 EaseOutCubic(f32 x) {
  f32 v = 1 - x;
  return 1 - (v * v * v);
}

f32 EaseOutQuad(f32 x) {
  f32 v = 1 - x;
  return 1 - (v * v);
}

v3 HexToRGB(unsigned int hex) {
  v3 color;
  color.x = ((hex >> 16) & 0xFF) / 255.0f; // Red
  color.y = ((hex >> 8) & 0xFF) / 255.0f;  // Green
  color.z = (hex & 0xFF) / 255.0f;         // Blue
  return color;
}
