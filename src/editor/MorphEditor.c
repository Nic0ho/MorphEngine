#include "MorphEditor.h"
#include "GLFW/glfw3.h"
#include "MorphInput.h"
#include "MorphLog.h"
#include <float.h>

void morphEditorUpdateInput(MorphEditor* editor, MorphInput* input, MorphCamera* editorCamera, f32 deltaTime)
{
    // BINDINGS --------------------

    //viewport
    if (editor->viewportCursorFocused)
    {
        //camera movement

        if (morphInputIsMouseButtonDown(input, GLFW_MOUSE_BUTTON_MIDDLE))
        {
            if (editor->lastViewportSize.x > 0.0f) 
            {
                f32 worldUnitsPerPixel = editorCamera->viewWidth / editor->lastViewportSize.x;

                editorCamera->position.x -= (f32)input->mouseDeltaX * worldUnitsPerPixel;
                editorCamera->position.y += (f32)input->mouseDeltaY * worldUnitsPerPixel; 
            }
        }

        if (input->scrollDelta != 0.0f)
        {
            editorCamera->viewWidth -= input->scrollDelta * editorCamera->zoomStrength;
            if (editorCamera->viewWidth < 0.5f)
                editorCamera->viewWidth = 0.5f;
        }
    }

    input->scrollDelta = 0;
}