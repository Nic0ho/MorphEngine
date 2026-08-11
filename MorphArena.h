#pragma once

#include "MorphTypes.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct
{
    u8* memory;
    usize size;
    usize curOffset;
} MorphArena;

u8* morphArenaAlloc(MorphArena* arena, usize bytes, usize align);
bool morphArenaCreate(MorphArena* arena, usize capacity);

void morphArenaDestroy(MorphArena* arena);
void morphArenaReset(MorphArena* arena);