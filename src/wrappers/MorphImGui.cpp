#include "MorphImGui.h"
#include "MorphLog.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include "imgui_internal.h"


extern "C"
{
bool morphImGuiInit(MorphVulkanContext *ctx, GLFWwindow *window)
{
    VkDescriptorPoolSize poolSize = {};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = 1000;

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = 1000;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;

    vkCreateDescriptorPool(ctx->logicalDevice, &poolInfo, nullptr, &ctx->imguiDescriptorPool);

    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui_ImplGlfw_InitForVulkan(window, true);

    VkPipelineRenderingCreateInfoKHR pipelineRenderingInfo = {};
    pipelineRenderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
    pipelineRenderingInfo.colorAttachmentCount = 1;
    pipelineRenderingInfo.pColorAttachmentFormats = &ctx->swapchainFormat;

    ImGui_ImplVulkan_InitInfo initInfo = {};
    initInfo.ApiVersion         = VK_API_VERSION_1_3;
    initInfo.Instance           = ctx->instance;
    initInfo.PhysicalDevice     = ctx->physicalDevice;
    initInfo.Device             = ctx->logicalDevice;
    initInfo.QueueFamily        = ctx->graphicsFamily;
    initInfo.Queue              = ctx->graphicsQueue;
    initInfo.DescriptorPoolSize = 1000;
    initInfo.MinImageCount      = 2;
    initInfo.ImageCount         = ctx->swapchainImageCount;
    initInfo.UseDynamicRendering = true;
    initInfo.PipelineInfoMain.PipelineRenderingCreateInfo = pipelineRenderingInfo;

    ImGui_ImplVulkan_Init(&initInfo);

    return true;
}

void morphImGuiNewFrame()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void morphImGuiEndFrame()
{
    ImGui::EndFrame();
    ImGui::UpdatePlatformWindows();
}

void morphImGuiRender(VkCommandBuffer cmd)
{
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
}

void morphImGuiShutdown(MorphVulkanContext* ctx)
{
    vkDeviceWaitIdle(ctx->logicalDevice);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    vkDestroyDescriptorPool(ctx->logicalDevice, ctx->imguiDescriptorPool, nullptr);
}

void morphImGuiBeginDockspace()
{ ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode); }

void morphImGuiBeginWindow(const char* name)
{ ImGui::Begin(name); }

void morphImGuiEndWindow()
{ ImGui::End(); }

VkDescriptorSet morphImGuiRegisterTexture(VkSampler sampler, VkImageView view)
{ return ImGui_ImplVulkan_AddTexture(sampler, view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL); }

void morphImGuiDrawOutput(MorphOutputConsoleBuffer* buffer)
{
    bool full = buffer->count == MAX_CONSOLE_OUTPUT_LINES;
    u32 start = full ? buffer->writeIndex : 0;

    for (u32 i = 0; i < buffer->count; i++)
    {
        u32 index = (start + i) % MAX_CONSOLE_OUTPUT_LINES;
        ImGui::TextUnformatted(buffer->messages[index]);
    }
}

void morphImGuiDrawContentDrawer()
{
    
}

void morphImGuiDrawTools()
{

}

void morphImGuiDrawOutliner()
{

}

void morphImGuiDrawDetails()
{

}

void morphImGuiDrawViewport(VkDescriptorSet descriptorSet, u32 texWidth, u32 texHeight)
{
    ImVec2 size = ImGui::GetContentRegionAvail();
    ImGui::Image((ImTextureID)descriptorSet, size);
}

void morphImGuiDrawMenuBar(MorphEditor *editor)
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            ImGui::MenuItem("Open HUB", NULL, nullptr);
            ImGui::MenuItem("Import", NULL, nullptr);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            ImGui::MenuItem("Project settings", NULL, nullptr);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Window"))
        {
            ImGui::MenuItem("Output", NULL, &editor->showOutput);
            ImGui::MenuItem("Content drawer", NULL, &editor->showContentDrawer);
            ImGui::MenuItem("Tools", NULL, &editor->showTools);
            ImGui::MenuItem("Outliner", NULL, &editor->showOutliner);
            ImGui::MenuItem("Details", NULL, &editor->showDetails);
            ImGui::MenuItem("Viewport", NULL, &editor->showViewport);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Build"))
        {
            ImGui::MenuItem("Build preferences", NULL, nullptr);
            ImGui::MenuItem("Build", NULL, nullptr);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

Vec2 morphImGuiGetViewportSize()
{
    ImVec2 size = ImGui::GetContentRegionAvail();
    Vec2 result = { (f32)size.x, (f32)size.y };

    return result;
}

bool morphImGuiGetViewportFocusedCursor()
{ return ImGui::IsWindowHovered(); }

}