// build via 
// gcc win.c -o win -lglfw -lm -lGL


#include <GLFW/glfw3.h>
#include <math.h>
#include <stdint.h>

#define u64 uint64_t

int main(void) {
  GLFWwindow *window;

  /* Initialize the library */
  if (!glfwInit())
    return -1;

  /* Create a windowed mode window and its OpenGL context */
  window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
  if (!window) {
    glfwTerminate();
    return -1;
  }

  /* Make the window's context current */
  glfwMakeContextCurrent(window);

  u64 freq = glfwGetTimerFrequency();
  u64 time = glfwGetTimerValue();
  /* Loop until the user closes the window */
  while (!glfwWindowShouldClose(window)) {
    double timeSec = (double)time / (double)freq;
    float g = (sinf(timeSec) + 1.0f) / 2.0f;
    glClearColor(g, g, g, 1.0);
    /* Render here */
    glClear(GL_COLOR_BUFFER_BIT);

    /* Swap front and back buffers */
    glfwSwapBuffers(window);

    /* Poll for and process events */
    glfwPollEvents();
    time = glfwGetTimerValue();
  }

  glfwTerminate();
  return 0;
}
