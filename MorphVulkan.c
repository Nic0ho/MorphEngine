#include "MorphVulkan.h"
#include "MorphArena.h"
#include "MorphCamera.h"
#include "MorphLog.h"

#include <GLFW/glfw3.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>

typedef struct
{
    f32 pos[2];
    f32 color[3];
    f32 uv[2];
} Vertex;

static const Vertex VERTICES[] =
{
    {{0.2f, -0.2f}, {0.7f, 0.2f, 0.3f}, {1.0f, 0.0f}},
    {{0.2f, 0.2f}, {0.1f, 0.3f, 0.7f}, {1.0f, 1.0f}},
    {{-0.2f, 0.2f}, {0.9f, 0.7f, 0.5f}, {0.0f, 1.0f}},
    {{-0.2f, -0.2f}, {0.9f, 0.7f, 0.5f}, {0.0f, 0.0f}}
};

static const u16 INDICES[] = { 0, 1, 2, 0, 2, 3 };

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
    MorphArena arena;
    u32 count = 0;
    vkEnumerateInstanceLayerProperties(&count, NULL);

    if (!morphArenaCreate(&arena, count * sizeof(VkLayerProperties)))
    { return false; };

    VkLayerProperties* available = (VkLayerProperties*)morphArenaAlloc(&arena, count * sizeof(VkLayerProperties), _Alignof(VkLayerProperties));
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
            morphArenaDestroy(&arena);
            return false;
        }
    }
    morphArenaDestroy(&arena);
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

    if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        morphLog(LOG_ERROR, "%s\n", data->pMessage);
    else if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        morphLog(LOG_WARNING, "%s\n", data->pMessage);
    else
        morphLog(LOG_MESSAGE, "%s\n", data->pMessage);

    return VK_FALSE; 
}

static VkShaderModule loadShader(VkDevice device, const char* path)
{
    MorphArena arena;

    //open file in binary
    FILE* f = fopen(path, "rb");
    if (!f)
    {
        morphLog(LOG_ERROR, "Cannot open shader: %s", path);
        return VK_NULL_HANDLE;
    }

    // get file size
    fseek(f, 0, SEEK_END);
    usize size = (usize)ftell(f);
    fseek(f, 0, SEEK_SET);

    //read bytes
    if (!morphArenaCreate(&arena, size))
    { return VK_NULL_HANDLE; }
    
    u32* code = (u32*)morphArenaAlloc(&arena, size, _Alignof(u32));
    fread(code, 1, size, f);
    fclose(f);

    VkShaderModuleCreateInfo info = {0};
    info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.codeSize = size;
    info.pCode = code;

    VkShaderModule module;
    if (vkCreateShaderModule(device, &info, NULL, &module) != VK_SUCCESS)
    {
        morphLog(LOG_ERROR, "Failed to create shader module: %s", path);
        morphArenaDestroy(&arena);
        return VK_NULL_HANDLE;
    }

    morphArenaDestroy(&arena);
    morphLog(LOG_MESSAGE, "Shader loaded: %s", path);
    return module;
}

static bool createSyncObjects(MorphVulkanContext* ctx)
{
    //one imageAvailable semaphore per swapchai image
    ctx->imageAvailableSemaphores = malloc(ctx->swapchainImageCount * sizeof(VkSemaphore));
    ctx->renderFinishedSemaphores = malloc(ctx->swapchainImageCount * sizeof(VkSemaphore));

    VkSemaphoreCreateInfo semInfo = {0};
    semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {0};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    //SIGNALED = frence starts signaled so first frame doesnt`t wait forever
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (u32 i = 0; i < ctx->swapchainImageCount; i++)
    {
        if (vkCreateSemaphore(ctx->logicalDevice, &semInfo, NULL, &ctx->imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(ctx->logicalDevice, &semInfo, NULL, &ctx->renderFinishedSemaphores[i]) != VK_SUCCESS)
        {
            morphLog(LOG_ERROR, "Failed to create semaphores for image %u !", i);
            return false;
        }
    }

    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vkCreateFence(ctx->logicalDevice, &fenceInfo, NULL, &ctx->inFlightFences[i]) != VK_SUCCESS)
        {
            morphLog(LOG_ERROR, "Failed to create fence for frame %u !", i);
            return false;
        }
    }

    morphLog(LOG_MESSAGE, "Sync objects created");
    return true;
}

