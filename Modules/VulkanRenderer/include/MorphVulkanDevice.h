#pragma once

#include <vulkan/vulkan.h>
#include <vector>

#include "Common/MorphTypes.h"

namespace MorphVK
{
struct PhysicalDevice
{
    VkPhysicalDevice m_physDevice;
    VkPhysicalDeviceProperties m_devProps;
    std::vector<VkQueueFamilyProperties> m_qFamilyProps;
    std::vector<VkBool32> m_qSupportsPresent;
    std::vector<VkSurfaceFormatKHR> m_surfaceFormats;
    VkSurfaceCapabilitiesKHR m_surfaceCaps;
    VkPhysicalDeviceMemoryProperties m_memProps;
    std::vector<VkPresentModeKHR> m_presentModes;
    VkPhysicalDeviceFeatures m_features;
    VkFormat m_depthFormat;
};

class VulkanPhysicalDevices
{
public:
    VulkanPhysicalDevices() {}
    ~VulkanPhysicalDevices() {}

    void Init(const VkInstance& Instance, const VkSurfaceKHR& Surface);

    u32 SelectDevice(VkQueueFlags RequiredQueueType, bool SupportsPresent);

    const PhysicalDevice& Selected() const;

private:
    std::vector<PhysicalDevice> m_devices;

    int m_devIndex = -1;
};
}