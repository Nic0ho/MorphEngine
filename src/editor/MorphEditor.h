#pragma once

#include "MorphInput.h"
#include "MorphLog.h"
#include "MorphProject.h"
#include "MorphTypes.h"
#include "MorphMath.h"
#include "MorphCamera.h"
#include "MorphScene.h"
#include "MorphBuffer.h"
#include "MorphAssetType.h"
#include "MorphVulkan.h"

typedef enum
{
    SELECTION_NONE,
    SELECTION_SCENE,
    SELECTION_ENTITY,
} SelectionType;

typedef struct
{
    MorphOutputConsoleBuffer output;

    //Project
    MorphProject project;

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
    SelectionType selectionType;
    EntityHandle selectedEntity;
} MorphEditor;

void morphEditorInit(MorphEditor* editor, MorphVulkanContext* vk);
void morphEditorShutdown(MorphEditor* editor, MorphVulkanContext* vk);
void morphEditorUpdateInput(MorphEditor* editor, MorphInput* input, MorphCamera* editorCamera, MorphScene* scene, f32 deltaTime);