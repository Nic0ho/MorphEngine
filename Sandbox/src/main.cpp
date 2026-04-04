#include "MorphVulkanUtil.h"
#include <stdio.h>
#include <stdlib.h>

#include <GLFW/glfw3.h>

#define GLFW_INCLUDE_VULKAN
#include "MorphVulkanCore.h"
#include "MorphVulkanWrapper.h"
#include "MorphVulkanShader.h"
#include "MorphVulkanGraphicsPipeline.h"


#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080
#define APP_NAME "Morph"

GLFWwindow* window = NULL;

void GLFW_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if((key == GLFW_KEY_ESCAPE) && (action == GLFW_PRESS))
    { glfwSetWindowShouldClose(window, GLFW_TRUE); }
}

class VulkanApp
{
public:
    VulkanApp()
    {

    }

    ~VulkanApp()
    {
        m_vkCore.FreeCommandBuffers((u32)m_cmdBufs.size(), m_cmdBufs.data());
        m_vkCore.DestroyFramebuffers(m_frameBuffers);
        vkDestroyShaderModule(m_vkCore.GetDevice(), m_vs, NULL);
        vkDestroyShaderModule(m_vkCore.GetDevice(), m_fs, NULL);
        delete m_pPipeline;
        vkDestroyRenderPass(m_vkCore.GetDevice(), m_renderPass, NULL);
    }

    void Init(const char* pAppName, GLFWwindow* pWindow)
    {
        m_vkCore.Init(pAppName, pWindow);
        m_numImages = m_vkCore.GetNumImages();
        m_pQueue = m_vkCore.GetQueue();
        m_renderPass = m_vkCore.CreateSimpleRenderPass();
        m_frameBuffers =m_vkCore.CreateFramebuffer(m_renderPass);
        CreateShaders();
        CreatePipeline();
        CreateCommandBuffers();
        RecordCommandBuffers();
    }

    void RenderScene()
    {
        u32 ImageIndex =m_pQueue->AcquireNextImage();

        m_pQueue->SubmitAsync(m_cmdBufs[ImageIndex]);

        m_pQueue->Present(ImageIndex);
    }

private:
    void CreateCommandBuffers()
    {
        m_cmdBufs.resize(m_numImages);
        m_vkCore.CreateCommandBuffers(m_numImages, m_cmdBufs.data());

        printf("Created command buffers\n");
    }

    void CreateShaders()
    {
        m_vs = MorphVK::CreateShaderModuleFromText(m_vkCore.GetDevice(), "shaders/test.vert");
        m_fs = MorphVK::CreateShaderModuleFromText(m_vkCore.GetDevice(), "shaders/test.frag");
    }

    void CreatePipeline()
    { m_pPipeline = new MorphVK::GraphicsPipeline(m_vkCore.GetDevice(), window, m_renderPass, m_vs, m_fs); }

    void RecordCommandBuffers()
    {
        VkClearColorValue ClearColor = { 1.0f, 0.0f, 0.0f, 1.0f };
        VkClearValue ClearValue;
        ClearValue.color = ClearColor;

        int WindowWidth, WindowHeight;
        glfwGetFramebufferSize(window, &WindowWidth, &WindowHeight);

        VkRenderPassBeginInfo RenderPassBeginInfo =
        {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .pNext = NULL,
            .renderPass = m_renderPass,
            .renderArea =
            {
                .offset = { .x = 0, .y = 0 },
                .extent = { .width = static_cast<uint32_t>(WindowWidth), .height = static_cast<uint32_t>(WindowHeight) }
            },
            .clearValueCount = 1,
            .pClearValues = &ClearValue
        };

        for(uint i = 0; i < m_cmdBufs.size(); i++)
        {
            MorphVK::BeginCommandBuffer(m_cmdBufs[i], VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

            RenderPassBeginInfo.framebuffer = m_frameBuffers[i];

            vkCmdBeginRenderPass(m_cmdBufs[i], &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

            m_pPipeline->Bind(m_cmdBufs[i]);

            u32 VertexCount = 3;
            u32 InstanceCount = 1;
            u32 FirstVertex = 0;
            u32 FirstInstance = 0;

            vkCmdDraw(m_cmdBufs[i], VertexCount, InstanceCount, FirstVertex, FirstInstance);

            vkCmdEndRenderPass(m_cmdBufs[i]);
            
            VkResult res = vkEndCommandBuffer(m_cmdBufs[i]);
            CHECK_VK_RESULT(res, "vkEndCommandBuffer\n");
        }

        printf("Command buffers recorded\n");
    }

    MorphVK::VulkanCore m_vkCore;
    MorphVK::VulkanQueue* m_pQueue = NULL;
    int m_numImages = 0;
    std::vector<VkCommandBuffer> m_cmdBufs;
    VkRenderPass m_renderPass;
    std::vector<VkFramebuffer> m_frameBuffers;
    VkShaderModule m_vs;
    VkShaderModule m_fs;
    MorphVK::GraphicsPipeline* m_pPipeline = NULL;
};

int main(int argc, char* argv[])
{
    if(!glfwInit()) return 1;
    if(!glfwVulkanSupported()) return 1;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, APP_NAME, nullptr, nullptr);

    if(!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, GLFW_KeyCallback);

    VulkanApp App;
    App.Init(APP_NAME, window);

    while(!glfwWindowShouldClose(window))
    {
        App.RenderScene();
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}