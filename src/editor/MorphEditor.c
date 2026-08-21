#include "MorphEditor.h"
#include "GLFW/glfw3.h"
#include "MorphBuffer.h"
#include "MorphImGui.h"
#include "MorphInput.h"
#include "MorphLog.h"

static const char* iconPaths[ASSET_COUNT] =
{
    "assets/folder.png",
    "assets/texture.png",
    "assets/scene.png",
    "assets/entity.png",
    "assets/file.png", 
};


void morphEditorInit(MorphEditor* editor, MorphVulkanContext* vk)
{
    

    morphProjectCreate(&editor->project, "TestGame", "C:\\Users\\artem\\Documents"); 
    morphLogSetOutput(&editor->output);
    editor->showOutput = true;
    editor->showOutliner = true;
    editor->showDetails = true;
    editor->showTools = true;
    editor->showContentDrawer = true;
    editor->showViewport = true;

    vk->viewportDescriptorSet = morphImGuiRegisterTexture(vk->viewportTexture.sampler, vk->viewportTexture.view);
    for (u32 i = 0; i < ASSET_COUNT; i++)
    {
        if (morphTextureLoad(vk->logicalDevice, vk->physicalDevice, vk->commandPool, vk->graphicsQueue, iconPaths[i], &editor->assetIcons[i]))
            editor->assetIconIds[i] = morphImGuiRegisterTexture(editor->assetIcons[i].sampler, editor->assetIcons[i].view);
        else
            morphLog(LOG_ERROR, "Failed to load icon: %s", iconPaths[i]);
    }
}

void morphEditorShutdown(MorphEditor* editor, MorphVulkanContext* vk)
{
    for (u32 i = 0; i < ASSET_COUNT; i++)
        morphTextureDestroy(vk->logicalDevice, &editor->assetIcons[i]);
}

void morphEditorUpdateInput(MorphEditor* editor, MorphInput* input, MorphCamera* editorCamera, Entities* scene, f32 deltaTime)
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
}