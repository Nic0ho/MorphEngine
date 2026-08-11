#include "MorphAtlas.h"
#include "MorphBuffer.h"
#include "MorphLog.h"
#include "stb_image.h"
#include <stdlib.h>
#include <string.h>

u32 morphAtlasAddSprite(MorphAtlas* atlas, const char* path)
{
    if (atlas->count >= MAX_SPRITES)
    {
        morphLog(LOG_ERROR, "Can not load more sprites in the atlas");
        return MAX_SPRITES;
    }

    u32 id = atlas->count++;
    atlas->path[id] = path;

    return id;
}

bool morphAtlasBuild(MorphAtlas* atlas, VkDevice logicalDevice, VkPhysicalDevice physicalDevice, VkCommandPool pool, VkQueue queue)
{
    u8* pixelData[MAX_SPRITES] = {0};
    u32 width[MAX_SPRITES] = {0};
    u32 height[MAX_SPRITES] = {0};

    u32 totalWidth = 0;
    u32 maxHeight = 0;

    for (u32 i = 0; i < atlas->count; i++)
    {
        int w, h, ch;
        pixelData[i] = stbi_load(atlas->path[i], &w, &h, &ch, STBI_rgb_alpha);
        if (!pixelData[i])
        {
            morphLog(LOG_ERROR, "(Atlas)Failed to load %s", atlas->path[i]);
            for (u32 j = 0; j < i; j ++)
                stbi_image_free(pixelData[j]);
            return false;
        }

        width[i] = (u32)w;
        height[i] = (u32)h;
        totalWidth += (u32)w;
        if ((u32)h > maxHeight) maxHeight = (u32)h;
    }

    u8* atlasBuf = calloc(totalWidth * maxHeight * 4, 1);
    if (!atlasBuf)
    {
        morphLog(LOG_ERROR, "Atlas is out of memory!");
        for (u32 i = 0; i < atlas->count; i++)
            stbi_image_free(pixelData[i]);
        return false;
    }

    u32 cursorX = 0;
    for (u32 i = 0; i < atlas->count; i++)
    {
        for (u32 row = 0; row < height[i]; row++)
        {
            u8* src = pixelData[i] + row * width[i] * 4;
            u8* dst = atlasBuf + (row * totalWidth + cursorX) * 4;
            memcpy(dst, src, width[i] * 4);
        }

        atlas->sprite[i].uvOffset = (Vec2)
        {
            (f32)cursorX / (f32)totalWidth,
            0.0f
        };
        atlas->sprite[i].uvScale = (Vec2)
        {
            (f32)width[i] / (f32)totalWidth,
            (f32)height[i] / (f32)maxHeight
        };

        cursorX += width[i];
        stbi_image_free(pixelData[i]);
    }

    bool result = morphTextureFromPixels(logicalDevice, physicalDevice, pool, queue, atlasBuf, totalWidth, maxHeight, &atlas->texture);
    free(atlasBuf);

    if (result)
    { morphLog(LOG_MESSAGE, "Atlas built: %ux%u, %u sprites", totalWidth, maxHeight, atlas->count); }    

    return result;
}

void morphAtlasDestroy(MorphAtlas* atlas, VkDevice device)
{
    morphTextureDestroy(device, &atlas->texture);
}