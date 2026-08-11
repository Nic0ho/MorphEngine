cmake_minimum_required(VERSION 3.24)
project(MorphEngine C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#Packages
#Vulkan
find_package(Vulkan REQUIRED)

#executable
add_executable(MorphEngine
    #engine core
    main.c
    MorphVulkan.c
    MorphBuffer.c
    stb_image_impl.c
    MorphImGui.cpp
    MorphArena.c
    MorphLog.c
    MorphInput.c
    MorphCamera.c
    MorphTime.c
    MorphScene.c
    MorphAtlas.c

    #imgui
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_vulkan.cpp)

target_compile_definitions(MorphEngine PRIVATE 
    _CRT_SECURE_NO_WARNINGS
    MORPH_EDITOR
)

#directories
target_include_directories(MorphEngine PRIVATE
    #glfw
    external/glfw/include
    #stb
    external/stb
    #imgui
    external/imgui
    #Vulkan
    ${Vulkan_INCLUDE_DIRS}
)

#libraries
target_link_libraries(MorphEngine PRIVATE
    #GLFW
    ${CMAKE_SOURCE_DIR}/external/glfw/lib/glfw3.lib
    #Vulkan
    Vulkan::Vulkan
    #Windows specifies
    gdi32
    user32
    shell32
)