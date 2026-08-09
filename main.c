#include <stdio.h>

#include "MorphAtlas.h"
#include "MorphLog.h"
#include "MorphScene.h"
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

    //INITIALS
    //Time
    MorphTime timeState = {0};

    //Input
    MorphInput input = {0};
    glfwSetWindowUserPointer(window, &input);
    glfwSetScrollCallback(window, morphScrollCallback);

    // Camera
    MorphCamera camera ={0};
    camera.viewWidth = 5.0f;
    camera.zoomStrength = 0.25f;
    camera.speed = 3.0f;

    //Vulkan
    MorphVulkanContext vk = {0};
    if (!morphVulkanInit(&vk, window))
    {
        printf("[VULKAN ERROR] Vulkan init fail\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    // Scene
    Entities scene = {0};
    
    EntityHandle player = morphSceneSpawnEntity(&scene, ENTITY_PLAYER, (Vec2){1.0f, 0.0f}, (Vec2){1.0f, 1.0f});
    EntityHandle block = morphSceneSpawnEntity(&scene, ENTITY_BLOCK, (Vec2){0.0f, 0.0f}, (Vec2){1.0f, 1.0f});
    //sprites build
    morphAtlasAddSprite(&vk.atlas, "assets/player.png");
    morphAtlasAddSprite(&vk.atlas, "assets/block.png");
    morphAtlasBuild(&vk.atlas, vk.logicalDevice, vk.physicalDevice, vk.commandPool, vk.graphicsQueue);

    scene.spriteID[player.index] = 0;
    scene.spriteID[block.index] = 1;

    //main loop
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        morphTimeUpdate(&timeState);

        morphInputUpdate(&input, window);
        
        if (morphInputIsKeyDown(&input, GLFW_KEY_D))
            camera.position.x += (f32)timeState.deltaTime * camera.speed;
        if (morphInputIsKeyDown(&input, GLFW_KEY_A))
            camera.position.x -= (f32)timeState.deltaTime * camera.speed;
        if (morphInputIsKeyDown(&input, GLFW_KEY_W))
            camera.position.y += (f32)timeState.deltaTime * camera.speed;
        if (morphInputIsKeyDown(&input, GLFW_KEY_S))
            camera.position.y -= (f32)timeState.deltaTime * camera.speed;
        camera.viewWidth -= input.scrollDelta * camera.zoomStrength;
        if (camera.viewWidth < 0.5f)
            camera.viewWidth = 0.5f;
        input.scrollDelta = 0;
        if (morphInputIsKeyPressed(&input, GLFW_KEY_UP))
        {
            camera.zoomStrength += 0.05f;
            morphLog(LOG_MESSAGE, "Zoom strength is now %f", camera.zoomStrength);
        }
        if (morphInputIsKeyPressed(&input, GLFW_KEY_DOWN))
        {
            camera.zoomStrength -= 0.05f;
            morphLog(LOG_MESSAGE, "Zoom strength is now %f", camera.zoomStrength);
        }
        if (morphInputIsKeyPressed(&input, GLFW_KEY_LEFT))
        {
            camera.speed += 0.5f;
            morphLog(LOG_MESSAGE, "Camera speed is now %f", camera.speed);
        }
        if (morphInputIsKeyPressed(&input, GLFW_KEY_RIGHT))
        {
            camera.speed -= 0.5f;
            morphLog(LOG_MESSAGE, "Camera speed is now %f", camera.speed);
        }

        morphVulkanDraw(&vk, window, &camera, &scene);
    }

    //shutdown
    morphVulkanShutdown(&vk);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}