#include <stdio.h>
#include <stdlib.h>

#include <GLFW/glfw3.h>

#define GLFW_INCLUDE_VULKAN
#include "MorphVulkanCore.h"


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
    }

    void Init(const char* pAppName, GLFWwindow* pWindow)
    {
        m_vkCore.Init(pAppName, pWindow);
        m_numImages = m_vkCore.GetNumImages();
        CreateCommandBuffers();
    }

    void RenderScene()
    {

    }

private:
    void CreateCommandBuffers()
    {
        m_cmdBufs.resize(m_numImages);
        m_vkCore.CreateCommandBuffers(m_numImages, &m_cmdBufs[0]);
    }

    MorphVK::VulkanCore m_vkCore;
    int m_numImages = 0;
    std::vector<VkCommandBuffer> m_cmdBufs;
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