static void transitionImage(VkCommandBuffer cmd, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkImageMemoryBarrier2 barrier = {0};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    barrier.image = image;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;

    barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    barrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;

    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

    VkDependencyInfo depInfo = {0};
    depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    depInfo.imageMemoryBarrierCount = 1;
    depInfo.pImageMemoryBarriers = &barrier;

    vkCmdPipelineBarrier2(cmd, &depInfo);
}

static void recordCommandBuffer(MorphVulkanContext* ctx, u32 imageIndex, MorphCamera* camera)
{
    VkCommandBuffer cmd = ctx->commandBuffers[ctx->currentFrame];

    VkCommandBufferBeginInfo beginInfo = {0};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(cmd, &beginInfo);

    //transition: UNDEFINED -> COLOR_ATTACHMENT (ready to render into)
    transitionImage(cmd, ctx->swapchainImages[imageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    //begin dynamic rendering
    VkClearValue clearColor = {{{ 0.1f, 0.1f, 0.15f, 1.0f}}}; //black

    VkRenderingAttachmentInfo colorAttachment = {0};
    colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    colorAttachment.imageView = ctx->swapchainImageViews[imageIndex];
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; //clear on start
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; //keep after rendering
    colorAttachment.clearValue = clearColor;

    VkRenderingInfo renderingInfo = {0};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.renderArea.offset = (VkOffset2D){0, 0};
    renderingInfo.renderArea.extent = ctx->swapchainExtent;
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &colorAttachment;

    vkCmdBeginRendering(cmd, &renderingInfo);

    //bind pipeline
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->graphicsPipeline);

    //push transform directly into command buffer
    f32 aspectRatio = (f32)ctx->swapchainExtent.width / (f32)ctx->swapchainExtent.height;

    Mat4 transform = morphCameraGetViewProjection(camera, aspectRatio);
    vkCmdPushConstants(cmd, ctx->pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Mat4), &transform);

    VkDescriptorImageInfo imageInfo = {0};
    imageInfo.sampler = ctx->texture.sampler;
    imageInfo.imageView = ctx->texture.view;
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet write = {0};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstBinding = 0;
    write.descriptorCount = 1;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.pImageInfo = &imageInfo;

    ctx->fnPushDescriptors(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->pipelineLayout, 0, 1, &write);

    //set dynamic viewport
    VkViewport viewport = {0};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (f32)ctx->swapchainExtent.width;
    viewport.height = (f32)ctx->swapchainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vkCmdSetViewport(cmd, 0, 1, &viewport);

    //set dynamic scissor
    VkRect2D scissor = {0};
    scissor.offset = (VkOffset2D){0, 0};
    scissor.extent = ctx->swapchainExtent;
    
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    //bind vertex buffer
    VkBuffer vertexBuffers[] = {ctx->vertexBuffer.buffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);

    //bind index buffer
    vkCmdBindIndexBuffer(cmd, ctx->indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);


    //draw 3 verices, 1 instacne, startit at vertex 0
    vkCmdDrawIndexed(cmd, ctx->indexCount, 1, 0, 0, 0);
    vkCmdEndRendering(cmd);

    //transition: COLOR_ATTACHMENT -> PRESENT_SRC (ready to show om screen)
    transitionImage(cmd, ctx->swapchainImages[imageIndex], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    vkEndCommandBuffer(cmd);
}

static bool pickPhysicalDevice(MorphVulkanContext* ctx)
{
    MorphArena arena;

    // checking for amount of GPU`s
    u32 deviceCount = 0;
    vkEnumeratePhysicalDevices(ctx->instance, &deviceCount, NULL);

    if (deviceCount == 0)
    {
        morphLog(LOG_ERROR, "No GPUs with Vulkan support found!");
        return false;
    }

    if (!morphArenaCreate(&arena, deviceCount * sizeof(VkPhysicalDevice)))
    { return false; }

    //getting GPUs
    VkPhysicalDevice* devices = (VkPhysicalDevice*)morphArenaAlloc(&arena, deviceCount * sizeof(VkPhysicalDevice), _Alignof(VkPhysicalDevice));
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
            morphLog(LOG_MESSAGE, "GPU selected: %s", props.deviceName);
            break;
        }
    }

    //no discrete GPU case (any GPU)
    if (ctx->physicalDevice == VK_NULL_HANDLE)
    {
        ctx->physicalDevice = devices[0];
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(ctx->physicalDevice, &props);
        morphLog(LOG_MESSAGE, "No discrete GPU, falling back to: %s", props.deviceName);
    }

    morphArenaDestroy(&arena);

    // find a queue family supports graphics
    u32 familyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(ctx->physicalDevice, &familyCount, NULL);
    
    if (!morphArenaCreate(&arena, familyCount * sizeof(VkQueueFamilyProperties)))
    { return false; }

    VkQueueFamilyProperties* families = (VkQueueFamilyProperties*)morphArenaAlloc(&arena, familyCount * sizeof(VkQueueFamilyProperties), _Alignof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(ctx->physicalDevice, &familyCount, families);

    ctx->graphicsFamily = UINT32_MAX;

    for (u32 i = 0; i < familyCount; i++)
    {
        if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            ctx->graphicsFamily = i;
            morphLog(LOG_MESSAGE, "Graphics queue family: %u", i);
            break;
        }
    }

    morphArenaDestroy(&arena);

    if (ctx->graphicsFamily == UINT32_MAX)
    {
        morphLog(LOG_ERROR, "No graphics queue family found!");
        return false;
    }

    return true;
}

