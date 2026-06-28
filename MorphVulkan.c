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

static VkShaderModule loadShader(VkDevice device, const char* path)
{
    //open file in binary
    FILE* f = fopen(path, "rb");
    if (!f)
    {
        printf("[VULKAN ERROR] Cannot open shader: %s\n", path);
        return VK_NULL_HANDLE;
    }

    // get file size
    fseek(f, 0, SEEK_END);
    usize size = (usize)ftell(f);
    fseek(f, 0, SEEK_SET);

    //read bytes
    u32* code = malloc(size);
    fread(code, 1, size, f);
    fclose(f);

    VkShaderModuleCreateInfo info = {0};
    info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.codeSize = size;
    info.pCode = code;

    VkShaderModule module;
    if (vkCreateShaderModule(device, &info, NULL, &module) != VK_SUCCESS)
    {
        printf("[VULKAN ERROR] Failed to create shader module: %s\n", path);
        free(code);
        return VK_NULL_HANDLE;
    }

    free(code);
    printf("[VULKAN] Shader loaded: %s\n", path);
    return module;
}

static bool pickPhysicalDevice(MorphVulkanContext* ctx)
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

static bool createImageViews(MorphVulkanContext* ctx)
{
    ctx->swapchainImageViews = malloc(ctx->swapchainImageCount * sizeof(VkImageView));

    for (u32 i = 0; i < ctx->swapchainImageCount; i++)
    {
        VkImageViewCreateInfo viewInfo = {0};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = ctx->swapchainImages[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = ctx->swapchainFormat;

        //swizzle = which color channel maps to which. IDENTITY means R->R, G->G, B->B, A->A (no remapping)
        viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        //subresourceRange = which part of the image this view covers
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; //color image, not depth
        viewInfo.subresourceRange.baseMipLevel = 0; //start at mip level 0
        viewInfo.subresourceRange.levelCount = 1; //only one mip level (no mipmaps)
        viewInfo.subresourceRange.baseArrayLayer = 0; //start at layer 0
        viewInfo.subresourceRange.layerCount = 1; //only one layer (not cubemap or array)

        if (vkCreateImageView(ctx->logicalDevice, &viewInfo, NULL, &ctx->swapchainImageViews[i]) != VK_SUCCESS)
        {
            printf("[VULKAN ERROR] Failed to create image view %u\n", i);
            return false;
        }
    }

    printf("[VULKAN] Image views created (%u)\n", ctx->swapchainImageCount);
    return true;
}

static bool createGraphicsPipeline(MorphVulkanContext* ctx)
{
    //load shaders
    VkShaderModule vertShader = loadShader(ctx->logicalDevice, "shaders/triangle.vert.spv");
    VkShaderModule fragShader = loadShader(ctx->logicalDevice, "shaders/triangle.frag.spv");

    if (vertShader == VK_NULL_HANDLE || fragShader == VK_NULL_HANDLE)
    {
        printf("[VULKAN ERROR] Failed to load shaders!\n");
        return false;
    }

    //shader stage descriptors - tells pipeline whuch shader goes to which stage
    VkPipelineShaderStageCreateInfo shaderStages[2] = {0};

    //vert shader
    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].module = vertShader;
    shaderStages[0].pName = "main"; //entry point function name in the shader

    //frag shader
    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = fragShader;
    shaderStages[1].pName = "main"; //entry point funtion name in the shader

    //vertex input - empty, because harcoded in vertex shader
    VkPipelineVertexInputStateCreateInfo vertexInput = {0};
    vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    //input assembly - how to interpret vertices
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {0};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    //viewport and scissor - seet as dynamic state so we can resize without recreating pipeline
    VkPipelineViewportStateCreateInfo viewportState = {0};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;
    //actual values will set at drawtime via vkCmdSetViewport, vkCmdSetScissor

    //resterization - how to turn triangles into fragments
    VkPipelineRasterizationStateCreateInfo rasterizer = {0};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL; //not wireframe
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT; //skip back-facing triangles
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE; //vertex winding order
    rasterizer.lineWidth = 1.0f; //required even when not drawing lines
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;

    //multisampling - disable for now, 1 sample per pixel
    VkPipelineMultisampleStateCreateInfo multisampling = {0};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.sampleShadingEnable = VK_FALSE;

    //color blend attachment - how to write to the color attachment
    //blendEnable = VK_FALSE means just overwrite whatever was there
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {0};
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT |
        VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending = {0};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    //dynamic state - which pipeline settings can change at draw time without recreating pipeline
    VkDynamicState dynamicStates[] =
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState = {0};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;

    //pipeline layout - describes push counstannts and descriptor sets. Empty for now since triangle is harcoded
    VkPipelineLayoutCreateInfo layoutInfo = {0};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    if (vkCreatePipelineLayout(ctx->logicalDevice, &layoutInfo, NULL, &ctx->pipelineLayout) != VK_SUCCESS)
    {
        printf("[VULKAN ERROR] Failed to create pipeline layout!\n");
        return false;
    }

    //dynamic rendering info - replaces render pass object. decsribes what format the collor attachment will have
    VkPipelineRenderingCreateInfo renderingInfo = {0};
    renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachmentFormats = &ctx->swapchainFormat;

    //final pipeline createion
    VkGraphicsPipelineCreateInfo pipelineInfo = {0};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = &renderingInfo; //denamic rendering via pNext
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInput;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = ctx->pipelineLayout;

    if (vkCreateGraphicsPipelines(ctx->logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &ctx->graphicsPipeline) != VK_SUCCESS)
    {
        printf("[VULKAN ERROR] Failed to create graphics pipeline\n");
        return false;
    }

    //shaders baked into pipeline - modules no longer needed
    vkDestroyShaderModule(ctx->logicalDevice, vertShader, NULL);
    vkDestroyShaderModule(ctx->logicalDevice, fragShader, NULL);

    printf("[VULKAN] Graphics pipeline created\n");
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

    //image views
    if (!createImageViews(ctx))
        return false;

    //temp shader test
    if (!createGraphicsPipeline(ctx))
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

    //graphics pipline shutdown
    vkDestroyPipeline(ctx->logicalDevice, ctx->graphicsPipeline, NULL);
    vkDestroyPipelineLayout(ctx->logicalDevice, ctx->pipelineLayout, NULL);

    //image views shutdown
    for (u32 i = 0; i < ctx->swapchainImageCount; i++)
        vkDestroyImageView(ctx->logicalDevice, ctx->swapchainImageViews[i], NULL);
    free(ctx->swapchainImageViews);

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