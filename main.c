#include <stdio.h>

#include "MorphLog.h"
#include "MorphVulkan.h"
#include "MorphInput.h"
#include "MorphCamera.h"
#include "MorphTime.h"

#include <GLFW/glfw3.h>


int main(void)
{
    //GLFW initialization
    if (!glfwInit())
    {
        printf("GLFW init fail\n");
        return 1;
    }

    //block OpenGL
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    //window creation
    //                   win create func |   res     |   win name   |     ?      |
    GLFWwindow* window = glfwCreateWindow(1920, 1080, "MorphEngine", NULL, NULL);
    if (!window)
    {
        printf("Window creation fail\n");
        glfwTerminate();
        return 1;
    }

    //Initials
    MorphInput input = {0};      //Input
    MorphTime timeState = {0};   //Time
    MorphCamera camera ={0};     // Camera
    camera.viewWidth = 5.0f;

    //Vulkan
    MorphVulkanContext vk = {0};
    if (!morphVulkanInit(&vk, window))
    {
        printf("[VULKAN ERROR] Vulkan init fail\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    //main loop
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        morphTimeUpdate(&timeState);

        morphInputUpdate(&input, window);
        
        if (morphInputIsKeyDown(&input, GLFW_KEY_D))
            camera.position.x += (f32)timeState.deltaTime * 3.0f;
        if (morphInputIsKeyDown(&input, GLFW_KEY_A))
            camera.position.x -= (f32)timeState.deltaTime * 3.0f;
        if (morphInputIsKeyDown(&input, GLFW_KEY_W))
            camera.position.y += (f32)timeState.deltaTime * 3.0f;
        if (morphInputIsKeyDown(&input, GLFW_KEY_S))
            camera.position.y -= (f32)timeState.deltaTime * 3.0f;
        if (morphInputIsKeyPressed(&input, GLFW_KEY_UP))
            camera.viewWidth += 0.25f;
        if (morphInputIsKeyPressed(&input, GLFW_KEY_DOWN))
            camera.viewWidth -= 0.25f;

        morphVulkanDraw(&vk, window, &camera);
    }

    //shutdown
    morphVulkanShutdown(&vk);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}