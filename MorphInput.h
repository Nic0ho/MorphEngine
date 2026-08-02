#pragma once

#include "MorphTypes.h"
#include <glfw/glfw3.h>

typedef struct
{
    bool downNow[GLFW_KEY_LAST + 1];
    bool downLastFrame[GLFW_KEY_LAST + 1];
} MorphInput;

void morphInputUpdate(MorphInput* input, GLFWwindow* window);

bool morphInputIsKeyDown(MorphInput* input, u32 key);
bool morphInputIsKeyPressed(MorphInput* input, u32 key);
bool morphInputIsKeyReleased(MorphInput* input, u32 key);