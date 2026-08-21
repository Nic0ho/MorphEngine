#include <stdio.h>

#include "MorphAtlas.h"
#include "MorphBuffer.h"
#include "MorphImGui.h"
#include "MorphLog.h"
#include "MorphScene.h"
#include "MorphVulkan.h"
#include "MorphInput.h"
#include "MorphCamera.h"
#include "MorphTime.h"
#include "MorphEditor.h"

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

#ifdef MORPH_EDITOR
    //Editor
    MorphEditor editor = {0};
    Vec2 viewportSize = {0};
    morphLog(LOG_MESSAGE, "Editor initialized");
#endif

    //Time
    MorphTime timeState = {0};

    //Input
    MorphInput input = {0};
    glfwSetWindowUserPointer(window, &input);
    glfwSetScrollCallback(window, morphScrollCallback);

    // Camera
    MorphCamera camera = {0};
    camera.viewWidth = 5.0f;
    camera.zoomStrength = 0.25f;
    camera.sensitivity = 0.015f;

    //Vulkan
    MorphVulkanContext vk = {0};
    if (!morphVulkanInit(&vk, window))
    {
        printf("[VULKAN ERROR] Vulkan init fail\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    //ImGui
    if (!morphImGuiInit(&vk, window))
    {
        morphLog(LOG_ERROR, "ImGui init fail!");
        return 1;
    }
#ifdef MORPH_EDITOR
    morphEditorInit(&editor, &vk);
#endif
    morphLog(LOG_MESSAGE, "ImGui Loaded");

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
        
        scene.velocity[player.index] = (Vec2){0}; 

        morphEditorUpdateInput(&editor, &input, &camera, &scene, (f32)timeState.deltaTime);

        morphSceneUpdateMovement(&scene, (f32)timeState.deltaTime);
        
    #ifdef MORPH_EDITOR
        morphImGuiNewFrame();
        morphImGuiBeginDockspace();
        morphImGuiDrawMenuBar(&editor, (f32)timeState.deltaTime);
        if (editor.showOutput)
        {
            morphImGuiBeginWindow("Output");
            morphImGuiDrawOutput(&editor.output);
            morphImGuiEndWindow();
        }
        if (editor.showContentDrawer)
        {
            morphImGuiBeginWindow("Asset browser");
            morphImGuiDrawAssetBrowser(&editor);
            morphImGuiEndWindow();
            morphImGuiBeginWindow("Folder overwiew");
            morphImGuiDrawFolderOverview(&editor);
            morphImGuiEndWindow();
        }
        if (editor.showTools)
        {
            morphImGuiBeginWindow("Tools");
            morphImGuiDrawTools();
            morphImGuiEndWindow();
        }
        if (editor.showOutliner)
        {
            morphImGuiBeginWindow("Outliner");
            morphImGuiDrawOutliner(&scene, &editor);
            morphImGuiEndWindow();
        }
        if (editor.showDetails)
        {
            morphImGuiBeginWindow("Details");
            morphImGuiDrawDetails(&scene, &editor);
            morphImGuiEndWindow();
        }
        if (editor.showViewport)
        {
            morphImGuiBeginWindow("Viewport");
            viewportSize = morphImGuiGetViewportSize();

            if (viewportSize.x != editor.lastViewportSize.x || viewportSize.y != editor.lastViewportSize.y)
            {
                editor.lastViewportSize = viewportSize;
                editor.resizeTimer = 0.0f;
                editor.viewportNeedsResize = true;
            }
            if (editor.viewportNeedsResize)
            {
                editor.resizeTimer += (f32)timeState.deltaTime;
                if (editor.resizeTimer > 0.05f && viewportSize.x > 0 && viewportSize.y > 0)
                {
                    morphVulkanResizeViewport(&vk, (u32)viewportSize.x, (u32)viewportSize.y);
                    vk.viewportDescriptorSet = morphImGuiRegisterTexture(vk.viewportTexture.sampler, vk.viewportTexture.view);
                    editor.viewportNeedsResize = false;
                    editor.resizeTimer = 0.0f;
                }
            }
            
            morphImGuiDrawViewport(vk.viewportDescriptorSet, vk.viewportTexture.width, vk.viewportTexture.height);
            editor.viewportCursorFocused = morphImGuiGetViewportFocusedCursor();

            morphImGuiEndWindow();
        }
        morphVulkanDraw(&vk, window, &camera, &scene);
        
    #else
        morphVulkanDraw(&vk, window, &camera, &scene);
    #endif
    }

    //shutdown
#ifdef MORPH_EDITOR
    morphImGuiShutdown(&vk);
    morphEditorShutdown(&editor, &vk);
#endif
    morphVulkanShutdown(&vk);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}