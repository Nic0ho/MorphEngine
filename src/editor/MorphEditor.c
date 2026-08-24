#include "MorphEditor.h"
#include "GLFW/glfw3.h"
#include "MorphBuffer.h"
#include "MorphImGui.h"
#include "MorphInput.h"
#include "MorphLog.h"
#include "MorphTypes.h"
#include <string.h>
#include <stdio.h>

static const char* iconPaths[ASSET_COUNT] =
{
    "assets/folder.png",
    "assets/texture.png",
    "assets/scene.png",
    "assets/entity.png",
    "assets/file.png", 
};

void morphEditorLoadRecent(MorphEditor* editor)
{
    char recentPath[MAX_PATH_LEN];
    snprintf(recentPath, sizeof(recentPath), "%s\\recent.dat", editor->exeDir);

    FILE* file = fopen(recentPath, "r");
    if (!file) return;

    editor->recentCount = 0;
    while (editor->recentCount < MAX_RECENT)
    {
        if (!fgets(editor->recentProjects[editor->recentCount], MAX_PATH_LEN, file))
            break;
        char* newLine = strchr(editor->recentProjects[editor->recentCount], '\n');
        if (newLine) *newLine = '\0';
        editor->recentCount++;
    }

    fclose(file);
}

void morphEditorSaveRecent(MorphEditor* editor)
{
    char recentPath[MAX_PATH_LEN];
    snprintf(recentPath, sizeof(recentPath), "%s\\recent.dat", editor->exeDir);

    FILE* file = fopen(recentPath, "w");
    if (!file) return;

    for (u8 i = 0; i < editor->recentCount; i++)
        fprintf(file, "%s\n", editor->recentProjects[i]);

    fclose(file);
}

void morphEditorAddRecent(MorphEditor* editor, const char* projectPath)
{
    int existingIndex = -1;

    for (u8 i = 0; i < editor->recentCount; i++)
    {
        if  (strcmp(editor->recentProjects[i], projectPath) == 0)
        {
            existingIndex = i;
            break;
        }
    }

    if (existingIndex != -1)
        memmove(&editor->recentProjects[1], &editor->recentProjects[0], existingIndex * MAX_PATH_LEN);
    else
    {
        if (editor->recentCount < MAX_RECENT) editor->recentCount++;
        memmove(&editor->recentProjects[1], &editor->recentProjects[0], (MAX_RECENT - 1) * MAX_PATH_LEN);
    }

    strncpy(editor->recentProjects[0], projectPath, MAX_PATH_LEN);

    morphEditorSaveRecent(editor);
}

void morphEditorRemoveRecent(MorphEditor* editor, const char* projectPath)
{
    int existingIndex = -1;

    for (u8 i = 0; i < editor->recentCount; i++)
    {
        if  (strcmp(editor->recentProjects[i], projectPath) == 0)
        {
            existingIndex = i;
            break;
        }
    }

    if (existingIndex != -1)
    {
        memmove(&editor->recentProjects[existingIndex], &editor->recentProjects[existingIndex + 1], (editor->recentCount - existingIndex - 1) * MAX_PATH_LEN);
        editor->recentCount--;
    }

    morphEditorSaveRecent(editor);
}

void morphEditorInit(MorphEditor* editor, MorphVulkanContext* vk, const char* exeDir)
{
    morphLogSetOutput(&editor->output);

    vk->viewportDescriptorSet = morphImGuiRegisterTexture(vk->viewportTexture.sampler, vk->viewportTexture.view);
    for (u32 i = 0; i < ASSET_COUNT; i++)
    {
        if (morphTextureLoad(vk->logicalDevice, vk->physicalDevice, vk->commandPool, vk->graphicsQueue, iconPaths[i], &editor->assetIcons[i]))
            editor->assetIconIds[i] = morphImGuiRegisterTexture(editor->assetIcons[i].sampler, editor->assetIcons[i].view);
        else
            morphLog(LOG_ERROR, "Failed to load icon: %s", iconPaths[i]);
    }

    strncpy(editor->exeDir, exeDir, MAX_PATH_LEN);
    morphEditorLoadRecent(editor);
}

void morphEditorShutdown(MorphEditor* editor, MorphVulkanContext* vk)
{
    for (u32 i = 0; i < ASSET_COUNT; i++)
        morphTextureDestroy(vk->logicalDevice, &editor->assetIcons[i]);
}

void morphEditorUpdateInput(MorphEditor* editor, MorphInput* input, MorphCamera* editorCamera, MorphScene* scene, f32 deltaTime)
{
    // BINDINGS --------------------
    //file
    if (morphInputIsKeyDown(input, GLFW_KEY_LEFT_CONTROL) && morphInputIsKeyPressed(input, GLFW_KEY_S))
    {
        morphSceneSave(scene, "test_level.mrph");
    }

    //viewport
    if (editor->viewportCursorFocused)
    {
        //camera movement

        if (morphInputIsMouseButtonDown(input, GLFW_MOUSE_BUTTON_MIDDLE))
        {
            if (editor->lastViewportSize.x > 0.0f) 
            {
                f32 worldUnitsPerPixel = editorCamera->viewWidth / editor->lastViewportSize.x;

                editorCamera->position.x -= (f32)input->mouseDeltaX * worldUnitsPerPixel;
                editorCamera->position.y += (f32)input->mouseDeltaY * worldUnitsPerPixel; 
            }
        }

        if (input->scrollDelta != 0.0f)
        {
            editorCamera->viewWidth -= input->scrollDelta * editorCamera->zoomStrength;
            if (editorCamera->viewWidth < 0.5f)
                editorCamera->viewWidth = 0.5f;
        }
    }

    input->scrollDelta = 0;

    //outliner
    if (morphInputIsKeyPressed(input, GLFW_KEY_DELETE) && editor->selectionType == SELECTION_ENTITY)
    {
        morphSceneRemoveEntity(scene, editor->selectedEntity);
        editor->selectionType = SELECTION_NONE;
    }
}