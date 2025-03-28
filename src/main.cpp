#ifdef _WIN32
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup") // remove empty cmd
#include <windows.h>
#include <wingdi.h>
#endif

//program functionality libraries
#include <iostream>
#include <stdio.h>
#include <unordered_map>
#include <cstring>
#include <string>

//graphics libraries(headers)
#include <glad/glad.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include <GLFW/glfw3.h>
#include <imgui_impl_opengl3_loader.h>
#include "GUI_handler.h"
#include <re2/re2.h> // re2 test checks out

#include <glm/glm.hpp> //opengl matrix math, the one below and the one below that one too
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

//start of opengl icon variables

const char* vertexShaderSource = "#version 130\n"
    "in vec3 aPos;\n"
    "out vec3 cPos;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "void main()\n"
    "{\n"
    "   cPos=aPos;\n"
    "   gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
    "}\n";

const char* fragmentShaderSource = "#version 130\n"
    "out vec4 FragColor;\n"
    "in vec3 cPos;\n"
    "uniform vec3 color;\n"
    "uniform vec3 lightColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4((cPos.xyz/2.0+color.xyz/2.0)*lightColor, 1.0f);\n"
    "}\n";

GLuint VAO, VBO, EBO, fragmentShader, vertexShader, shaderProgram, FBO, textureHolder;

glm::mat4 model, view, projection; // initialize identity matrix

float vertices[] = { //cube
    -0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,

    -0.5f, -0.5f,  0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,
    -0.5f, -0.5f,  0.5f,

    -0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,

     0.5f,  0.5f,  0.5f,
     0.5f,  0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,

    -0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f, -0.5f,  0.5f,
    -0.5f, -0.5f,  0.5f,
    -0.5f, -0.5f, -0.5f,

    -0.5f,  0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
     0.5f,  0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f, -0.5f
};

GLuint indices[] = { //cube triangles
     0, 1, 2, // trio of vertices that form the triangle
     3, 4, 5,
     6, 7, 8,
     9, 10, 11,
     12, 13, 14,
     15, 16, 17,
     18, 19, 20,
     21, 22, 23,
     24, 25, 26,
     27, 28, 29,
     30, 31, 32,
     33, 34, 35
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
    
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // if you remove the clear framebuffer code, you get a trail on the polygon since it never gets cleared
    glEnable(GL_DEPTH_TEST);

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

//scaling function for ImGui windows
void ApplyScale(char name[], ImVec2 initSize, ImVec2 scale) {
    ImGui::Begin(name);

    ImGui::SetWindowSize(ImVec2(initSize.x*scale.x, initSize.y*scale.y), 0); //apply scale to the specifically named window

    ImGui::End();
}

//the below function is for the Explanation Window functionality
void Explain(char regexQuery[]) {
    ImGui::Begin("explanationWindow", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);

    std::unordered_map<char, char*> regexCommandToExplanation = {
        {'$',"$ asserts position at the end of a line"},
        {'^',"^ asserts position at start of a line"},
    };

    char* p;
    char sep[] = {
        "$^"
    };
    p = strpbrk(regexQuery, sep);
    while (p != nullptr) {
        if(regexCommandToExplanation[p[0]])
            ImGui::BulletText(regexCommandToExplanation[p[0]]);
            regexCommandToExplanation.erase(p[0]);
        p = strpbrk(p+1, sep);
    }

    ImGui::End();
}

//the below function is for the Valid Input Window functionality
void generateValidInput(char regexQuery[]) {
  
}


//start of window generation functions

void generateMainWindow(ImVec2 scale) {
   ImGui::Begin("main",nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoBringToFrontOnFocus); 
   ImGui::SetWindowPos(ImVec2(0, 0), 0);
   ApplyScale("main",ImVec2(1280,720), scale); //here the initDisplaySize is the original size of the window
   ImGui::Text("REGEX_DBG"); // Display text (you can use format strings like printf)
   ImGui::End();
}

void generateFocusedExplanationWindow(ImVec2 scale, int state[], char regexQuery[]) {
   ImGui::Begin("explanationWindow", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
   if (ImGui::Button("Go back", ImVec2(120 * scale.x, 30 * scale.y)))
     state[0] = 0;
   ImGui::Text("This is the explanation window!");
   Explain(regexQuery);
   ImGui::SetWindowPos(ImVec2(380 * scale.x, 20 + 300 * scale.y), 0);
   ApplyScale("explanationWindow", ImVec2(900, 420), scale);
   ImGui::End();
}

void generateFocusedValidInputWindow(ImVec2 scale, int state[], char regexQuery[]) {
   ImGui::Begin("1", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove); //if you use the same name it changes the already existing window
   if (ImGui::Button("Go back", ImVec2(120 * scale.x, 30 * scale.y)))
     state[0] = 0;
   ImGui::Text("This is the valid input window!");
   ImGui::SetWindowPos(ImVec2(380*scale.x,20+300*scale.y), 0);
   ApplyScale("1", ImVec2(900,420), scale);
   ImGui::End();
}

void generateBothVIandEWindows(ImVec2 scale, int state[], char regexQuery[]) {
   
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

}

//The below function generates the above windows
void generateWindows(GLFWwindow* window, int& displayW, int& displayH, ImVec2 initDisplaySize, int state[]) { //This is where you put secondary windows (tabs,buttons,tables,checkboxes and other windows)
   glfwGetFramebufferSize(window, &displayW, &displayH);
   ImVec2 scale = ImVec2(displayW / initDisplaySize.x, displayH / initDisplaySize.y);
    
   generateMainWindow(scale); // generate main window and append into it
    
   static char buf[10000] = "Example: ^([0-9])\\1{3}$"; //this is the Regex Input tab
   {
      ImGui::Begin("Input Window", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration);
      ImGui::InputText("<--regexInput", buf, IM_ARRAYSIZE(buf));
      ImGui::TextWrapped(buf);
      ImGui::SetWindowPos(ImVec2(380 * scale.x, 20), 0);
      ApplyScale("Input Window", ImVec2(900, 300), scale);
      ImGui::End();
   }
   switch (state[0]) {
     case 1:
     { //focused explanation window
       generateFocusedExplanationWindow(scale, state, buf);
       break;
     }
     case 2:
     { //focused valid input window
       generateFocusedValidInputWindow(scale, state, buf);
       break;
     }
     default:
     { //both valid input and explanation windows at the same time
       generateBothVIandEWindows(scale, state, buf);
       break;
     }
   }
}

//end of window generation functions


void initTextureAndViewport() {
    
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);  // Bind framebuffer

    // Clear the framebuffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(10, -10, 250, 250);
    glBindTexture(GL_TEXTURE_2D, textureHolder);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 250, 250, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0); // Create texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // Set texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // Attach texture to the framebuffer
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureHolder, 0);
    
}

