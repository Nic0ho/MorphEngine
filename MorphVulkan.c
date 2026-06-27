#include "MorphVulkan.h"
#include <GLFW/glfw3.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//Vulkan validation layers
static const char* VALIDATION_LAYERS[] = { "VK_LAYER_KHRONOS_validation" };
static const u32 VALIDATION_LAYER_COUNT = 1;

#ifdef NDEBUG
    static const bool VALIDATION_ENABLED = false;
#else
    static const bool VALIDATION_ENABLED = true;
#endif

static bool checkValidationSupport(void)
{
    u32 count = 0;
    vkEnumerateInstanceLayerProperties(&count, NULL);

    VkLayerProperties* available = malloc(count * sizeof(VkLayerProperties));
    vkEnumerateInstanceLayerProperties(&count, available);

    for (u32 i = 0; i < VALIDATION_LAYER_COUNT; i++)
    {
        bool found = false;
        for (u32 j = 0; j < count; j++)
        {
            if (strcmp(VALIDATION_LAYERS[i], available[j].layerName) == 0)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            free(available);
            return false;
        }
    }
    free(available);
    return true;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback
(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT* data,
    void* userData
)
{
    (void)type;
    (void)userData;

    if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        printf("[VULKAN ERROR] %s\n\n", data->pMessage);
    else
        printf("[VULKAN] %s\n\n", data->pMessage);

    return VK_FALSE; 
}

static bool  pickPhysicalDevice(MorphVulkanContext* ctx)
{
    // checking for amount of GPU`s
    u32 deviceCount = 0;
    vkEnumeratePhysicalDevices(ctx->instance, &deviceCount, NULL);

    if (deviceCount == 0)
    {
        printf("[VULKAN ERROR] No GPUs with Vulkan support found\n");
        return false;
    }

    //getting GPUs
    VkPhysicalDevice* devices = malloc(deviceCount * sizeof(VkPhysicalDevice));
    vkEnumeratePhysicalDevices(ctx->instance, &deviceCount, devices);

    // picking the first GPU
    ctx->physicalDevice = VK_NULL_HANDLE;

    for (u32 i = 0; i < deviceCount; i++)
    {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(devices[i], &props);

        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            ctx->physicalDevice = devices[i];
            printf("[VULKAN] GPU selected: %s\n", props.deviceName);
            break;
        }
    }

    //no discrete GPU case (any GPU)
    if (ctx->physicalDevice == VK_NULL_HANDLE)
    {
        ctx->physicalDevice = devices[0];
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(ctx->physicalDevice, &props);
        printf("[VULKAN] No discrete GPU, falling back to: %s\n", props.deviceName);
    }

    free(devices);

    // find a queue family supports graphics
    u32 familyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(ctx->physicalDevice, &familyCount, NULL);
    
    VkQueueFamilyProperties* families = malloc(familyCount * sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(ctx->physicalDevice, &familyCount, families);

    ctx->graphicsFamily = UINT32_MAX;

    for (u32 i = 0; i < familyCount; i++)
    {
        if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            ctx->graphicsFamily = i;
            printf("[VULKAN] Graphics queue family: %u\n", i);
            break;
        }
    }

    free(families);

    if (ctx->graphicsFamily == UINT32_MAX)
    {
        printf("[VULKAN ERROR] No graphics queue family found\n");
        return false;
    }

    return true;
}

static VkSurfaceFormatKHR pickSurfaceFormat(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    u32 count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, NULL);

    VkSurfaceFormatKHR* formats = malloc(count* sizeof(VkSurfaceFormatKHR));
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, formats);

    //prefer sRGB B8G8R8A8
    VkSurfaceFormatKHR result = formats[0]; //fallback
    for (u32 i = 0; i < count; i++)
    {
        if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            result = formats[i];
            break;
        }
    }
    
    free(formats);
    return result;
}

static VkPresentModeKHR pickPresentMode(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    u32 count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, NULL);

    VkPresentModeKHR* modes = malloc(count * sizeof(VkPresentModeKHR));
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, modes);

    //prefer mailbox, fallback to FIFO
    VkPresentModeKHR result = VK_PRESENT_MODE_FIFO_KHR;
    for (u32 i = 0; i < count; i++)
    {
        if (modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            result = VK_PRESENT_MODE_MAILBOX_KHR;
            break;
        }
    }

    free(modes);
    return result;
}

