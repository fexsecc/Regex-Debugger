cmake_minimum_required(VERSION 3.16) # Chosen by chatgpt lol
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(regdebug VERSION 1.0)

file(GLOB_RECURSE SRC_FILES src/*.cpp)
add_executable(regdebug ${SRC_FILES})
target_include_directories(regdebug PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
