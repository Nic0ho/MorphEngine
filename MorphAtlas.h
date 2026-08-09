#pragma once

#include "MorphMath.h"
#include "MorphTypes.h"
#include "MorphBuffer.h"
#include <vulkan/vulkan.h>

#define MAX_SPRITES 64

typedef struct
{
    Vec2 uvOffset;
    Vec2 uvScale;
} SpriteRect;

typedef struct
{
    MorphTexture texture;
    SpriteRect   sprite[MAX_SPRITES];
    u32          count;
    const char*  path[MAX_SPRITES];
} MorphAtlas;

u32 morphAtlasAddSprite(MorphAtlas* atlas, const char* path);

bool morphAtlasBuild(MorphAtlas* atlas, VkDevice logicalDevice, VkPhysicalDevice physicalDevice, VkCommandPool pool, VkQueue queue);
void morphAtlasDestroy(MorphAtlas* atlas, VkDevice device);