static bool createSwapchain(MorphVulkanContext* ctx, GLFWwindow* window)
{
    //query what the surface supports
    VkSurfaceCapabilitiesKHR caps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(ctx->physicalDevice, ctx->surface, &caps);

    VkSurfaceFormatKHR format = pickSurfaceFormat(ctx->physicalDevice, ctx->surface);
    VkPresentModeKHR mode = pickPresentMode(ctx->physicalDevice, ctx->surface);

    //extent = resolution of swapchain images
    //query actual pixel size from GLFW - differs from screen coord on hight-DPI
    VkExtent2D extent;
    if (caps.currentExtent.width != UINT32_MAX)
    {
        extent = caps.currentExtent;
    }
    else
    {
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        extent.width = (u32)w;
        extent.height = (u32)h;

        //clamp to what the surface supports
        if (extent.width < caps.minImageExtent.width)   extent.width = caps.minImageExtent.width;
        if (extent.width > caps.maxImageExtent.width)   extent.width = caps.maxImageExtent.width;
        if (extent.height < caps.minImageExtent.height) extent.height = caps.minImageExtent.height;
        if (extent.height > caps.maxImageExtent.height) extent.height = caps.maxImageExtent.height;
    }

    //request one more image than minumim for better pipelining (but dont exceed maximum, where 0 means no maximum)
    u32 imageCount = caps.minImageCount + 1;
    if (caps.maxImageCount > 0 && imageCount > caps.maxImageCount)
        imageCount = caps.maxImageCount;

    VkSwapchainCreateInfoKHR swapchainInfo = {0};
    swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainInfo.surface = ctx->surface;
    swapchainInfo.minImageCount = imageCount;
    swapchainInfo.imageFormat = format.format;
    swapchainInfo.imageColorSpace = format.colorSpace;
    swapchainInfo.imageExtent = extent;
    swapchainInfo.imageArrayLayers = 1; //always will be 1 unless VR stereo rendering
    swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // one queue family owns images
    swapchainInfo.preTransform = caps.currentTransform; //no extra rotation/flip
    swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; //no window transparancy
    swapchainInfo.presentMode = mode;
    swapchainInfo.clipped = VK_TRUE; //dont render pixels hidden behind other windows

    if (vkCreateSwapchainKHR(ctx->logicalDevice, &swapchainInfo, NULL, & ctx->swapchain))
    {
        printf("[VULKAN ERROR] Failed to create swapchain!\n");
        return false;
    }

    //retrieve the actual images the swapchain created (Vulkan may have created more than required)
    vkGetSwapchainImagesKHR(ctx->logicalDevice, ctx->swapchain, &ctx->swapchainImageCount, NULL);

    ctx->swapchainImages = malloc(ctx->swapchainImageCount * sizeof(VkImage));
    vkGetSwapchainImagesKHR(ctx->logicalDevice, ctx->swapchain, &ctx->swapchainImageCount, ctx->swapchainImages);

    ctx->swapchainFormat = format.format;
    ctx->swapchainExtent = extent;

    printf("[VULKAN] Swapchain created (%ux%u, %u images)\n", extent.width, extent.height, ctx->swapchainImageCount);
    
    return true;
}

static bool createLogicalDevice(MorphVulkanContext* ctx)
{
    // tell the Vulkan that i need one queue from the graphics family (1.0 is the highest priority)
    f32 queuePriority = 1.0f;

    VkDeviceQueueCreateInfo queueInfo = {0};
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.queueFamilyIndex = ctx->graphicsFamily;
    queueInfo.queueCount = 1;
    queueInfo.pQueuePriorities = &queuePriority;

    //FEATURES
    VkPhysicalDeviceVulkan13Features features13 = {0};
    features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    features13.dynamicRendering = VK_TRUE;

    VkPhysicalDeviceVulkan14Features features14 = {0};
    features14.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES;
    features14.pushDescriptor = VK_TRUE;
    features14.pNext = &features13;

    // needed device extensions
    const char* deviceExtensions[] =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME //for presenting images to window
    };

    VkDeviceCreateInfo deviceInfo = {0};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.pNext = &features14;
    deviceInfo.queueCreateInfoCount = 1;
    deviceInfo.pQueueCreateInfos = &queueInfo;
    deviceInfo.enabledExtensionCount = 1;
    deviceInfo.ppEnabledExtensionNames = deviceExtensions;

    if (vkCreateDevice(ctx->physicalDevice, &deviceInfo, NULL, &ctx->logicalDevice) != VK_SUCCESS)
    {
        printf("[VULKAN ERROR] Failed to create logical device\n");
        return false;
    }

    // get the queue handle (0 iundex means the first queue in this family)
    vkGetDeviceQueue(ctx->logicalDevice, ctx->graphicsFamily, 0, &ctx->graphicsQueue);

    printf("[VULKAN] Logical device created\n");
    return true;
}

