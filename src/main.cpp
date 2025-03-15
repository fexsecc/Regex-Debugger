#ifdef _WIN32
#include <iostream>
#include <windows.h>
#include <wingdi.h>
#endif
#include <stdio.h>
#include <GL/gl.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include <GLFW/glfw3.h>
#include <GLES2/gl2.h>
#include <imgui_impl_opengl3_loader.h>
#include "GUI_handler.h"

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

GLFWwindow* InitializeGUI(ImVec2 initDisplaySize) { // Generate the main window
  const char* glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

  // Create window with graphics context
  GLFWwindow* window = glfwCreateWindow(initDisplaySize.x, initDisplaySize.y, "Regex-Debugger", nullptr, nullptr);
  if (window == nullptr)
      return NULL;
  
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1); // Enable vsync


  // Setup Dear Imgui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable keyboard navigation
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable gamepad navigation

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  //ImGui::StyleColorsLight();

  // Setup platform/renderer backends
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  // Our state
  bool show_main_window = true;
  bool show_demo_window = true;
  
  return window;
}

void scale(char name[], ImVec2 initSize, ImVec2 displaySize, ImVec2 initDisplaySize) {
    ImGui::Begin(name);
    ImVec2 size = ImGui::GetWindowSize();

    ImVec2 scale = ImVec2(displaySize.x / initDisplaySize.x, displaySize.y / initDisplaySize.y);

    ImGui::SetWindowSize(ImVec2(initSize.x*scale.x, initSize.y*scale.y), 0);
    ImGui::End();
}

void generateWindows(GLFWwindow* window,int &displayW, int &displayH, ImVec2 initDisplaySize, int state[]) { //This is where you put secondary windows (tabs,buttons,tables,checkboxes and other windows)
   glfwGetFramebufferSize(window, &displayW, &displayH);
   { // Create window called hello world and append into it
     ImGui::Begin("Hello world",nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoBringToFrontOnFocus); 
     ImGui::SetWindowPos(ImVec2(0, 0), 0);
     ImGui::SetWindowSize(initDisplaySize, 0);
     scale("Hello world", initDisplaySize, ImVec2(displayW, displayH), initDisplaySize); //here the initDisplaySize is the original size of the window
     ImGui::Text("yo how we doin"); // Display text (you can use format strings like printf)
     ImGui::End();
   }
   switch (state[0]) {
   case 1:
   {
     ImGui::Begin("2", nullptr, ImGuiWindowFlags_NoDecoration); //if you use the same name it changes the already existing window
     ImGui::SetWindowSize(ImVec2(150, 300), 0);
     scale("2", ImVec2(150, 300), ImVec2(displayW, displayH), initDisplaySize);
     ImGui::Text("good job, you clicked the button!");
     ImGui::End();
     break;
   }
   default:
   {
       ImGui::Begin("2", nullptr, ImGuiWindowFlags_NoDecoration);
       if (ImGui::Button("Cool Button", ImVec2(90, 40))) {
           state[0] = 1;
       }
       scale("2", ImVec2(90, 40), ImVec2(displayW, displayH), initDisplaySize);
       ImGui::End();
       break;
   }
   }
}

void RenderGUI(GLFWwindow* window,ImVec2 initDisplaySize) { 
  bool showCoolButton = true;
  ImVec4 clear_color = ImVec4(0.1f, 0.1f, 0.1f, 1.00f);
  int display_w, display_h;
  int state[1] = {}; // change this when you add another window with different states

  while (!glfwWindowShouldClose(window)) {
    // Poll and handle events (inputs, window resize, etc.)
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
    // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
    glfwPollEvents();
    if (glfwGetWindowAttrib(window, GLFW_ICONIFIED)) {
      ImGui_ImplGlfw_Sleep(10);
      continue;
    }
    
    // Start the ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Window contents are in the generateWindows() function (use Begin/End pair to create a named window)
    generateWindows(window, display_h, display_w, initDisplaySize, state);
    
    ImGui::Render();
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
  }

  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();
}

int main() {
  ImVec2 initDisplaySize = ImVec2(1280.0, 720.0);
  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit())
    return 1;
  RenderGUI(InitializeGUI(initDisplaySize),initDisplaySize); // InitializeGUI() returns the window that is being rendered
  return 0;
}