static bool pickSurfaceFormat(VkPhysicalDevice device, VkSurfaceKHR surface, VkSurfaceFormatKHR* out)
{
    MorphArena arena;
    u32 count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, NULL);

    if (!morphArenaCreate(&arena, count* sizeof(VkSurfaceFormatKHR)))
    { return false; }

    VkSurfaceFormatKHR* formats = (VkSurfaceFormatKHR*)morphArenaAlloc(&arena, count* sizeof(VkSurfaceFormatKHR), _Alignof(VkSurfaceFormatKHR));
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
    
    morphArenaDestroy(&arena);
    
    *out = result;
    return true;
}

static bool pickPresentMode(VkPhysicalDevice device, VkSurfaceKHR surface, VkPresentModeKHR* out)
{
    MorphArena arena;
    u32 count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, NULL);

    if (!morphArenaCreate(&arena, count * sizeof(VkPresentModeKHR)))
    { return false; }

    VkPresentModeKHR* modes = (VkPresentModeKHR*)morphArenaAlloc(&arena, count * sizeof(VkPresentModeKHR), _Alignof(VkPresentModeKHR));
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

    morphArenaDestroy(&arena);

    *out = result;
    return true;
}

static bool createSwapchain(MorphVulkanContext* ctx, GLFWwindow* window)
{
    //query what the surface supports
    VkSurfaceCapabilitiesKHR caps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(ctx->physicalDevice, ctx->surface, &caps);

    VkSurfaceFormatKHR format;
    if (!pickSurfaceFormat(ctx->physicalDevice, ctx->surface, &format))
    { return false; }

    VkPresentModeKHR mode;
    if (!pickPresentMode(ctx->physicalDevice, ctx->surface, &mode))
    { return false; }

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

    if (vkCreateSwapchainKHR(ctx->logicalDevice, &swapchainInfo, NULL, &ctx->swapchain) != VK_SUCCESS)
    {
        morphLog(LOG_ERROR, "Failed to create swapchain!");
        return false;
    }

    //retrieve the actual images the swapchain created (Vulkan may have created more than required)
    vkGetSwapchainImagesKHR(ctx->logicalDevice, ctx->swapchain, &ctx->swapchainImageCount, NULL);

    ctx->swapchainImages = malloc( ctx->swapchainImageCount * sizeof(VkImage));
    vkGetSwapchainImagesKHR(ctx->logicalDevice, ctx->swapchain, &ctx->swapchainImageCount, ctx->swapchainImages);

    ctx->swapchainFormat = format.format;
    ctx->swapchainExtent = extent;

    morphLog(LOG_MESSAGE, "Swapchain created (%ux%u, %u images)", extent.width, extent.height, ctx->swapchainImageCount);
    
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
    features13.synchronization2 = VK_TRUE;

    VkPhysicalDeviceVulkan14Features features14 = {0};
    features14.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES;
    features14.pushDescriptor = VK_TRUE;
    features14.pNext = &features13;

    // needed device extensions
    const char* deviceExtensions[] =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME, //for presenting images to window
        VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME
    };

    VkDeviceCreateInfo deviceInfo = {0};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.pNext = &features14;
    deviceInfo.queueCreateInfoCount = 1;
    deviceInfo.pQueueCreateInfos = &queueInfo;
    deviceInfo.enabledExtensionCount = 2;
    deviceInfo.ppEnabledExtensionNames = deviceExtensions;

    if (vkCreateDevice(ctx->physicalDevice, &deviceInfo, NULL, &ctx->logicalDevice) != VK_SUCCESS)
    {
        morphLog(LOG_ERROR, "Failed to create logical device!");
        return false;
    }

    // get the queue handle (0 iundex means the first queue in this family)
    vkGetDeviceQueue(ctx->logicalDevice, ctx->graphicsFamily, 0, &ctx->graphicsQueue);

    ctx->fnPushDescriptors = (PFN_vkCmdPushDescriptorSetKHR)vkGetDeviceProcAddr(ctx->logicalDevice, "vkCmdPushDescriptorSetKHR");

    morphLog(LOG_MESSAGE, "Logical device created");
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
            morphLog(LOG_ERROR, "Failed to create image view %u !", i);
            return false;
        }
    }

    morphLog(LOG_MESSAGE, "Image views created (%u)", ctx->swapchainImageCount);
    return true;
}

