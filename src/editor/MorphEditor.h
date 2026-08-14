#pragma once

#include "MorphLog.h"
#include "MorphTypes.h"
#include "MorphMath.h"

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
} MorphEditor;