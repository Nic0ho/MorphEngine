#include <vector>
#include <assert.h>

#include "VulkanCore/src/Common/MorphTypes.h"
#include "VulkanCore/MorphVulkanCore.h"
#include "VulkanCore/MorphVulkanUtil.h"

namespace MorphVK
{
VulkanCore::VulkanCore()
{
}

VulkanCore::~VulkanCore()
{
    vkDestroyInstance(m_instance, NULL);
    printf("Vulkan instance destroyed\n");
}

void VulkanCore::Init(const char* pAppName)
{ CreateInstance(pAppName); }

void VulkanCore::CreateInstance(const char* pAppName)
{
    std::vector<const char*> Layers = { "VK_LAYER_KHRONOS_validation" };
    std::vector<const char*> Extensions =
    {
        VK_KHR_SURFACE_EXTENSION_NAME,
#if defined (_WIN32)
        "VK_KHR_win32_surface",
#endif
#if defined (__APPLE__)
        "VK_MVK_macos_surface",
#endif
#if defined (__linux__)
        "VK_KHR_xcb_surface",
#endif
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
        VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
    };

    VkApplicationInfo AppInfo =
    {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = NULL,
        .pApplicationName = pAppName,
        .applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
        .pEngineName = "Morph Engine",
        .engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
        .apiVersion = VK_API_VERSION_1_0
    };

    VkInstanceCreateInfo CreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .pApplicationInfo = &AppInfo,
        .enabledLayerCount = (u32)(Layers.size()),
        .ppEnabledLayerNames = Layers.data(),
        .enabledExtensionCount = (u32)(Extensions.size()),
        .ppEnabledExtensionNames = Extensions.data()
    };

    VkResult res = vkCreateInstance(&CreateInfo, NULL, &m_instance);
    CHECK_VK_RESULT(res, "Create instance");
    printf("Vulkan instance create\n");
}
}