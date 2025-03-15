#ifdef _WIN32
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

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

int main() {
  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit())
    return 1;

  const char* glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  
  int initW = 1280, initH = 720;
  // Create window with graphics context
  GLFWwindow* window = glfwCreateWindow(initW, initH, "Regex-Debugger", nullptr, nullptr);
  if (window == nullptr)
      return 1;
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
  ImVec4 clear_color = ImVec4(0.1f, 0.1f, 0.1f, 1.00f);
  
  bool showCoolButton = true;

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

    // Start the ImGui demo frame
    //if (show_demo_window)
    //ImGui::ShowDemoWindow(&show_demo_window);
    

    // Window contents (use Begin/End pair to create a named window)
    if (showCoolButton)
    {
        ImGui::Begin("cool button holder", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize);
        if (ImGui::Button("Cool Button"))
            showCoolButton = false;
        ImGui::End();
    }
    glfwGetFramebufferSize(window, &initW, &initH);
    {
        ImGui::Begin("Hello world",nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground); // Create window called hello world and append into it
        ImGui::SetWindowPos(ImVec2(0, 0), 0);
        ImGui::SetWindowSize(ImVec2(initW, initH), 0);
        ImGui::Text("yo how we doin"); // Display text (you can use format strings like printf)
        ImGui::End();
    }


    ImGui::Render();
    int display_w, display_h;
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

  return 0;
}
