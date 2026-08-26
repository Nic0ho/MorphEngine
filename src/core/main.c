#include "MorphBuffer.h"
#include "MorphImGui.h"
#include "MorphLog.h"
#include "MorphProject.h"
#include "MorphScene.h"
#include "MorphTypes.h"
#include "MorphVulkan.h"
#include "MorphInput.h"
#include "MorphCamera.h"
#include "MorphTime.h"
#include "MorphEditor.h"
#include "MorphPlatform.h"
#include <GLFW/glfw3.h>
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "stb_image.h"

int main(int argc, char* argv[])
{
#ifdef MORPH_EDITOR
    char exeDir[MAX_PATH_LEN];
    GetModuleFileNameA(NULL, exeDir, MAX_PATH_LEN);
    
    char* lastSlash = strrchr(exeDir, '\\');
    if (lastSlash) *lastSlash = '\0';

    SetCurrentDirectoryA(exeDir);
#endif

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

    //Vulkan
    MorphVulkanContext vk = {0};
    if (!morphVulkanInit(&vk, window))
    {
        printf("[VULKAN ERROR] Vulkan init fail\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    // Camera
    MorphCamera camera = {0};
    camera.viewWidth = 5.0f;
    camera.zoomStrength = 0.25f;
    camera.sensitivity = 0.015f;

    // Scene
    MorphScene scene = {0};

#ifdef MORPH_EDITOR
    //ImGui
    if (!morphImGuiInit(&vk, window))
    {
        morphLog(LOG_ERROR, "ImGui init fail!");
        return 1;
    }
    morphLog(LOG_MESSAGE, "ImGui Loaded");

    //Editor
    MorphEditor editor = {0};
    Vec2 viewportSize = {0};

    morphEditorInit(&editor, &vk, exeDir);

    char associated[MAX_PATH_LEN];
    snprintf(associated, sizeof(associated), "%s\\morph.registered", exeDir);
    if (GetFileAttributesA(associated) == INVALID_FILE_ATTRIBUTES)
    {
        char fullExePath[MAX_PATH_LEN];
        snprintf(fullExePath, sizeof(fullExePath), "%s\\MorphEngine.exe", exeDir);
        morphPlatformRegisterFileAssociation(fullExePath);

        //create flag file
        FILE* f = fopen(associated, "w");
        if (f) fclose(f);
    }

    if (argc > 1)
    {
        morphEditorOpenProject(&editor, &camera, &scene, argv[1]);
    }
    else
    {
        char untitledDir[MAX_PATH_LEN];
        snprintf(untitledDir, sizeof(untitledDir), "%s\\Untitled", exeDir);
        morphPlatformRemoveDirectory(untitledDir);

        morphProjectCreate(&editor.project,"Untitled", exeDir, exeDir);
        editor.project.temporary = true;
        editor.showHUB = true;
    }
    
    morphLog(LOG_MESSAGE, "Editor initialized");
#endif

    //Time
    MorphTime timeState = {0};

    //Input
    MorphInput input = {0};
    glfwSetWindowUserPointer(window, &input);
    glfwSetScrollCallback(window, morphScrollCallback);

    //main loop
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        morphTimeUpdate(&timeState);

        morphInputUpdate(&input, window);

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
        if (editor.showHUB)
        {
            morphImGuiDrawHub(&editor, &camera, &scene);
        }
        morphVulkanDraw(&vk, window, &camera, &scene);
        
    #else
        morphVulkanDraw(&vk, window, &camera, &scene);
    #endif
    }

    //shutdown
#ifdef MORPH_EDITOR
    morphProjectShutdown(&editor.project);
    morphImGuiShutdown(&vk);
    morphEditorShutdown(&editor, &vk);
#endif
    morphVulkanShutdown(&vk);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}