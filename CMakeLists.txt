cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
set(PROJECT_NAME MorphEngine)
project(${PROJECT_NAME} VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

include(FetchContent)

# GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1)

FetchContent_MakeAvailable(glfw glm)

# Assimp
FetchContent_Declare(
        assimp
        GIT_REPOSITORY https://github.com/assimp/assimp.git
        GIT_TAG v5.4.1)

set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INJECT_DEBUG_POSTFIX OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(assimp)

find_package(Vulkan REQUIRED)

add_subdirectory(Engine)
#MODULES
add_subdirectory(Modules/VulkanRenderer)
#END_MODULES
#add_subdirectory(Editor)
add_subdirectory(Sandbox)
#add_subdirectory(Hub)