#version 330 core

out vec4 FragColor;

uniform sampler2D texture1;
uniform int shape; // ugly hack, 1 is texture, 2 is square, 3 is circle
uniform vec4 color;

in vec2 texCoord;
uniform float radius;

void main() {
  if (shape == 1)
    FragColor = texture(texture1, texCoord);
  else if (shape == 2)
    FragColor = color;
  else if (shape == 3) {
    // TODO: circle will be implemented
    FragColor = color;
  }
}
