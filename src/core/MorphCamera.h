#pragma once

#include "MorphTypes.h"
#include "MorphMath.h"

typedef struct
{
    Vec2 position;
    f32  viewWidth;
    f32  sensitivity;
    f32  zoomStrength;
} MorphCamera;

Mat4 morphCameraGetViewProjection(MorphCamera* camera, f32 aspectRatio);