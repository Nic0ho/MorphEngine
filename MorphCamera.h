#pragma once

#include "MorphTypes.h"
#include "MorphMath.h"

typedef struct
{
    Vec2 position;
    f32 viewWidth;
} MorphCamera;

Mat4 morphCameraGetViewProjection(MorphCamera* camera, f32 aspectRatio);