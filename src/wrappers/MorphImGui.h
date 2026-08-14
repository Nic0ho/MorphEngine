#pragma once

#include "MorphEditor.h"
#include "MorphVulkan.h"
#include "MorphMath.h"
#include "MorphTypes.h"
#include "MorphLog.h"
#include <GLFW/glfw3.h>

#ifdef __cplusplus
extern "C"
{
#endif

bool morphImGuiInit(MorphVulkanContext* ctx, GLFWwindow* window);
void morphImGuiNewFrame();
void morphImGuiEndFrame();
void morphImGuiRender(VkCommandBuffer cmd);
void morphImGuiShutdown(MorphVulkanContext* ctx);

//panels
void morphImGuiDrawOutput(MorphOutputConsoleBuffer* buffer);
void morphImGuiDrawContentDrawer();
void morphImGuiDrawTools();
void morphImGuiDrawOutliner();
void morphImGuiDrawDetails();
void morphImGuiDrawViewport(VkDescriptorSet descriptorSet, u32 texWidth, u32 texHeight);
void morphImGuiDrawMenuBar(MorphEditor* editor);

//one line wrappers
void morphImGuiBeginDockspace();
void morphImGuiBeginWindow(const char* name);
void morphImGuiEndWindow();
VkDescriptorSet morphImGuiRegisterTexture(VkSampler sampler, VkImageView view);
bool morphImGuiGetViewportFocusedCursor();

//getters
Vec2 morphImGuiGetViewportSize();

#ifdef __cplusplus
}
#endif