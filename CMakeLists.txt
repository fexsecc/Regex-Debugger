cmake_minimum_required(VERSION 3.20)
project(regdebug VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================

## Find dependencies
find_package(glfw3 REQUIRED)
find_package(glad REQUIRED)
find_package(glm REQUIRED)
find_package(freetype REQUIRED)
find_package(imgui REQUIRED)
find_package(re2 REQUIRED)

# stb does not have a CMake build, but is header-only
#find_path(STB_INCLUDE_DIRS "stb.h") 

file(GLOB SRC_FILES "src/*.cpp")
## Create main executable
add_executable(regdebug ${SRC_FILES})
target_include_directories(regdebug
  PRIVATE 
    #${STB_INCLUDE_DIRS}
    "${CMAKE_CURRENT_LIST_DIR}/source"
)
target_link_libraries(
  regdebug
  PRIVATE
    freetype
    glfw
    glad::glad
    glm::glm
    imgui::imgui
    re2::re2
)

# ============================================================


target_include_directories(regdebug PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
