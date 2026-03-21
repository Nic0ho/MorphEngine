cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
set(PROJECT_NAME MorphEngine)
project(${PROJECT_NAME} VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(glfw)

find_package(Vulkan REQUIRED)

add_subdirectory(Engine)
#MODULES
add_subdirectory(Modules/VulkanRenderer)
#END_MODULES
add_subdirectory(Editor)
add_subdirectory(Sandbox)
add_subdirectory(Hub)