void performShaderUniformArithmetic() {
    //uniform arithmetic

    //"global" variables
    glUseProgram(shaderProgram);
    float timeValue = glfwGetTime();
    int Location;
    model = glm::mat4(1.0f);
    view = glm::mat4(1.0f);
    projection = glm::mat4(1.0f);
    //vertex shader uniform arithmetic

    Location = glGetUniformLocation(shaderProgram, "model"); //using a model matrix to transform the vertices (default transforms)
    model = glm::rotate(model, glm::radians(65.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f)); //scale setter
    model = glm::translate(model, glm::vec3(sin(timeValue*0.5f) * 5.0f, -cos(timeValue*0.5f) * 5.0f, 0.0f));
    model = glm::rotate(model, glm::radians(timeValue*300), glm::vec3(1.0f, 1.0f, 1.0f));
    glUniformMatrix4fv(Location, 1, GL_FALSE, glm::value_ptr(model));

    Location = glGetUniformLocation(shaderProgram, "view"); // view transform sets the camera
    GLfloat camerax = cos(timeValue) * 2;
    GLfloat cameraz = sin(timeValue) * 2;
    view = glm::lookAt(glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f,1.0f,0.0f)); // creates a "camera" (a separate xyz coordinate system that simulates a camera)
    glUniformMatrix4fv(Location, 1, GL_FALSE, glm::value_ptr(view));

    Location = glGetUniformLocation(shaderProgram, "projection"); //projection matrix sets the view mode to "perspective" (so things look smaller further away)
    projection = glm::perspective(glm::radians(45.0f), 200.0f / 200.0f, 0.1f, 100.0f);
    glUniformMatrix4fv(Location, 1, GL_FALSE, glm::value_ptr(projection));

    //fragment shader uniform arithmetic
    Location = glGetUniformLocation(shaderProgram, "color");
    glUniform3f(Location, sin(timeValue) / 2.0 + 0.5, cos(timeValue) / 2.0 + 0.5, sin(timeValue) / 4.0 + cos(timeValue) / 4.0 + 0.5);
    Location = glGetUniformLocation(shaderProgram, "lightColor");
    glUniform3f(Location, 1.0f, 1.0f, 1.0f); //diffuse light

    glUseProgram(0);

    //end of uniform arithmetic

}

void drawAndDisplayTexture(ImVec2 scale) {
    
    // Render to the framebuffer
    glUseProgram(shaderProgram);   // Use the shader program
    glBindVertexArray(VAO);       // Bind the vertex array object

    glDrawElements(GL_TRIANGLES, 120, GL_UNSIGNED_INT, 0);  // Render the Triangle
    
    // Display the texture in ImGui
    ImGui::Begin("Icon", nullptr, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
    ImGui::SetWindowPos(ImVec2(0, 420*scale.y), 0);
    ImGui::Image(textureHolder, ImVec2(300*scale.x, 300*scale.y)); // Display the texture
    ApplyScale("Icon", ImVec2(300,300), scale);
    ImGui::End();
    
}

//the function below generates the floating 3d model using the above three functions
void generateIcon(ImVec2 displaySize,ImVec2 InitDisplaySize) {
    
    /*
    This function puts a texture into a frameBuffer using a frameBufferObject(FBO), 
    
    Then it generates an empty image using textureHolder and applies the shader to it.

    After rendering and drawing the texture, this function puts the texture into an ImGui tab using ImGui::Image
    */

    ImVec2 scale = ImVec2(displaySize.x / InitDisplaySize.x, displaySize.y / InitDisplaySize.y);
    
    initTextureAndViewport(); 
    
    performShaderUniformArithmetic();

    drawAndDisplayTexture(scale);

    // Unbind framebuffer (back to default framebuffer)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

//render loop(core function of the whole GUI)
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