bool morphVulkanInit(MorphVulkanContext* ctx, GLFWwindow* window)
{
    if (VALIDATION_ENABLED && !checkValidationSupport())
    {
        printf("Validation layers not available");
        return false;
    }

    //app info
    VkApplicationInfo appInfo = {0};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "MorphEngine";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "MorphEngine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_4;

    //extensions
    u32 glfwExtCount = 0;
    const char** glfwExts = glfwGetRequiredInstanceExtensions(&glfwExtCount);

    u32 extCount = glfwExtCount + (VALIDATION_ENABLED ? 1 : 0);
    const char** extensions = malloc(extCount * sizeof(const char*));
    
    for (u32 i = 0; i < glfwExtCount; i++)
        extensions[i] = glfwExts[i];

    if (VALIDATION_ENABLED)
        extensions[glfwExtCount] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;


    //instance
    VkInstanceCreateInfo instanceInfo = {0};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &appInfo;
    instanceInfo.enabledExtensionCount = extCount;
    instanceInfo.ppEnabledExtensionNames = extensions;
    if (VALIDATION_ENABLED)
    {
        instanceInfo.enabledLayerCount = VALIDATION_LAYER_COUNT;
        instanceInfo.ppEnabledLayerNames = VALIDATION_LAYERS;
    }

    if (vkCreateInstance(&instanceInfo, NULL, &ctx->instance) != VK_SUCCESS)
    {
        printf("[VULKAN ERROR] Failed to create Vulkan instance\n");
        free(extensions);
        return false;
    }

    free(extensions);
    printf("[VULKAN] Vulkan instance created\n");

    //debug messenger
    ctx->debugMessenger = VK_NULL_HANDLE;
    if(VALIDATION_ENABLED)
    {
        VkDebugUtilsMessengerCreateInfoEXT dmInfo = {0};
        dmInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        dmInfo.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        dmInfo.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        dmInfo.pfnUserCallback = debug_callback;

        PFN_vkCreateDebugUtilsMessengerEXT createFn = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(ctx->instance, "vkCreateDebugUtilsMessengerEXT");

        if (!createFn || createFn(ctx->instance, &dmInfo, NULL, &ctx->debugMessenger) != VK_SUCCESS)
            printf("[VULKAN ERROR] Debug messenger creation failed\n");
        else
            printf("[VULKAN] Debug messenger created\n");
    }

    //window surface
    if (glfwCreateWindowSurface(ctx->instance, window, NULL, &ctx->surface) != VK_SUCCESS)
    {
        printf("[VULKAN ERROR] Failed to create window surface!\n");
        return false;
    }
    printf("[VULKAN] Window surface created\n");

    //physical device
    if (!pickPhysicalDevice(ctx))
        return false;

    //logical device
    if (!createLogicalDevice(ctx))
        return false;

    //swapchain
    if (!createSwapchain(ctx, window))
        return false;

    return true;
}

void morphVulkanShutdown(MorphVulkanContext *ctx)
{
    //debug messenger shutdown
    if (VALIDATION_ENABLED && ctx->debugMessenger != VK_NULL_HANDLE)
    {
        PFN_vkDestroyDebugUtilsMessengerEXT destroyFn = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(ctx->instance, "vkDestroyDebugUtilsMessengerEXT");

        if (destroyFn)
            destroyFn(ctx->instance, ctx->debugMessenger, NULL);
    }

     // swapchain shutdown
    free(ctx->swapchainImages);
    vkDestroySwapchainKHR(ctx->logicalDevice, ctx->swapchain, NULL);

    // logical device shutdown
    vkDestroyDevice(ctx->logicalDevice, NULL);

    //window surface shutdown
    vkDestroySurfaceKHR(ctx->instance, ctx->surface, NULL);
    
    // instance shutdown
    vkDestroyInstance(ctx->instance, NULL);

    printf("[VULKAN] Vulkan shutdown\n");
}