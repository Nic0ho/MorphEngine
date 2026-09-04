#include "MorphImGui.h"
#include "MorphLog.h"
#include "MorphTypes.h"
#include "MorphMath.h"
#include "GLFW/glfw3.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include <cstddef>
#include <cstring>
#include <stdlib.h>
#include <windows.h>

extern "C"
{

bool morphImGuiInit(MorphVulkanContext* ctx, GLFWwindow* window)
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

    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x0400, 0x052F, // Cyrillic + Cyrillic Supplement
        0x2DE0, 0x2DFF, // Cyrillic Extended-A
        0xA640, 0xA69F, // Cyrillic Extended-B
        0,
    };

    ImFontConfig fontConfig;
    fontConfig.OversampleH = 3;
    fontConfig.OversampleV = 3;
    fontConfig.PixelSnapH  = true;

    float dpiScaleX, dpiScaleY;
    glfwGetWindowContentScale(window, &dpiScaleX, &dpiScaleY);

    f32 baseFontSize = 16.0f;
    io.Fonts->AddFontFromFileTTF("assets\\fonts\\Nunito-Regular.ttf", baseFontSize * dpiScaleX, &fontConfig, ranges);
    ImGui::GetStyle().ScaleAllSizes(dpiScaleX);
    io.IniFilename = NULL;

    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui_ImplGlfw_InitForVulkan(window, true);

    VkPipelineRenderingCreateInfoKHR pipelineRenderingInfo = {};
    pipelineRenderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
    pipelineRenderingInfo.colorAttachmentCount = 1;
    pipelineRenderingInfo.pColorAttachmentFormats = &ctx->swapchainFormat;

    ImGui_ImplVulkan_InitInfo initInfo = {};
    initInfo.ApiVersion = VK_API_VERSION_1_3;
    initInfo.Instance = ctx->instance;
    initInfo.PhysicalDevice = ctx->physicalDevice;
    initInfo.Device = ctx->logicalDevice;
    initInfo.QueueFamily = ctx->graphicsFamily;
    initInfo.Queue = ctx->graphicsQueue;
    initInfo.DescriptorPoolSize = 1000;
    initInfo.MinImageCount = 2;
    initInfo.ImageCount = ctx->swapchainImageCount;
    initInfo.UseDynamicRendering = true;
    initInfo.PipelineInfoMain.PipelineRenderingCreateInfo = pipelineRenderingInfo;

    ImGui_ImplVulkan_Init(&initInfo);

    // Global style — see lessons in handoff before touching these
    ImGuiStyle& style          = ImGui::GetStyle();
    style.WindowPadding        = ImVec2(12.0f, 12.0f);
    style.WindowRounding       = 8.0f;
    style.WindowBorderSize     = 1.0f;
    style.Colors[ImGuiCol_Border] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);

    return true;
}

void morphImGuiSetIniPath(const char* path)
{
    ImGuiIO& io  = ImGui::GetIO();
    io.IniFilename = path;

    wchar_t widePath[MAX_PATH_LEN];
    MultiByteToWideChar(CP_ACP, 0, path, -1, widePath, MAX_PATH_LEN);

    HANDLE hFile = CreateFileW(widePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        DWORD size = GetFileSize(hFile, NULL);
        char* buf = (char*)malloc(size + 1);
        DWORD bytesRead = 0;
        ReadFile(hFile, buf, size, &bytesRead, NULL);
        CloseHandle(hFile);
        buf[bytesRead] = '\0';
        ImGui::LoadIniSettingsFromMemory(buf, bytesRead);
        free(buf);
        morphLog(LOG_MESSAGE, "Project layout loaded from: %s", path);
    }
    else morphLog(LOG_MESSAGE, "No saved project layout found: %s", path);
}

void morphImGuiNewFrame(void)
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void morphImGuiEndFrame(void)
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

void morphImGuiBeginDockspace(void)
{ ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode); }

void morphImGuiBeginWindow(const char* name)
{ ImGui::Begin(name); }

void morphImGuiEndWindow(void)
{ ImGui::End(); }

void morphBeginTiledWindow(const char* name, f32 gap)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin(name);
    ImGui::PopStyleVar();

    ImGui::SetCursorPos(ImVec2(gap, gap));

    ImVec2 size = ImGui::GetContentRegionAvail();
    size.x -= gap;
    size.y -= gap;

    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 10.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f, 12.0f));
    ImGui::BeginChild("##TiledInner", size, ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding);
}

void morphEndTiledWindow(void)
{
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
    ImGui::End();
}

VkDescriptorSet morphImGuiRegisterTexture(VkSampler sampler, VkImageView view)
{ return ImGui_ImplVulkan_AddTexture(sampler, view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL); }

Vec2 morphImGuiGetViewportSize(void)
{
    ImVec2 size  = ImGui::GetContentRegionAvail();
    Vec2 result  = { (f32)size.x, (f32)size.y };
    return result;
}

bool morphImGuiGetViewportFocusedCursor(void)
{ return ImGui::IsWindowHovered(); }

}