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
#include <cstring>
#include <re2/re2.h> // re2 test checks out

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

void ApplyScale(char name[], ImVec2 initSize, ImVec2 scale) {
    ImGui::Begin(name);

    ImGui::SetWindowSize(ImVec2(initSize.x*scale.x, initSize.y*scale.y), 0); //apply scale to the specifically named window

    ImGui::End();
}

void generateWindows(GLFWwindow* window,int &displayW, int &displayH, ImVec2 initDisplaySize, int state[]) { //This is where you put secondary windows (tabs,buttons,tables,checkboxes and other windows)
   glfwGetFramebufferSize(window, &displayW, &displayH);
   ImVec2 scale = ImVec2(displayW / initDisplaySize.x, displayH / initDisplaySize.y);
   { // Create window called main and append into it
     ImGui::Begin("main",nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoBringToFrontOnFocus); 
     ImGui::SetWindowPos(ImVec2(0, 0), 0);
     ApplyScale("main",ImVec2(1280,720), scale); //here the initDisplaySize is the original size of the window
     ImGui::Text("REGEX_DBG"); // Display text (you can use format strings like printf)
     ImGui::End();
   }
   { // Create a window to hold the "Valid input" and "Explanation" tabs
       ImGui::Begin("VI&Ebg", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBringToFrontOnFocus);
       ImGui::SetWindowPos(ImVec2(380 * scale.x, 20 + 300 * scale.y), 0);
       ImGui::SetWindowSize(ImVec2(900,900), 0);
       ImGui::End();
   }
   static char buf[10000] = "Example: ^([0-9])\\1{3}$"; //this is the Regex Input tab
   {
       ImGui::Begin("Input Window", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration);
       ImGui::InputText("<--regexInput", buf, IM_ARRAYSIZE(buf));
       ImGui::TextWrapped(buf);
       ImGui::SetWindowPos(ImVec2(380*scale.x,20), 0);
       ApplyScale("Input Window", ImVec2(900, 300), scale);
       ImGui::End();
   }
   switch (state[0]) {
     case 1:
     { //focused explanation window

       //create the explanation window
       ImGui::Begin("3", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
       if (ImGui::Button("Go back", ImVec2(120 * scale.x, 30 * scale.y)))
           state[0] = 0;
       ImGui::Text("This is the explanation window!");
       ImGui::SetWindowPos(ImVec2(380 * scale.x, 20 + 300 * scale.y), 0);
       ApplyScale("3", ImVec2(900, 420), scale);
       ImGui::End();
       break;
     }
     case 2:
     { //focused valid input window

       //create the valid input window
       ImGui::Begin("1", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove); //if you use the same name it changes the already existing window
       if (ImGui::Button("Go back", ImVec2(120 * scale.x, 30 * scale.y)))
           state[0] = 0;
       ImGui::Text("This is the valid input window!");
       ImGui::SetWindowPos(ImVec2(380*scale.x,20+300*scale.y), 0);
       ApplyScale("1", ImVec2(900,420), scale);
       ImGui::End();
       break;
     }
     default:
     { //both valid input and explanation windows at the same time
       
       //create the valid input window

       ImGui::Begin("1", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove); //if you use the same name it changes the already existing window
       if (ImGui::Button("Focus VI window", ImVec2(120 * scale.x, 30 * scale.y)))
           state[0] = 2;
       ImGui::Text("This is the valid input window!");
       ImGui::SetWindowPos(ImVec2(380*scale.x,20+300*scale.y), 0);
       ApplyScale("1", ImVec2(450,420), scale);
       ImGui::End();

       // create the explanation window

       ImGui::Begin("3", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
       if (ImGui::Button("Focus Ex window", ImVec2(120 * scale.x, 30 * scale.y)))
           state[0] = 1;
       ImGui::Text("This is the explanation window!");
       ImGui::SetWindowPos(ImVec2(830 * scale.x, 20 + 300 * scale.y), 0);
       ApplyScale("3", ImVec2(450, 420), scale);
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
