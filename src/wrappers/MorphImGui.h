#pragma once

#include "MorphVulkan.h"
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

//one line wrappers
void morphImGuiBeginDockspace();
void morphImGuiBeginWindow();
void morphImGuiEndWindow();

#ifdef __cplusplus
}
#endif