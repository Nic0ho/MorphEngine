#include "MorphArena.h"

u8* morphArenaAlloc(MorphArena* arena, usize bytes, usize align)
{
    usize alignedOffset = ((arena->curOffset + (align-1)) / align) * align;

    usize newOffset = alignedOffset + bytes;

    if (newOffset > arena->size)
    {
        return NULL;
    }

    u8* result = arena->memory + alignedOffset;
    arena->curOffset = newOffset;

    return result;
}

bool morphArenaCreate(MorphArena* arena, usize capacity)
{
    arena->memory = malloc(capacity);

    if (arena->memory == NULL)
    {
        printf("[ALLOCATION ERROR] Failed to allocate arena");
        return false;
    }

    arena->size = capacity;
    arena->curOffset = 0;

    return true;
}

void morphArenaDestroy(MorphArena* arena)
{
    if (arena == NULL)
    {
        printf("[ALLOCATION ERROR] Arena equals NULL");
        return;
    }

    free(arena->memory);

    arena->memory = NULL;
    arena->size = 0;
    arena->curOffset = 0;
    
    return;
}

void morphArenaReset(MorphArena* arena)
{
    arena->curOffset = 0;
    return;
}