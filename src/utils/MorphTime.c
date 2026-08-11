#include "MorphTime.h"

#include <GLFW/glfw3.h>

void morphTimeUpdate(MorphTime* time)
{
    f64 now = glfwGetTime();

    time->deltaTime = now - time->lastFrame;
    time->lastFrame = now;

    return;
}