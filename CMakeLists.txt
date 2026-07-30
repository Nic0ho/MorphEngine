cmake_minimum_required(VERSION 3.24)
project(MorphEngine C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#Packages
#Vulkan
find_package(Vulkan REQUIRED)

#executable
add_executable(MorphEngine
    main.c
    MorphVulkan.c
    MorphBuffer.c
    stb_image_impl.c
    MorphArena.c
    MorphLog.c)
target_compile_definitions(MorphEngine PRIVATE _CRT_SECURE_NO_WARNINGS)

#directories
target_include_directories(MorphEngine PRIVATE
    #glfw
    external/glfw/include
    #stb
    external/stb
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