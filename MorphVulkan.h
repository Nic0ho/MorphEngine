#pragma once

#include "MorphTypes.h"
#include "vulkan/vulkan.h"
#include <GLFW/glfw3.h>

typedef struct
{
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice  physicalDevice;
    u32 graphicsFamily;
    VkDevice logicalDevice;
    VkQueue graphicsQueue;
    VkSurfaceKHR surface;
} MorphVulkanContext;

bool morphVulkanInit(MorphVulkanContext* ctx, GLFWwindow* window);
void morphVulkanShutdown(MorphVulkanContext* ctx);