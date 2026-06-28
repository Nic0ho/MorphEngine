#pragma once

#include "MorphTypes.h"
#include "vulkan/vulkan.h"
#include <GLFW/glfw3.h>

#define MAX_FRAMES_IN_FLIGHT 2

typedef struct
{
    VkInstance               instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice         physicalDevice;
    u32                      graphicsFamily;
    VkDevice                 logicalDevice;
    VkQueue                  graphicsQueue;
    VkSurfaceKHR             surface;
    VkSwapchainKHR           swapchain;
    VkFormat                 swapchainFormat;
    VkExtent2D               swapchainExtent;
    VkImage*                 swapchainImages;
    u32                      swapchainImageCount;
    VkImageView*             swapchainImageViews;
    VkPipelineLayout         pipelineLayout;
    VkPipeline               graphicsPipeline;
    VkCommandPool            commandPool;
    VkCommandBuffer          commandBuffers[MAX_FRAMES_IN_FLIGHT];
    
} MorphVulkanContext;

bool morphVulkanInit(MorphVulkanContext* ctx, GLFWwindow* window);
void morphVulkanShutdown(MorphVulkanContext* ctx);