#version 330 core

// in vec3 myColor;
// in vec2 texCoord;

out vec4 FragColor;
// uniform float frac;
// uniform sampler2D texture1;
// uniform sampler2D texture2;

void main() {
  // FragColor = texture(texture1, texCoord);
  // FragColor = mix(texture(texture1, texCoord), texture(texture2, texCoord), frac);
  FragColor = vec4(0.3, 0.9, 0.3, 1);
}
