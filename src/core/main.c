#include "MorphEditor.h"
#include "MorphImGui.h"
#include "MorphVulkan.h"
#include "MorphTime.h"
#include "MorphPlatform.h"
#include <GLFW/glfw3.h>
#include <windows.h>
#include <shellapi.h>

#ifdef MORPH_EDITOR
static bool morphEngineRoute(const char* projectFile, const char* exeDir)
{
    FILE* f = fopen(projectFile, "rb");
    if (!f)
    {
        char msg[512];
        snprintf(msg, sizeof(msg), "Router failed to open .mproj file!\nPath: %s", projectFile);
        MessageBoxA(NULL, msg, "MorphEngine Router Error", MB_ICONERROR);
        return false;
    }
 
    char projectName[128];
    char targetEngineDir[MAX_PATH_LEN];
    fread(projectName, 1, 128, f);
    fread(targetEngineDir, 1, MAX_PATH_LEN, f);
    fclose(f);
    targetEngineDir[MAX_PATH_LEN - 1] = '\0';
 
    if (_stricmp(exeDir, targetEngineDir) == 0)
        return false;
 
    char targetExe[MAX_PATH_LEN];
    snprintf(targetExe, sizeof(targetExe), "%s\\MorphEngine.exe", targetEngineDir);
 
    char quotedArgs[MAX_PATH_LEN + 4];
    snprintf(quotedArgs, sizeof(quotedArgs), "\"%s\"", projectFile);
 
    HINSTANCE result = ShellExecuteA(NULL, "open", targetExe, quotedArgs, NULL, SW_SHOWDEFAULT);
    if ((intptr_t)result <= 32)
    {
        char msg[512];
        snprintf(msg, sizeof(msg), "Router failed to launch target engine!\nTarget: %s\nCode: %lld", targetExe, (long long)result);
        MessageBoxA(NULL, msg, "MorphEngine Router Error", MB_ICONERROR);
    }
 
    return true;
}
#endif


int main(int argc, char* argv[])
{
#ifdef MORPH_EDITOR
    char exeDir[MAX_PATH_LEN];
    GetModuleFileNameA(NULL, exeDir, MAX_PATH_LEN);
    char* lastSlash = strrchr(exeDir, '\\');
    if (lastSlash) *lastSlash = '\0';
    
    char projectFile[MAX_PATH_LEN] = {0};
    if (argc > 1)
    {
        char cleanArg[MAX_PATH_LEN];
        strncpy(cleanArg, argv[1], MAX_PATH_LEN);
        if (cleanArg[0] == '"') memmove(cleanArg, cleanArg + 1, strlen(cleanArg));
        size_t len = strlen(cleanArg);
        if (len > 0 && cleanArg[len - 1] == '"') cleanArg[len - 1] = '\0';
        GetFullPathNameA(cleanArg, MAX_PATH_LEN, projectFile, NULL);
    }
 
    SetCurrentDirectoryA(exeDir);
 
    if (argc > 1 && morphEngineRoute(projectFile, exeDir))
        return 0;
#endif

    //GLFW initialization
    if (!glfwInit())
    {
        printf("GLFW init fail\n");
        return 1;
    }

    //block OpenGL
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

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

    char associatedFlag[MAX_PATH_LEN];
    snprintf(associatedFlag, sizeof(associatedFlag), "%s\\morph.registered", exeDir);
    if (GetFileAttributesA(associatedFlag) == INVALID_FILE_ATTRIBUTES)
    {
        char fullExePath[MAX_PATH_LEN];
        snprintf(fullExePath, sizeof(fullExePath), "%s\\MorphEngine.exe", exeDir);
        morphPlatformRegisterFileAssociation(fullExePath);

        FILE* f = fopen(associatedFlag, "w");
        if (f) fclose(f);
    }

    if (argc > 1)
    {
        morphEditorOpenProject(&editor, &camera, projectFile);
    }
    else
    {
        char untitledDir[MAX_PATH_LEN];
        snprintf(untitledDir, sizeof(untitledDir), "%s\\Untitled", exeDir);
        morphPlatformRemoveDirectory(untitledDir);
 
        morphProjectCreate(&editor.project, "Untitled", exeDir, exeDir);
        editor.project.temporary = true;
        editor.showHUB = true;
 
        snprintf(editor.imguiIniPath, MAX_PATH_LEN, "%s\\Untitled\\Engine\\imgui.ini", exeDir);
        morphImGuiSetIniPath(editor.imguiIniPath);
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
        
    #ifdef MORPH_EDITOR
        morphSceneUpdateMovement(morphEditorGetActiveScene(&editor), (f32)timeState.deltaTime);
        morphEditorUpdateInput(&editor, &input, &camera, morphEditorGetActiveScene(&editor), (f32)timeState.deltaTime);

        morphImGuiNewFrame();
        morphImGuiDrawMenuBar(&editor, window, (f32)timeState.deltaTime);
        if (editor.showOutput)
        {
            morphBeginTiledWindow("Output", 5.0f, &editor, PANEL_OUTPUT);
            morphImGuiDrawOutput(&editor.output);
            morphEndTiledWindow(&editor, PANEL_OUTPUT);
        }
        if (editor.showContentDrawer)
        {
            morphBeginTiledWindow("Asset browser", 5.0f, &editor, PANEL_ASSET_BROWSER);
            morphImGuiDrawAssetBrowser(&editor);
            morphEndTiledWindow(&editor, PANEL_ASSET_BROWSER);

            morphBeginTiledWindow("Folder overwiew", 5.0f, &editor, PANEL_FOLDER_CONTENT);
            morphImGuiDrawFolderOverview(&editor);
            morphEndTiledWindow(&editor, PANEL_FOLDER_CONTENT);
        }
        if (editor.showTools)
        {
            morphBeginTiledWindow("Tools", 5.0f, &editor, PANEL_TOOLS);
            morphImGuiDrawTools();
            morphEndTiledWindow(&editor, PANEL_TOOLS);
        }
        if (editor.showOutliner)
        {
            morphBeginTiledWindow("Outliner", 5.0f, &editor, PANEL_OUTLINER);
            morphImGuiDrawOutliner(&editor);
            morphEndTiledWindow(&editor, PANEL_OUTLINER);
        }
        if (editor.showDetails)
        {
            morphBeginTiledWindow("Details", 5.0f, &editor, PANEL_DETAILS);
            morphImGuiDrawDetails(&editor);
            morphEndTiledWindow(&editor, PANEL_DETAILS);
        }
        if (editor.showViewport)
        {
            morphBeginTiledWindow("Viewport", 5.0f, &editor, PANEL_VIEWPORT);
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
            morphEndTiledWindow(&editor, PANEL_VIEWPORT);
        }
        if (editor.showHUB)
        {
            morphImGuiDrawHub(&editor, &camera);
        }
        morphVulkanDraw(&vk, window, &camera, morphEditorGetActiveScene(&editor));
        
    #else
        MorphScene runtimeScene = {0};
        morphVulkanDraw(&vk, window, &camera, &runtimeScene);
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