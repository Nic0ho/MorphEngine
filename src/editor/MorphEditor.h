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

#ifdef __cplusplus
extern "C"
{
#endif

#define MAX_RECENT 10

typedef enum
{
    SELECTION_NONE,
    SELECTION_SCENE,
    SELECTION_ENTITY,
} SelectionType;

typedef struct
{
    MorphOutputConsoleBuffer output;

    //Engine
    char exeDir[MAX_PATH_LEN];
    char imguiIniPath[MAX_PATH_LEN];

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
    bool showHUB;

    //viewport
    Vec2 lastViewportSize;
    f32 resizeTimer;
    bool viewportNeedsResize;
    bool viewportCursorFocused;

    //outliner
    SelectionType selectionType;
    EntityHandle selectedEntity;

    //recent projects
    char recentProjects[MAX_RECENT][MAX_PATH_LEN];
    u32 recentCount;
} MorphEditor;

void morphEditorInit(MorphEditor* editor, MorphVulkanContext* vk, const char* exeDir);
void morphEditorShutdown(MorphEditor* editor, MorphVulkanContext* vk);
void morphEditorOpenProject(MorphEditor* editor, MorphCamera* camera, MorphScene* scene, const char* projectTL);
void morphEditorUpdateInput(MorphEditor* editor, MorphInput* input, MorphCamera* editorCamera, MorphScene* scene, f32 deltaTime);
void morphEditorLoadRecent(MorphEditor* editor);
void morphEditorSaveRecent(MorphEditor* editor);
void morphEditorAddRecent(MorphEditor* editor, const char* projectPath);
void morphEditorRemoveRecent(MorphEditor* editor, const char* projectPath);

#ifdef __cplusplus
}
#endif