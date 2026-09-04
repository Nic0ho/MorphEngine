#pragma once

#include "MorphEditor.h"
#include "MorphVulkan.h"
#include "MorphMath.h"
#include "MorphTypes.h"
#include "MorphLog.h"
#include <GLFW/glfw3.h>

#define MAX_HISTORY 64

#ifdef __cplusplus
extern "C"
{
#endif

// Init / frame / shutdown
bool morphImGuiInit(MorphVulkanContext* ctx, GLFWwindow* window);
void morphImGuiNewFrame(void);
void morphImGuiEndFrame(void);
void morphImGuiRender(VkCommandBuffer cmd);
void morphImGuiShutdown(MorphVulkanContext* ctx);
void morphImGuiSetIniPath(const char* path);

// Panels
void morphImGuiDrawOutput(MorphOutputConsoleBuffer* buffer);
void morphImGuiDrawAssetBrowser(MorphEditor* editor);
void morphImGuiDrawFolderOverview(MorphEditor* editor);
void morphImGuiDrawTools(void);
void morphImGuiDrawOutliner(MorphEditor* editor);
void morphImGuiDrawDetails(MorphEditor* editor);
void morphImGuiDrawViewport(VkDescriptorSet descriptorSet, u32 texWidth, u32 texHeight);
void morphImGuiDrawMenuBar(MorphEditor* editor, GLFWwindow* window, f32 deltaTime);
void morphImGuiDrawHub(MorphEditor* editor, MorphCamera* camera);

// One-liner wrappers
void morphImGuiBeginDockspace(void);
void morphImGuiBeginWindow(const char* name);
void morphImGuiEndWindow(void);
void morphBeginTiledWindow(const char* name, f32 gap);
void morphEndTiledWindow(void);
VkDescriptorSet morphImGuiRegisterTexture(VkSampler sampler, VkImageView view);
void morphImGuiResetContentBrowser(void);

// Getters
Vec2 morphImGuiGetViewportSize(void);
bool morphImGuiGetViewportFocusedCursor(void);

#ifdef __cplusplus
}
#endif
