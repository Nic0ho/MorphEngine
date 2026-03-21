#include <assert.h>

#include "Common/MorphTypes.h"
#include "MorphVulkanCore.h"
#include "MorphVulkanUtil.h"

namespace MorphVK
{
VulkanCore::VulkanCore()
{
}

VulkanCore::~VulkanCore()
{
    printf("--------------------------------------\n");

    vkDestroyDevice(m_device, NULL);

    PFN_vkDestroySurfaceKHR vkDestroySurface = VK_NULL_HANDLE;
    vkDestroySurface = (PFN_vkDestroySurfaceKHR)vkGetInstanceProcAddr(m_instance, "vkDestroySurfaceKHR");
    if(!vkDestroySurface)
    {
        MORPH_ERROR("Cannot find address of vkDestroyDebugUtilsMessenger\n");
        exit(1);
    }

    vkDestroySurface(m_instance, m_surface, NULL);

    printf("GLFW window surface destroyed\n");

    PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessenger = VK_NULL_HANDLE;
    vkDestroyDebugUtilsMessenger = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT");
    if(!vkDestroyDebugUtilsMessenger)
    {
        MORPH_ERROR("Cannot find address of vkDestroyDebugUtilsMessenger\n");
        exit(1);
    }
    vkDestroyDebugUtilsMessenger(m_instance, m_debugMessenger, NULL);

    printf("Debug callback destroyed\n");

    vkDestroyInstance(m_instance, NULL);
    printf("Vulkan instance destroyed\n");
}

void VulkanCore::Init(const char* pAppName, GLFWwindow* pWindow)
{
    m_pWindow = pWindow;
    CreateInstance(pAppName);
    CreateDebugCallback();
    CreateSurface(pWindow);
    m_physDevices.Init(m_instance, m_surface);
    m_queueFamily = m_physDevices.SelectDevice(VK_QUEUE_GRAPHICS_BIT, true);
    CreateDevice();
}

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

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback
(
    VkDebugUtilsMessageSeverityFlagBitsEXT Severity,
    VkDebugUtilsMessageTypeFlagsEXT Type,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData
)
{
    printf("Debug callback: %s\n", pCallbackData->pMessage);
    printf("  Severity %s\n", GetDebugSeverityStr(Severity));
    printf("  Type %s\n", GetDebugType(Type));
    printf("  Objects ");

    for(u32 i = 0; i  < pCallbackData->objectCount; i++)
    { printf("%llx ", pCallbackData->pObjects[i].objectHandle); }

    return VK_FALSE;
}

void VulkanCore::CreateDebugCallback()
{
    VkDebugUtilsMessengerCreateInfoEXT MessengerCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .pNext = NULL,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = &DebugCallback,
        .pUserData = NULL
    };

    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessenger =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT");
    if(!vkCreateDebugUtilsMessenger)
    {
        MORPH_ERROR("Cannot find address of vkCreateDebugUtilsMessenger\n");
        exit(1);
    }

    VkResult res = vkCreateDebugUtilsMessenger(m_instance, &MessengerCreateInfo, NULL, &m_debugMessenger);
    CHECK_VK_RESULT(res, "debug utils messenger");

    printf("Debug utils messenger created\n");
}

void VulkanCore::CreateSurface(GLFWwindow* pWindow)
{
    if(glfwCreateWindowSurface(m_instance, pWindow, NULL, &m_surface))
    {
        fprintf(stderr, "Error creating GLFW window surface\n");
        exit(1);
    }

    printf("GLFW window surface created\n");
}

void VulkanCore::CreateDevice()
{
    float qPriorities[] = { 1.0f };

    VkDeviceQueueCreateInfo qInfo =
    {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .queueFamilyIndex = m_queueFamily,
        .queueCount = 1,
        .pQueuePriorities = &qPriorities[0]
    };

    std::vector<const char*> DevExts =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME
    };

    if(m_physDevices.Selected().m_features.geometryShader == VK_FALSE)
    { MORPH_ERROR("The Geometry Shader is not supported!\n"); }

    if(m_physDevices.Selected().m_features.tessellationShader == VK_FALSE)
    { MORPH_ERROR("The Tessellation Shader is not supported!\n"); }

    VkPhysicalDeviceFeatures DeviceFeatures = { 0 };
    DeviceFeatures.geometryShader = VK_TRUE;
    DeviceFeatures.tessellationShader = VK_TRUE;

    VkDeviceCreateInfo DeviceCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &qInfo,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = NULL,
        .enabledExtensionCount = (u32)DevExts.size(),
        .ppEnabledExtensionNames = DevExts.data(),
        .pEnabledFeatures = NULL
    };

    VkResult res = vkCreateDevice(m_physDevices.Selected().m_physDevice, &DeviceCreateInfo, NULL, &m_device);
    CHECK_VK_RESULT(res, "CreateDevice\n");

    printf("\nDevice created\n");
}

static VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& PresentModes)
{
    for(int i = 0; i < PresentModes.size(); i++)
    { if(PresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) return PresentModes[i]; }
}

static u32 ChooseNumImages(const VkSurfaceCapabilitiesKHR& Capabilities)
{
    u32 RequestedNumImages = Capabilities.minImageCount + 1;

    int FinalNumImages = 0;

    if((Capabilities.maxImageCount > 0) && (RequestedNumImages > Capabilities.maxImageCount))
    { FinalNumImages = Capabilities.maxImageCount; }
    else
    { FinalNumImages = RequestedNumImages; }

    return FinalNumImages;
}

static VkSurfaceFormatKHR ChooseSurfaceFormatAndColorSpace(const std::vector<VkSurfaceFormatKHR>& SurfaceFormats)
{
    for(int i = 0; i < SurfaceFormats.size(); i++)
    {
        if((SurfaceFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB) && (SurfaceFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR))
        { return SurfaceFormats[i]; }
    }

    return SurfaceFormats[0];
}

VkImageView CreateImageView(VkDevice Device, VkImage Image, VkFormat Format, VkImageAspectFlags AspectFlags, VkImageViewType ViewType, u32 LayerCount,  u32 mipLevels)
{
    VkImageViewCreateInfo ViewInfo =
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .image = Image,
        .viewType = ViewType,
        .format = Format,
        .components =
        {
            .r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .a = VK_COMPONENT_SWIZZLE_IDENTITY
        },
        .subresourceRange =
        {
            .aspectMask = AspectFlags,
            .baseMipLevel = 0,
            .levelCount = mipLevels,
            .baseArrayLayer = 0,
            .layerCount = LayerCount
        }
    };
    VkImageView ImageView;
    VkResult res =vkCreateImageView(Device, &ViewInfo, NULL, &ImageView);
    CHECK_VK_RESULT(res, "vkCreateImageView");
    return ImageView;
}

void VulkanCore::CreateSwapChain()
{
    const VkSurfaceCapabilitiesKHR& SurfaceCaps = m_physDevices.Selected().m_surfaceCaps;

    u32 NumImages = ChooseNumImages(SurfaceCaps);

    const std::vector<VkPresentModeKHR>& PresentModes = m_physDevices.Selected().m_presentModes;
    VkPresentModeKHR PresentMode = ChoosePresentMode(PresentModes);

    VkSurfaceFormatKHR SurfaceFormat = ChooseSurfaceFormatAndColorSpace(m_physDevices.Selected().m_surfaceFormats);

    VkSwapchainCreateInfoKHR SwapChainCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = NULL,
        .flags = 0,
        .surface = m_surface,
        .minImageCount = NumImages,
        .imageFormat = SurfaceFormat.format,
        .imageColorSpace = SurfaceFormat.colorSpace,
        .imageExtent = SurfaceCaps.currentExtent,
        .imageArrayLayers = 1,
        .imageUsage = (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT),
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 1,
        .pQueueFamilyIndices = &m_queueFamily,
        .preTransform = SurfaceCaps.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = PresentMode,
        .clipped = VK_TRUE
    };

    VkResult res = vkCreateSwapchainKHR(m_device, &SwapChainCreateInfo, NULL, &m_swapChain);
    CHECK_VK_RESULT(res, "vkCreateSwapchainKHR");

    printf("Swap chain created\n");

    uint NumSwapChainImages = 0;
    res = vkGetSwapchainImagesKHR(m_device, m_swapChain, &NumSwapChainImages, NULL);
    CHECK_VK_RESULT(res, "vkGetSwapchainImagesKHR\n");
    assert(NumImages == NumSwapChainImages);

    printf("Number of images %d\n", NumSwapChainImages);

    m_images.resize(NumSwapChainImages);
    m_imageViews.resize(NumSwapChainImages);
}
}   