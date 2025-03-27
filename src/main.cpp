#ifdef _WIN32
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup") // remove empty cmd
#include <windows.h>
#include <wingdi.h>
#endif
#include <iostream>
#include <stdio.h>
#include <glad/glad.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include <GLFW/glfw3.h>
#include <imgui_impl_opengl3_loader.h>
#include "GUI_handler.h"
#include <cstring>
#include <re2/re2.h> // re2 test checks out

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

//opengl icon variables

const char *vertexShaderSource = "#version 130\n"
    "in vec3 aPos;\n"
    "uniform vec2 wobble;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x+wobble.x,aPos.y+wobble.y,aPos.z, 1.0);\n"
    "}\n";
const char *fragmentShaderSource = "#version 130\n"
    "out vec4 FragColor;\n"
    "uniform vec3 color;"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(color, 1.0f);\n"
    "}\n";

GLuint VAO, VBO, EBO, fragmentShader, vertexShader, shaderProgram, FBO, textureHolder;

float vertices[] = {
    -0.5f,  0.5f, 0.0f, // bottom left 
     0.5f,  0.5f, 0.0f, // bottom right
     0.5f, -0.5f, 0.0f, // top right
    -0.5f, -0.5f, 0.0f  // top left
}; 
GLuint indices[] = {
     0, 1, 3, //first triangle
     1, 2, 3  //second triangles
};

//end of opengl icon variables

void initShaders() {
    vertexShader = glCreateShader(GL_VERTEX_SHADER); // create shader
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader); // compile shader

    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // fragment shader
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); // create shader
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader); // compile shader
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // link shaders
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    
    glGenBuffers(1, &EBO); //generate the Element Buffer Object
    glGenVertexArrays(1, &VAO); //generate the Vertex Array Object
    glGenBuffers(1, &VBO); // generate the Vertex Buffer Object (the only one for now)

    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    
    // Generate and configure framebuffer
    glGenFramebuffers(1, &FBO);

    // Create texture to store the rendering output
    glGenTextures(1, &textureHolder); 
    
    // Check if the framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Framebuffer not complete!" << std::endl;
        return; // Return early if FBO is not complete
    }

}

GLFWwindow* InitializeGUI(ImVec2 initDisplaySize) { // Generate the main window
  const char* glsl_version = "#version 130";
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

  // Create window with graphics context
  GLFWwindow* window = glfwCreateWindow(initDisplaySize.x, initDisplaySize.y, "Regex-Debugger", nullptr, nullptr);
  if (window == nullptr)
      return NULL;
  
  glfwMakeContextCurrent(window);
  gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
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
  
  initShaders();

  return window;
}

void ApplyScale(char name[], ImVec2 initSize, ImVec2 scale) {
    ImGui::Begin(name);

    ImGui::SetWindowSize(ImVec2(initSize.x*scale.x, initSize.y*scale.y), 0); //apply scale to the specifically named window

    ImGui::End();
}

void generateIcon(ImVec2 displaySize,ImVec2 InitDisplaySize) { 
    
    /*
    This function puts a texture into a frameBuffer using a frameBufferObject(FBO), 
    
    Then it generates an empty image using textureHolder and applies the shader to it.

    After rendering it, it puts the texture into an ImGui tab using ImGui::image
    */
    
    //uniform arithmetic

    //"global" variables
    glUseProgram(shaderProgram);
    float timeValue = glfwGetTime();
    int Location;

    //vertex shader uniform arithmetic
    
    Location = glGetUniformLocation(shaderProgram, "wobble");
    glUniform2f(Location, cos(timeValue*4)/3-0.04, sin(timeValue*4)/3); //computers suck at floating point arithmetic
    
    //fragment shader uniform arithmetic
    Location = glGetUniformLocation(shaderProgram, "color");
    glUniform3f(Location, sin(timeValue) / 2.0 + 0.5, cos(timeValue) / 2.0 + 0.5, sin(timeValue) / 4.0 + cos(timeValue) / 4.0 + 0.5);

    glUseProgram(0);
    //end of uniform arithmetic

    ImVec2 scale = ImVec2(displaySize.x / InitDisplaySize.x, displaySize.y / InitDisplaySize.y);
    
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);  // Bind framebuffer
    glViewport(10, -10, 200, 200);
    glBindTexture(GL_TEXTURE_2D, textureHolder);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 200, 200, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0); // Create texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // Set texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // Attach texture to the framebuffer
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureHolder, 0);
    
    
    // if you remove the clear framebuffer code, you get a trail on the square since it never gets cleared

    // Clear the framebuffer
    glClear(GL_COLOR_BUFFER_BIT); 


    // Render to the framebuffer
    glUseProgram(shaderProgram);   // Use the shader program
    glBindVertexArray(VAO);       // Bind the vertex array object
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);  // Render the Triangle
    

    // Display the texture in ImGui
    ImGui::Begin("Icon", nullptr, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
    ImGui::SetWindowPos(ImVec2(0, 420*scale.y), 0);
    ImGui::Image(textureHolder, ImVec2(300*scale.x, 300*scale.y)); // Display the texture
    ApplyScale("Icon", ImVec2(300,300), scale);
    ImGui::End();
    
    // Unbind framebuffer (back to default framebuffer)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}


void generateWindows(GLFWwindow* window, int& displayW, int& displayH, ImVec2 initDisplaySize, int state[]) { //This is where you put secondary windows (tabs,buttons,tables,checkboxes and other windows)
   glfwGetFramebufferSize(window, &displayW, &displayH);
   ImVec2 scale = ImVec2(displayW / initDisplaySize.x, displayH / initDisplaySize.y);
   { // Create window called main and append into it
     ImGui::Begin("main",nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoBringToFrontOnFocus); 
     ImGui::SetWindowPos(ImVec2(0, 0), 0);
     ApplyScale("main",ImVec2(1280,720), scale); //here the initDisplaySize is the original size of the window
     ImGui::Text("REGEX_DBG"); // Display text (you can use format strings like printf)
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

    ImVec2 regionMin = ImVec2(-500, -500);  // Bottom-left corner
    ImVec2 regionMax = ImVec2(500, 500);  // Top-right corner
    
    // Start the ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    // Window contents are in the generateWindows() function (use Begin/End pair to create a named window)
    generateWindows(window, display_h, display_w, initDisplaySize, state);

    generateIcon(ImVec2(display_h,display_w),initDisplaySize);

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

