#pragma once
#ifdef _WIN32
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup") // remove empty cmd
#include <windows.h>
#include <wingdi.h>
#endif

//program functionality libraries
#include <iostream>
#include <stdio.h>
#include <unordered_map>
#include <map>
#include <cstring>
#include <string>
#include <vector>

//graphics libraries(headers)
#include <glad/glad.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include <GLFW/glfw3.h>
#include <imgui_impl_opengl3_loader.h>
#include <re2/re2.h>

#include <glm/glm.hpp> //opengl matrix math, the one below and the one below that one too
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

constexpr auto X_RES = 1280.0;
constexpr auto Y_RES = 720.0;
std::string const fontRelPath = "/fonts/JetBrainsMono-Regular.ttf";


void glfw_error_callback(int error, const char* description);
void initShaders();
GLFWwindow* InitializeGUI(ImVec2 initDisplaySize);
void ApplyScale(char name[], ImVec2 initSize, ImVec2 scale);
void Explain(char regexQuery[]);
void generateMainWindow(ImVec2 scale);
void generateFocusedExplanationWindow(ImVec2 scale, int state[], char regexQuery[]);
void generateFocusedValidInputWindow(ImVec2 scale, int state[], char regexQuery[]);
void generateBothVIandEWindows(ImVec2 scale, int state[], char regexQuery[]);
void generateWindows(GLFWwindow* window, int& displayW, int& displayH, ImVec2 initDisplaySize, int state[]);
void initTextureAndViewport();
void performShaderUniformArithmetic();
void drawAndDisplayTexture(ImVec2 scale);
void generateIcon(ImVec2 displaySize, ImVec2 InitDisplaySize);
void RenderGUI(GLFWwindow* window, ImVec2 initDisplaySize);
