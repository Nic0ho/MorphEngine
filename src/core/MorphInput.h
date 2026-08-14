#pragma once

#include "MorphTypes.h"
#include <GLFW/glfw3.h>

#define MAX_MOUSE_BUTTONS 8

typedef struct
{
    //keyboard
    bool downNow[GLFW_KEY_LAST + 1];
    bool downLastFrame[GLFW_KEY_LAST + 1];

    //mouse
    bool mouseDownNow[MAX_MOUSE_BUTTONS];
    bool mouseDownLastFrame[MAX_MOUSE_BUTTONS];
    f32 mouseX;
    f32 mouseY;
    f32 mouseDeltaX;
    f32 mouseDeltaY;
    f32 scrollDelta;
} MorphInput;

void morphInputUpdate(MorphInput* input, GLFWwindow* window);
void morphScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

bool morphInputIsKeyDown(MorphInput* input, u32 key);
bool morphInputIsKeyPressed(MorphInput* input, u32 key);
bool morphInputIsKeyReleased(MorphInput* input, u32 key);

bool morphInputIsMouseButtonDown(MorphInput* input, u8 button);