#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "MorphVulkanDevice.h"
#include "Common/MorphTypes.h"
#include "MorphVulkanQueue.h"

namespace MorphVK
{
class BufferAndMemory
{
public:
    BufferAndMemory() {}

    VkBuffer m_buffer = NULL;
    VkDeviceMemory m_mem = NULL;
    VkDeviceSize m_allocationSize = 0;

    void Update(VkDevice Device, const void* pData, size_t Size);

    void Destroy(VkDevice Device);
};

class VulkanTexture
{
public:
    VulkanTexture() = default;

    VkImage m_image = VK_NULL_HANDLE;
    VkDeviceMemory m_mem = VK_NULL_HANDLE;
    VkImageView m_view = VK_NULL_HANDLE;
    VkSampler m_sampler = VK_NULL_HANDLE;

    void Destroy(VkDevice Device);
};

class VulkanCore
{
public:
    VulkanCore();

    ~VulkanCore();

    void Init(const char* pAppName, GLFWwindow* pWindow, bool DepthEnabled = false);

    int GetNumImages() const { return (int)m_images.size(); }

    const VkImage& GetImage(int Index) const;

    VulkanQueue* GetQueue() { return &m_queue; }

    VkDevice GetDevice() const { return m_device; }

    void CreateCommandBuffers(u32 Count, VkCommandBuffer* pCmdBufs);
    void FreeCommandBuffers(u32 Count, const VkCommandBuffer* pCmdBufs);

    std::vector<VkFramebuffer> CreateFramebuffer(VkRenderPass RenderPass);
    void DestroyFramebuffers(const std::vector<VkFramebuffer>& Framebuffers);

    VkRenderPass CreateSimpleRenderPass();
    BufferAndMemory CreateVertexBuffer(const void* pVertices, size_t Size);

    void GetFramebufferSize(int& Width, int& Height) const;
    std::vector<BufferAndMemory> CreateUniformBuffers(size_t Size);

    void CreateTexture(const char* pFilename, VulkanTexture& Tex);
    VulkanTexture CreateDepthBuffer();
    void CreateDepthResources();

private:
    void CreateInstance(const char* pAppName);
    void CreateDebugCallback();
    void CreateSurface();
    void CreateDevice();
    void CreateSwapChain();
    void CreateCommandBufferPool();
    u32 GetMemoryTypeIndex(u32 MemTypeBitsMask, VkMemoryPropertyFlags ReqMemPropFlags);
    void CopyBuffer(VkBuffer Dst, VkBuffer Src, VkDeviceSize Size);
    BufferAndMemory CreateBuffer(VkDeviceSize Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags Properties);
    BufferAndMemory CreateUniformBuffer(size_t Size);

    void CreateTextureImageFromData(VulkanTexture& Tex, const void* pPixels, u32 ImageWidth, u32 ImageHeight, VkFormat TexFormat);
    void CreateImage(VulkanTexture& Tex, u32 ImageWidth, u32 ImageHeight, VkFormat TexFormat, VkImageUsageFlags UsageFlags, VkMemoryPropertyFlagBits PropertyFlags);
    void UpdateTextureImage(VulkanTexture& Tex, u32 ImageWidth, u32 ImageHeight, VkFormat TexFormat, const void* pPixels);
    void CopyBufferToImage(VkImage Dst, VkBuffer Src, u32 ImageWidth, u32 ImageHeight);
    void TransitionImageLayout(VkImage& Image, VkFormat Format, VkImageLayout OldLayout, VkImageLayout NewLayout);
    void SubmitCopyCommand();
    void CopyBufferToBuffer(VkBuffer Dst, VkBuffer Src, VkDeviceSize Size);

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
    std::vector<VulkanTexture> m_depthImages;
    VkCommandPool m_cmdBufPool;
    VulkanQueue m_queue;
    std::vector<VkFramebuffer> m_frameBuffers;
    VkCommandBuffer m_copyCmdBuf;
    int m_windowWidth = 0;
    int m_windowHeight = 0;
    bool m_depthEnabled = false;
};
}