static bool createVertexBuffer(MorphVulkanContext* ctx)
{
    VkDeviceSize size = sizeof(VERTICES);

    //staging buffer - CPU write here
    MorphBuffer staging;
    if (!morphBufferCreate(ctx->logicalDevice, ctx->physicalDevice, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &staging))
        return false;

    //copy vertex data into staging buffer
    void* data;
    vkMapMemory(ctx->logicalDevice, staging.memory, 0, size, 0, &data);
    memcpy(data, VERTICES, (usize)size);
    vkUnmapMemory(ctx->logicalDevice, staging.memory);

    //vertex buffer - GPU reads from gere (DEVICE_LOCAL = fasttest)
    if (!morphBufferCreate(ctx->logicalDevice, ctx->physicalDevice, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &ctx->vertexBuffer))
        return false;

    //copy staging -> vertex buffer
    morphBufferCopy(ctx->logicalDevice, ctx->commandPool, ctx->graphicsQueue, &staging, &ctx->vertexBuffer, size);

    //staging no longer needed
    morphBufferDestroy(ctx->logicalDevice, &staging);

    morphLog(LOG_MESSAGE, "Vertex buffer created");
    
    return true;
}

static bool createIndexBuffer(MorphVulkanContext* ctx)
{
    ctx->indexCount = LEN(INDICES);
    VkDeviceSize size = sizeof(INDICES);

    MorphBuffer staging;
    if (!morphBufferCreate(ctx->logicalDevice, ctx->physicalDevice, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &staging))
        return false;

    void* data;
    vkMapMemory(ctx->logicalDevice, staging.memory, 0, size, 0, &data);
    memcpy(data, INDICES, (usize)size);
    vkUnmapMemory(ctx->logicalDevice, staging.memory);

    if (!morphBufferCreate(ctx->logicalDevice, ctx->physicalDevice, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &ctx->indexBuffer))
        return false;

    morphBufferCopy(ctx->logicalDevice, ctx->commandPool, ctx->graphicsQueue, &staging, &ctx->indexBuffer, size);

    morphBufferDestroy(ctx->logicalDevice, &staging);

    morphLog(LOG_MESSAGE, "Index buffer created (%u indices)", ctx->indexCount);
    return true;
}

