#pragma once

#include "MorphVulkan.h"
#include <GLFW/glfw3.h>

#ifdef __cplusplus
extern "C" {
#endif

bool morphImGuiInit(MorphVulkanContext* ctx, GLFWwindow* window);
void morphImGuiNewFrame();
void morphImGuiRender(VkCommandBuffer cmd);
void morphImGuiShutdown(MorphVulkanContext* ctx);

void morphImGuiShowDemo();

#ifdef __cplusplus
}
#endif