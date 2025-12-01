#version 330 core

layout(location = 0) in vec3 aPos;
// layout (location = 1) in vec3 aColor;
// layout(location = 1) in vec2 aTexCoord;

// uniform mat4 modelMY;
// uniform mat4 modelM;
// uniform mat4 viewM;
// uniform mat4 projectionM;

// out vec3 myColor;
// out vec2 texCoord;
void main() {
  // gl_Position = projectionM * viewM * modelM * vec4(aPos, 1.0);
  // texCoord = aTexCoord;

  // myColor = aColor;
  gl_Position = vec4(aPos, 1.0);
}
