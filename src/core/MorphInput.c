#include "MorphInput.h"
#include "GLFW/glfw3.h"

#include <string.h>

void morphInputUpdate(MorphInput* input, GLFWwindow* window)
{
    memcpy(input->downLastFrame, input->downNow, sizeof(input->downNow));

    for (u32 i = 0; i < LEN(input->downNow); i++)
        input->downNow[i] = (glfwGetKey(window, i) == GLFW_PRESS);

    return;
}

void morphScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    MorphInput* input = (MorphInput*)glfwGetWindowUserPointer(window);

    input->scrollDelta += (f32)yoffset;

    return;
}

bool morphInputIsKeyDown(MorphInput* input, u32 key)
{ return input->downNow[key]; }

bool morphInputIsKeyPressed(MorphInput* input, u32 key)
{ return (!input->downLastFrame[key] && input->downNow[key]); }

bool morphInputIsKeyReleased(MorphInput* input, u32 key)
{ return (input->downLastFrame[key] && !input->downNow[key]); }