static bool createDescriptorSetLayout(MorphVulkanContext* ctx)
{
    VkDescriptorSetLayoutBinding binding = {0};
    binding.binding = 0;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    binding.descriptorCount = 1;
    binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo = {0};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &binding;

    if (vkCreateDescriptorSetLayout(ctx->logicalDevice, &layoutInfo, NULL, &ctx->descriptorSetLayout) != VK_SUCCESS)
    {
        morphLog(LOG_ERROR, "Failed to create descriptor set layout!");
        return false;
    }

    morphLog(LOG_MESSAGE, "Descriptor set layout created");
    
    return true;
}

static bool createGraphicsPipeline(MorphVulkanContext* ctx)
{
    //load shaders
    VkShaderModule vertShader = loadShader(ctx->logicalDevice, "shaders/triangle.vert.spv");
    VkShaderModule fragShader = loadShader(ctx->logicalDevice, "shaders/triangle.frag.spv");

    if (vertShader == VK_NULL_HANDLE || fragShader == VK_NULL_HANDLE)
    {
        morphLog(LOG_ERROR, "Failed to load shaders!");
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

    //vertex input - describe the vertex buffer layout
    VkVertexInputBindingDescription binding = {0};
    binding.binding = 0;
    binding.stride = sizeof(Vertex);
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attrs[3] = {0};
    //pos
    attrs[0].binding = 0;
    attrs[0].location = 0;
    attrs[0].format = VK_FORMAT_R32G32_SFLOAT;
    attrs[0].offset = offsetof(Vertex, pos);
    //color
    attrs[1].binding = 0;
    attrs[1].location = 1;
    attrs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attrs[1].offset = offsetof(Vertex, color);
    //uv
    attrs[2].binding = 0;
    attrs[2].location = 2;
    attrs[2].format = VK_FORMAT_R32G32_SFLOAT;
    attrs[2].offset = offsetof(Vertex, uv);

    VkPipelineVertexInputStateCreateInfo vertexInput = {0};
    vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInput.vertexBindingDescriptionCount = 1;
    vertexInput.pVertexBindingDescriptions = &binding;
    vertexInput.vertexAttributeDescriptionCount = 3;
    vertexInput.pVertexAttributeDescriptions = attrs;

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
    colorBlendAttachment.blendEnable = VK_TRUE;
    //color
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;
    //alpha
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD;

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

    VkPushConstantRange pushRange = {0};
    pushRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushRange.offset = 0;
    pushRange.size = sizeof(Mat4);

    //pipeline layout - describes push counstannts and descriptor sets. Empty for now since triangle is harcoded
    VkPipelineLayoutCreateInfo layoutInfo = {0};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.setLayoutCount = 1;
    layoutInfo.pSetLayouts = &ctx->descriptorSetLayout;
    layoutInfo.pushConstantRangeCount = 1;
    layoutInfo.pPushConstantRanges = &pushRange;

    if (vkCreatePipelineLayout(ctx->logicalDevice, &layoutInfo, NULL, &ctx->pipelineLayout) != VK_SUCCESS)
    {
        morphLog(LOG_ERROR, "Failed to create pipeline layout!");
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
        morphLog(LOG_ERROR, "Failed to create graphics pipeline!");
        return false;
    }

    //shaders baked into pipeline - modules no longer needed
    vkDestroyShaderModule(ctx->logicalDevice, vertShader, NULL);
    vkDestroyShaderModule(ctx->logicalDevice, fragShader, NULL);

    morphLog(LOG_MESSAGE, "Graphics pipeline created");
    return true;
}

static bool createCommandPool(MorphVulkanContext* ctx)
{
    VkCommandPoolCreateInfo poolInfo = {0};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = ctx->graphicsFamily;

    //RESET_COMMAN_BUFFER_BIT = allow individual vommad buffers to be re-recorder. without this flag needs to reset the entire pool to re-record any buffer
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(ctx->logicalDevice, &poolInfo, NULL, &ctx->commandPool) != VK_SUCCESS)
    {
        morphLog(LOG_ERROR, "Failed to create command pool!");
        return false;
    }

    //allocate command buffers from the pool
    VkCommandBufferAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = ctx->commandPool;
    //PRIMARY = can be sub,itted to queue directly
    //SECONDATY = can only be called from primarty buffers
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

    if (vkAllocateCommandBuffers(ctx->logicalDevice, &allocInfo, ctx->commandBuffers) != VK_SUCCESS)
    {
        morphLog(LOG_ERROR, "Failed to allocate command buffers!");
        return false;
    }

    morphLog(LOG_MESSAGE, "Command pool and buffers created");
    return true;
}

