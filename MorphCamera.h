#pragma once

#include "MorphTypes.h"
#include "MorphMath.h"

typedef struct
{
    Vec2 position;
} MorphCamera;

Mat4 morphCameraGetViewProjection(MorphCamera* camera, f32 viewWidth, f32 viewHeight);