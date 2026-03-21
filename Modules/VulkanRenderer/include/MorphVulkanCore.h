#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "MorphVulkanDevice.h"
#include "Common/MorphTypes.h"

namespace MorphVK
{
class VulkanCore
{
public:
    VulkanCore();

    ~VulkanCore();

    void Init(const char* pAppName, GLFWwindow* pWindow);

private:
    void CreateInstance(const char* pAppName);
    void CreateDebugCallback();
    void CreateSurface(GLFWwindow* pWindow);
    void CreateDevice();
    void CreateSwapChain();

    VkInstance m_instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
    GLFWwindow* m_pWindow = NULL;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VulkanPhysicalDevices m_physDevices;
    u32 m_queueFamily = 0;
    VkDevice m_device;
    VkSwapchainKHR m_swapChain;
    std::vector<VkImage> m_images;
    std::vector<VkImageView> m_imageViews;
};
}