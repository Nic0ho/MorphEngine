#include "MorphVulkanUtil.h"
#include <stdio.h>
#include <stdlib.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "MorphVulkanCore.h"
#include "MorphVulkanWrapper.h"
#include "MorphVulkanShader.h"
#include "MorphVulkanGraphicsPipeline.h"
#include "MorphVulkanSimpleMesh.h"
#include "MorphVulkanGLFW.h"

#include "Camera/MorphCamera.h"

#define WINDOW_WIDTH  1920
#define WINDOW_HEIGHT 1080
#define APP_NAME      "Morph"

class VulkanApp : public MorphVK::GLFWCallbacks
{
public:
    VulkanApp(int w, int h) : m_windowWidth(w), m_windowHeight(h) {}

    ~VulkanApp()
    {
        m_vkCore.FreeCommandBuffers((u32)m_cmdBufs.size(), m_cmdBufs.data());
        m_vkCore.DestroyFramebuffers(m_frameBuffers);
        vkDestroyShaderModule(m_vkCore.GetDevice(), m_vs, NULL);
        vkDestroyShaderModule(m_vkCore.GetDevice(), m_fs, NULL);
        delete m_pPipeline;
        vkDestroyRenderPass(m_vkCore.GetDevice(), m_renderPass, NULL);
        m_mesh.Destroy(m_vkCore.GetDevice());
        for (auto& ub : m_uniformBuffers)
            ub.Destroy(m_vkCore.GetDevice());
    }

