#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "MorphVulkanDevice.h"
#include "Common/MorphTypes.h"
#include "MorphVulkanQueue.h"

namespace MorphVK
{
class VulkanCore
{
public:
    VulkanCore();

    ~VulkanCore();

    void Init(const char* pAppName, GLFWwindow* pWindow);

    int GetNumImages() const { return (int)m_images.size(); }

    const VkImage& GetImage(int Index) const;

    VulkanQueue* GetQueue() { return &m_queue; }

    VkDevice GetDevice() const { return m_device; }

    void CreateCommandBuffers(u32 Count, VkCommandBuffer* pCmdBufs);
    void FreeCommandBuffers(u32 Count, const VkCommandBuffer* pCmdBufs);

    std::vector<VkFramebuffer> CreateFramebuffer(VkRenderPass RenderPass);
    void DestroyFramebuffers(const std::vector<VkFramebuffer>& Framebuffers);

    VkRenderPass CreateSimpleRenderPass();

private:
    void CreateInstance(const char* pAppName);
    void CreateDebugCallback();
    void CreateSurface();
    void CreateDevice();
    void CreateSwapChain();
    void CreateCommandBufferPool();

    VkInstance m_instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
    GLFWwindow* m_pWindow = NULL;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VulkanPhysicalDevices m_physDevices;
    u32 m_queueFamily = 0;
    VkDevice m_device;
    VkSurfaceFormatKHR m_swapChainSurfaceFormat;
    VkSwapchainKHR m_swapChain;
    std::vector<VkImage> m_images;
    std::vector<VkImageView> m_imageViews;
    VkCommandPool m_cmdBufPool;
    VulkanQueue m_queue;
    std::vector<VkFramebuffer> m_frameBuffers;
};
}