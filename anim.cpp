#pragma once

typedef struct Spring {
  float target;
  float velocity;
  float current;
} Spring;

float stiffness;
float damping;

void InitAnimations() {
  stiffness = 3200;
  damping = 80;
}

void UpdateSpring(Spring* spring, float deltaSec) {
  float displacement = spring->target - spring->current;
  float springForce = displacement * stiffness;
  float dampingForce = spring->velocity * damping;

  spring->velocity += (springForce - dampingForce) * deltaSec;
  spring->current += spring->velocity * deltaSec;
}