    void Init(const char* pAppName)
    {
        m_pWindow = MorphVK::glfw_vulkan_init(WINDOW_WIDTH, WINDOW_HEIGHT, pAppName);

        glfwSetInputMode(m_pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        m_vkCore.Init(pAppName, m_pWindow);
        m_numImages = m_vkCore.GetNumImages();
        m_pQueue    = m_vkCore.GetQueue();

        m_renderPass  = m_vkCore.CreateSimpleRenderPass();
        m_frameBuffers = m_vkCore.CreateFramebuffer(m_renderPass);

        CreateShaders();
        CreateVertexBuffer();
        CreateUniformBuffers();
        CreatePipeline();
        CreateCommandBuffers();
        RecordCommandBuffers();

        MorphVK::glfw_vulkan_set_callbacks(m_pWindow, this);

        float aspect = (float)m_windowWidth / (float)m_windowHeight;
        m_camera = Morph::Camera({ 0.f, 0.f, 3.f }, 45.f, aspect, 0.1f, 1000.f);
    }

    void RenderScene(float dt)
    {
        u32 ImageIndex = m_pQueue->AcquireNextImage();
        UpdateUniformBuffers(ImageIndex, dt);
        m_pQueue->SubmitAsync(m_cmdBufs[ImageIndex]);
        m_pQueue->Present(ImageIndex);
    }


    void Key(GLFWwindow* pWindow, int Key, int Scancode, int Action, int Mods) override
    {
        if (Key >= 0 && Key < 512)
        {
            if (Action == GLFW_PRESS)   m_keys[Key] = true;
            if (Action == GLFW_RELEASE) m_keys[Key] = false;
        }

        if (Key == GLFW_KEY_ESCAPE && Action == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(pWindow, GLFW_TRUE);
        }
    }

    void MouseMove(GLFWwindow* pWindow, double xpos, double ypos) override
    {
        if (!m_isCameraActive) return;
        
        if (m_firstMouse)
        {
            m_lastMouseX = (float)xpos;
            m_lastMouseY = (float)ypos;
            m_firstMouse = false;
        }

        float dx =  (float)xpos - m_lastMouseX;
        float dy = -(float)ypos + m_lastMouseY;

        m_lastMouseX = (float)xpos;
        m_lastMouseY = (float)ypos;

        m_camera.ProcessMouseMovement(dx, dy);
    }

    void MouseButton(GLFWwindow* pWindow, int Button, int Action, int Mods) override {
        if (Button == GLFW_MOUSE_BUTTON_RIGHT)
        {
            if (Action == GLFW_PRESS)
            {
                m_isCameraActive = true;
                m_firstMouse = true;
                glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
            else if (Action == GLFW_RELEASE)
            {
                m_isCameraActive = false;
                glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
        }
    }

    void Execute()
    {
        float prevTime = (float)glfwGetTime();

        while (!glfwWindowShouldClose(m_pWindow))
        {
            float curTime = (float)glfwGetTime();
            float dt      = curTime - prevTime;
            prevTime = curTime;

            ProcessCameraInput(dt);

            RenderScene(dt);
            glfwPollEvents();
        }

        glfwDestroyWindow(m_pWindow);
        glfwTerminate();
    }

private:
    void CreateCommandBuffers()
    {
        m_cmdBufs.resize(m_numImages);
        m_vkCore.CreateCommandBuffers(m_numImages, m_cmdBufs.data());
        printf("Created command buffers\n");
    }

    void CreateVertexBuffer()
    {
        struct Vertex
        {
            Vertex(float x, float y, float z, float u, float v)
                : x(x), y(y), z(z), u(u), v(v) {}
            float x, y, z, u, v;
        };

        std::vector<Vertex> Vertices =
        {
            Vertex(-1.f, -1.f, 0.f,  0.f, 0.f),
            Vertex( 1.f, -1.f, 0.f,  1.f, 0.f),
            Vertex( 0.f,  1.f, 0.f,  0.5f, 1.f)
        };

        m_mesh.m_vertexBufferSize = sizeof(Vertices[0]) * Vertices.size();
        m_mesh.m_vb = m_vkCore.CreateVertexBuffer(Vertices.data(), m_mesh.m_vertexBufferSize);
    }

    struct UniformData { glm::mat4 WVP; };

    void CreateUniformBuffers()
    { m_uniformBuffers = m_vkCore.CreateUniformBuffers(sizeof(UniformData)); }

    void CreateShaders()
    {
        m_vs = MorphVK::CreateShaderModuleFromText(m_vkCore.GetDevice(), "shaders/test.vert");
        m_fs = MorphVK::CreateShaderModuleFromText(m_vkCore.GetDevice(), "shaders/test.frag");
    }

    void CreatePipeline()
    {
        m_pPipeline = new MorphVK::GraphicsPipeline(
            m_vkCore.GetDevice(), m_pWindow, m_renderPass,
            m_vs, m_fs, &m_mesh,
            m_numImages, m_uniformBuffers, sizeof(UniformData));
    }

    void RecordCommandBuffers()
    {
        VkClearColorValue ClearColor = { 0.1f, 0.1f, 0.15f, 1.0f };
        VkClearValue ClearValue;
        ClearValue.color = ClearColor;

        int W, H;
        glfwGetFramebufferSize(m_pWindow, &W, &H);

        VkRenderPassBeginInfo RenderPassBeginInfo =
        {
            .sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .pNext       = NULL,
            .renderPass  = m_renderPass,
            .renderArea  =
            {
                .offset = { 0, 0 },
                .extent = { (uint32_t)W, (uint32_t)H }
            },
            .clearValueCount = 1,
            .pClearValues    = &ClearValue
        };

        for(uint i = 0; i < m_cmdBufs.size(); i++)
        {
            MorphVK::BeginCommandBuffer(m_cmdBufs[i], VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

            RenderPassBeginInfo.framebuffer = m_frameBuffers[i];
            vkCmdBeginRenderPass(m_cmdBufs[i], &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

            m_pPipeline->Bind(m_cmdBufs[i], i);
            vkCmdDraw(m_cmdBufs[i], 3, 1, 0, 0);

            vkCmdEndRenderPass(m_cmdBufs[i]);

            VkResult res = vkEndCommandBuffer(m_cmdBufs[i]);
            CHECK_VK_RESULT(res, "vkEndCommandBuffer\n");
        }

        printf("Command buffers recorded\n");
    }


    void ProcessCameraInput(float dt)
    {
        if (!m_isCameraActive) return;
        using CM = Morph::CameraMovement;
        if (m_keys[GLFW_KEY_W]) m_camera.ProcessKeyboard(CM::Forward,  dt);
        if (m_keys[GLFW_KEY_S]) m_camera.ProcessKeyboard(CM::Backward, dt);
        if (m_keys[GLFW_KEY_A]) m_camera.ProcessKeyboard(CM::Left,     dt);
        if (m_keys[GLFW_KEY_D]) m_camera.ProcessKeyboard(CM::Right,    dt);
        if (m_keys[GLFW_KEY_E]) m_camera.ProcessKeyboard(CM::Up,       dt);
        if (m_keys[GLFW_KEY_Q]) m_camera.ProcessKeyboard(CM::Down,     dt);
    }

    void UpdateUniformBuffers(uint32_t ImageIndex, float dt)
    {
        static float angle = 0.f;
        angle += 1.0f * dt;

        glm::mat4 World = glm::rotate(glm::mat4(1.f), angle, glm::vec3(0.f, 1.f, 0.f));

        glm::mat4 View = m_camera.GetViewMatrix();
        glm::mat4 Proj = m_camera.GetProjectionMatrix();

        UniformData ubo;
        ubo.WVP = Proj * View * World;

        m_uniformBuffers[ImageIndex].Update(m_vkCore.GetDevice(), &ubo, sizeof(ubo));
    }

    MorphVK::VulkanCore              m_vkCore;
    MorphVK::VulkanQueue*            m_pQueue     = NULL;
    int                              m_numImages  = 0;
    std::vector<VkCommandBuffer>     m_cmdBufs;
    VkRenderPass                     m_renderPass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer>       m_frameBuffers;
    VkShaderModule                   m_vs         = VK_NULL_HANDLE;
    VkShaderModule                   m_fs         = VK_NULL_HANDLE;
    MorphVK::GraphicsPipeline*       m_pPipeline  = NULL;
    MorphVK::SimpleMesh              m_mesh;
    std::vector<MorphVK::BufferAndMemory> m_uniformBuffers;
    GLFWwindow*                      m_pWindow    = NULL;
    int                              m_windowWidth  = 0;
    int                              m_windowHeight = 0;

    Morph::Camera m_camera;

    bool m_keys[512] = {};

    bool m_isCameraActive = false;
    float m_lastMouseX = 0.f;
    float m_lastMouseY = 0.f;
    bool  m_firstMouse = true;
};

int main(int argc, char* argv[])
{
    VulkanApp App(WINDOW_WIDTH, WINDOW_HEIGHT);
    App.Init(APP_NAME);
    App.Execute();
    return 0;
}