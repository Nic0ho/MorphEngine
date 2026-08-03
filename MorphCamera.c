#include "MorphCamera.h"

Mat4 morphCameraGetViewProjection(MorphCamera* camera, f32 viewWidth, f32 viewHeight)
{
    f32 left = camera->position.x - (viewWidth / 2.0f);
    f32 right = camera->position.x + (viewWidth / 2.0f);
    f32 bottom = camera->position.y - (viewHeight / 2.0f);
    f32 top = camera->position.y + (viewHeight / 2.0f);

    return mat4Ortho(left, right, bottom, top, -1.0f, 1.0f);
}