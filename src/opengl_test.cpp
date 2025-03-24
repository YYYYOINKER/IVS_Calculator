#include <iostream>
#include <GLFW/glfw3.h>

int main () {
  
  // initialize GLFW
  if ( !glfwInit() ) {
    std::cerr << "GLFW Initialization Failed!" << std::endl;
    return -1;
  }

  // window configuration
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

  // create window (width, height, title, fullscreen_monitor, share_resources)
  // NULL means "default options"
  GLFWwindow* window = glfwCreateWindow(800, 600, "Calculator Window", NULL, NULL);
  if ( !window ) {
    std::cerr << "Window creation Failed!" << std::endl;
    glfwTerminate();
    return -1;
  }

  // make the windows context current
  // Selecting which canvas to paint on when you have multiple
  glfwMakeContextCurrent(window);

  //==================================================================================
  //                                     MAIN LOOP
  //==================================================================================
  while ( !glfwWindowShouldClose(window) ) {

    // 1. Clear screen
    glClearColor(0.5f, 0.7f, 1.0f, 1.0f); // Light blue
    glClear(GL_COLOR_BUFFER_BIT);

    // 2. Draw things here later (calculator, buttons, etc.)

    // 3. Show the frame
    glfwSwapBuffers(window);

    // 4. Check for input
    glfwPollEvents();
  }

  // clean up
  glfwTerminate();
  return 0;
  
}

