#pragma once

#include "MorphInput.h"
#include "MorphLog.h"
#include "MorphTypes.h"
#include "MorphMath.h"
#include "MorphCamera.h"
#include "MorphScene.h"
#include "MorphBuffer.h"
#include "MorphAssetType.h"
#include "MorphVulkan.h"

typedef struct
{
    MorphOutputConsoleBuffer output;

    //Icons
    MorphTexture assetIcons[ASSET_COUNT];
    VkDescriptorSet assetIconIds[ASSET_COUNT];

    //window visibility
    bool showOutput;
    bool showTools;
    bool showOutliner;
    bool showContentDrawer;
    bool showDetails;
    bool showViewport;

    //viewport
    Vec2 lastViewportSize;
    f32 resizeTimer;
    bool viewportNeedsResize;
    bool viewportCursorFocused;

    //outliner
    EntityHandle selectedEntity;
    bool hasSelection;
} MorphEditor;

void morphEditorInit(MorphEditor* editor, MorphVulkanContext* vk);
void morphEditorShutdown(MorphEditor* editor, MorphVulkanContext* vk);
void morphEditorUpdateInput(MorphEditor* editor, MorphInput* input, MorphCamera* editorCamera, Entities* scene, f32 deltaTime);