cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
set(PROJECT_NAME MorphEngine)
project(${PROJECT_NAME} VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(glfw)

find_package(Vulkan REQUIRED)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/VulkanCore/src/Vulkan/core.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    glfw 
    Vulkan::Vulkan
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)