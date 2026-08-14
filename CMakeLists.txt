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
    #core
    src/core/main.c
    src/core/MorphCamera.c
    src/core/MorphInput.c
    src/core/MorphScene.c
    #editor
    src/editor/MorphEditor.c
    #rendering
    src/rendering/MorphAtlas.c
    src/rendering/MorphBuffer.c
    src/rendering/MorphVulkan.c
    #utils
    src/utils/MorphArena.c
    src/utils/MorphLog.c
    src/utils/MorphTime.c
    #wrappers
    src/wrappers/MorphImGui.cpp
    src/wrappers/MorphStbImage.c

    #imgui
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_vulkan.cpp)

option(MORPH_EDITOR "Build with editor" ON)

if(MORPH_EDITOR)
    target_compile_definitions(MorphEngine PRIVATE _CRT_SECURE_NO_WARNINGS MORPH_EDITOR)
endif()

#directories
target_include_directories(MorphEngine PRIVATE
    # EXTERNAL -----------
    external/glfw/include #glfw
    external/stb #stb
    external/imgui #imgui
    ${Vulkan_INCLUDE_DIRS} #Vulkan
    
    # SOURCE   -----------
    src/core      #core
    src/editor    #editor
    src/utils     #utils
    src/rendering #rendering
    src/wrappers  #wrappers
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