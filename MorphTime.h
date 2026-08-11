#pragma once

#include "MorphTypes.h"

typedef struct
{
    f64 lastFrame;
    f64 deltaTime;
}  MorphTime;

void morphTimeUpdate(MorphTime* time);