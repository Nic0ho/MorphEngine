#pragma once

#include "MorphTypes.h"
#include "vulkan/vulkan.h"
#include "MorphMath.h"
#include "MorphBuffer.h"
#include "MorphCamera.h"

#include <GLFW/glfw3.h>

#define MAX_FRAMES_IN_FLIGHT 2

typedef struct
{
    VkInstance                    instance;
    VkDebugUtilsMessengerEXT      debugMessenger;
    VkPhysicalDevice              physicalDevice;
    u32                           graphicsFamily;
    VkDevice                      logicalDevice;
    VkQueue                       graphicsQueue;
    VkSurfaceKHR                  surface;
    VkSwapchainKHR                swapchain;
    VkFormat                      swapchainFormat;
    VkExtent2D                    swapchainExtent;
    VkImage*                      swapchainImages;
    u32                           swapchainImageCount;
    VkImageView*                  swapchainImageViews;
    VkPipelineLayout              pipelineLayout;
    VkPipeline                    graphicsPipeline;
    VkCommandPool                 commandPool;
    VkCommandBuffer               commandBuffers[MAX_FRAMES_IN_FLIGHT];
    VkSemaphore*                  imageAvailableSemaphores;
    VkSemaphore*                  renderFinishedSemaphores;
    VkFence                       inFlightFences[MAX_FRAMES_IN_FLIGHT];
    u32                           currentFrame;
    u32                           acquireIndex;
    MorphBuffer                   vertexBuffer;
    MorphBuffer                   indexBuffer;
    u32                           indexCount;
    MorphTexture                  texture;
    VkDescriptorSetLayout         descriptorSetLayout;
    PFN_vkCmdPushDescriptorSetKHR fnPushDescriptors;
} MorphVulkanContext;

bool morphVulkanInit(MorphVulkanContext* ctx, GLFWwindow* window);
void morphVulkanShutdown(MorphVulkanContext* ctx);
void morphVulkanDraw(MorphVulkanContext* ctx, GLFWwindow* window, MorphCamera* camera);