static void cleanupSwapchain(MorphVulkanContext* ctx)
{
    for (u32 i = 0; i < ctx->swapchainImageCount; i++)
        vkDestroyImageView(ctx->logicalDevice, ctx->swapchainImageViews[i], NULL);
    free(ctx->swapchainImageViews);

    free(ctx->swapchainImages);
    vkDestroySwapchainKHR(ctx->logicalDevice, ctx->swapchain, NULL);
}

static bool recreateSwapchain(MorphVulkanContext* ctx, GLFWwindow* window)
{
    //handle minimized - block until window has a valid size again
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(ctx->logicalDevice); // GPU must finish before destroying what its using
    
    cleanupSwapchain(ctx);

    if (!createSwapchain(ctx, window) ||
        !createImageViews(ctx))
        return false;

    morphLog(LOG_MESSAGE, "Swapchain recreated (%ux%u)", ctx->swapchainExtent.width, ctx->swapchainExtent.height);
    
    return true;
}

void morphVulkanDraw(MorphVulkanContext* ctx, GLFWwindow* window, MorphCamera* camera)
{
    //wait untill this frame slot is free (GPU finished with it)
    vkWaitForFences(ctx->logicalDevice, 1, &ctx->inFlightFences[ctx->currentFrame], VK_TRUE, UINT64_MAX);

    u32 semIdx = ctx->acquireIndex;

    //get next swapchain image
    u32 imageIndex;
    VkResult result = vkAcquireNextImageKHR(ctx->logicalDevice, ctx->swapchain, UINT64_MAX, ctx->imageAvailableSemaphores[semIdx], VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        if (!recreateSwapchain(ctx, window))
            morphLog(LOG_ERROR, "Failed to recreate swapchain!");
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        morphLog(LOG_ERROR, "Failed to acquire swapchain image!");
        return;
    }

    //reset fence only after we know we`re going to submit
    vkResetFences(ctx->logicalDevice, 1, &ctx->inFlightFences[ctx->currentFrame]);

    //record commands
    vkResetCommandBuffer(ctx->commandBuffers[ctx->currentFrame], 0);
    recordCommandBuffer(ctx, imageIndex, camera);

    //submit
    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submitInfo = {0};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &ctx->imageAvailableSemaphores[semIdx];
    submitInfo.pWaitDstStageMask = &waitStage;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &ctx->commandBuffers[ctx->currentFrame];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &ctx->renderFinishedSemaphores[imageIndex];

    vkQueueSubmit(ctx->graphicsQueue, 1, &submitInfo, ctx->inFlightFences[ctx->currentFrame]);

    //present
    VkPresentInfoKHR presentInfo = {0};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &ctx->renderFinishedSemaphores[imageIndex];
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &ctx->swapchain;
    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(ctx->graphicsQueue, &presentInfo);

    //advance to next frame slot
    ctx->acquireIndex = (ctx->acquireIndex + 1) % ctx->swapchainImageCount;
    ctx->currentFrame = (ctx->currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

bool morphVulkanInit(MorphVulkanContext* ctx, GLFWwindow* window)
{
    if (VALIDATION_ENABLED && !checkValidationSupport())
    {
        morphLog(LOG_WARNING, "Validation layers not available!");
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
    MorphArena arena;

    if (!morphArenaCreate(&arena, extCount * sizeof(const char*)))
    { return false; }

    const char** extensions = (const char**)morphArenaAlloc(&arena, extCount * sizeof(const char*), _Alignof(const char**));
    
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
        morphLog(LOG_ERROR, "Failed to create Vulkan instance!");
        morphArenaDestroy(&arena);
        return false;
    }

    morphArenaDestroy(&arena);
    morphLog(LOG_MESSAGE, "Vulkan instance created");

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
            morphLog(LOG_ERROR, "Debug messenger creation failed!");
        else
            morphLog(LOG_MESSAGE, "Debug messenger created");
    }

    //window surface
    if (glfwCreateWindowSurface(ctx->instance, window, NULL, &ctx->surface) != VK_SUCCESS)
    {
        morphLog(LOG_ERROR, "Failed to create window surface!");
        return false;
    }
    morphLog(LOG_MESSAGE, "Window surface created");

        
    if (!pickPhysicalDevice(ctx)        || //physical device
        !createLogicalDevice(ctx)       || //logical device
        !createSwapchain(ctx, window)   || //swapchain
        !createImageViews(ctx)          || //image views
        !createDescriptorSetLayout(ctx) || //descriptor set
        !createGraphicsPipeline(ctx)    || //graphics pipeline
        !createCommandPool(ctx)         || //command pool
        !createSyncObjects(ctx)         || //create sync objects
        !createVertexBuffer(ctx)        || //create vertex buffer
        !createIndexBuffer(ctx)         ||  //create index buffer
        !morphTextureLoad(ctx->logicalDevice, ctx->physicalDevice, ctx->commandPool, ctx->graphicsQueue, "assets/test.png", &ctx->texture))
        return false;

    return true;
}

void morphVulkanShutdown(MorphVulkanContext *ctx)
{
    vkDeviceWaitIdle(ctx->logicalDevice); //wait for GPU to finish before destroy

    //debug messenger shutdown
    if (VALIDATION_ENABLED && ctx->debugMessenger != VK_NULL_HANDLE)
    {
        PFN_vkDestroyDebugUtilsMessengerEXT destroyFn = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(ctx->instance, "vkDestroyDebugUtilsMessengerEXT");

        if (destroyFn)
            destroyFn(ctx->instance, ctx->debugMessenger, NULL);
    }

    //sync objects shutdown
    for (u32 i = 0; i < ctx->swapchainImageCount; i++)
    {
        vkDestroySemaphore(ctx->logicalDevice, ctx->imageAvailableSemaphores[i], NULL);
        vkDestroySemaphore(ctx->logicalDevice, ctx->renderFinishedSemaphores[i], NULL);
    }
    free(ctx->imageAvailableSemaphores);
    free(ctx->renderFinishedSemaphores);

    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        vkDestroyFence(ctx->logicalDevice, ctx->inFlightFences[i], NULL);

    //buffer destroy
    morphBufferDestroy(ctx->logicalDevice, &ctx->indexBuffer);
    morphBufferDestroy(ctx->logicalDevice, &ctx->vertexBuffer);

    //texture shutdown
    morphTextureDestroy(ctx->logicalDevice, &ctx->texture);
    
    //graphics pipline shutdown
    vkDestroyPipeline(ctx->logicalDevice, ctx->graphicsPipeline, NULL);
    vkDestroyPipelineLayout(ctx->logicalDevice, ctx->pipelineLayout, NULL);

    //descriptor set shutdown
    vkDestroyDescriptorSetLayout(ctx->logicalDevice, ctx->descriptorSetLayout, NULL);

    //command pool shutdown
    vkDestroyCommandPool(ctx->logicalDevice, ctx->commandPool, NULL);

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

    morphLog(LOG_MESSAGE, "Vulkan shutdown");
}