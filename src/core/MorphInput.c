#include "MorphInput.h"
#include "GLFW/glfw3.h"

#include <string.h>

void morphInputUpdate(MorphInput* input, GLFWwindow* window)
{
    memcpy(input->downLastFrame, input->downNow, sizeof(input->downNow));
    memcpy(input->mouseDownLastFrame, input->mouseDownNow, sizeof(input->mouseDownNow));

    for (u32 i = 0; i < LEN(input->downNow); i++)
        input->downNow[i] = (glfwGetKey(window, i) == GLFW_PRESS);

    for (u32 i = 0; i < MAX_MOUSE_BUTTONS; i++)
        input->mouseDownNow[i] = (glfwGetMouseButton(window, i) == GLFW_PRESS);

    f64 nMousePosX;
    f64 nMousePosY;
    glfwGetCursorPos(window, &nMousePosX, &nMousePosY);

    static bool firstFrame = true;
    if (firstFrame)
    {
        input->mouseX = nMousePosX;
        input->mouseY = nMousePosY;
        firstFrame = false;
    }

    input->mouseDeltaX = nMousePosX - input->mouseX;
    input->mouseDeltaY = nMousePosY - input->mouseY;

    input->mouseX = nMousePosX;
    input->mouseY = nMousePosY;
}

void morphScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    MorphInput* input = (MorphInput*)glfwGetWindowUserPointer(window);

    input->scrollDelta += (f32)yoffset;
}

bool morphInputIsKeyDown(MorphInput* input, u32 key)
{ return input->downNow[key]; }

bool morphInputIsKeyPressed(MorphInput* input, u32 key)
{ return (!input->downLastFrame[key] && input->downNow[key]); }

bool morphInputIsKeyReleased(MorphInput* input, u32 key)
{ return (input->downLastFrame[key] && !input->downNow[key]); }

bool morphInputIsMouseButtonDown(MorphInput* input, u8 button)
{ return input->mouseDownNow[button]; }