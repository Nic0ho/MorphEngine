#pragma once

#include "MorphInput.h"
#include "MorphLog.h"
#include "MorphTypes.h"
#include "MorphMath.h"
#include "MorphCamera.h"

typedef struct
{
    MorphOutputConsoleBuffer output;

    //window visibility
    bool showOutput;
    bool showTools;
    bool showOutliner;
    bool showContentDrawer;
    bool showDetails;
    bool showViewport;

    //viewport
    Vec2 lastViewportSize;
    f32 resizeTimer;
    bool viewportNeedsResize;
    bool viewportCursorFocused;
} MorphEditor;

void morphEditorUpdateInput(MorphEditor* editor, MorphInput* input, MorphCamera* editorCamera, f32 deltaTime);