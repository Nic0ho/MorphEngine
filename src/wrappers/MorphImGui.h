#pragma once

#include "MorphEditor.h"
#include "MorphVulkan.h"
#include "MorphMath.h"
#include "MorphTypes.h"
#include "MorphLog.h"
#include <GLFW/glfw3.h>

#define MAX_HISTORY 64
#define MAX_PATH_LEN 512

#ifdef __cplusplus
extern "C"
{
#endif

struct FileItem
{
    char name[256];
    bool isDir;
};

static char selectedPath[MAX_PATH_LEN] = "";
static char pathHistory[MAX_HISTORY][MAX_PATH_LEN];
static int pathHistoryCount = 0;
static int pathHistoryIndex = -1;

bool morphImGuiInit(MorphVulkanContext* ctx, GLFWwindow* window);
void morphImGuiNewFrame(void);
void morphImGuiEndFrame(void);
void morphImGuiRender(VkCommandBuffer cmd);
void morphImGuiShutdown(MorphVulkanContext* ctx);

//panels
void morphImGuiDrawOutput(MorphOutputConsoleBuffer* buffer);
void morphImGuiDrawAssetBrowser(void);
void morphImGuiDrawFolderOverview(MorphEditor* editor);
void morphImGuiDrawTools(void);
void morphImGuiDrawOutliner(Entities* scene, MorphEditor* editor);
void morphImGuiDrawDetails(Entities* scene, MorphEditor* editor);
void morphImGuiDrawViewport(VkDescriptorSet descriptorSet, u32 texWidth, u32 texHeight);
void morphImGuiDrawMenuBar(MorphEditor* editor, f32 deltaTime);

//one line wrappers
void morphImGuiBeginDockspace(void);
void morphImGuiBeginWindow(const char* name);
void morphImGuiEndWindow(void);
VkDescriptorSet morphImGuiRegisterTexture(VkSampler sampler, VkImageView view);
bool morphImGuiGetViewportFocusedCursor();

//getters
Vec2 morphImGuiGetViewportSize(void);

#ifdef __cplusplus
}
#endif