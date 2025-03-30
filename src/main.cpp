#include "gui.h"

int main() {
  ImVec2 initDisplaySize = ImVec2(X_RES, Y_RES);
  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit())
    return 1;
  RenderGUI(InitializeGUI(initDisplaySize),initDisplaySize); // InitializeGUI() returns the window that is being rendered
  return 0;
}
