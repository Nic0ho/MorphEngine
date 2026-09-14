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
#define MAX_TABS  16

typedef enum
{
    SELECTION_NONE,
    SELECTION_SCENE,
    SELECTION_ENTITY,
} SelectionType;

typedef enum
{
    TAB_SCENE,
    TAB_TEXTURE,
    TAB_MATERIAL,
    TAB_SPRITE,
    TAB_PREFAB,
} MorphTabType;

typedef enum
{
    PANEL_TOOLS,
    PANEL_VIEWPORT,
    PANEL_OUTLINER,
    PANEL_DETAILS,
    PANEL_ASSET_BROWSER,
    PANEL_FOLDER_CONTENT,
    PANEL_OUTPUT,
    PANEL_COUNT
} MorphPanelId;

typedef union
{
    MorphScene scene;
} MorphTabContent;

typedef struct
{
    MorphTabType type;
    MorphTabContent content;
    char filepath[MAX_PATH_LEN];
    char title[64];
    bool hasPath;
    bool dirty;
    bool open;
} MorphEditorTab;

typedef struct
{
    bool isInBlock;
    Vec2 blockMin;
    Vec2 blockMax;
} MorphPanelState;

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

    //Tabs
    MorphEditorTab tabs[MAX_TABS];
    MorphPanelState panelStates[PANEL_COUNT];
    u32 tabCount;
    u32 activeTab;

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
void morphEditorOpenProject(MorphEditor* editor, MorphCamera* camera, const char* projectTL);
void morphEditorUpdateInput(MorphEditor* editor, MorphInput* input, MorphCamera* editorCamera, MorphScene* scene, f32 deltaTime);
void morphEditorLoadRecent(MorphEditor* editor);
void morphEditorSaveRecent(MorphEditor* editor);
void morphEditorAddRecent(MorphEditor* editor, const char* projectPath);
void morphEditorRemoveRecent(MorphEditor* editor, const char* projectPath);

//getters
static inline MorphScene* morphEditorGetActiveScene(MorphEditor* editor)
{ return &editor->tabs[editor->activeTab].content.scene; }

#ifdef __cplusplus
}
#endif