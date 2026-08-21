#include "MorphScene.h"
#include "MorphAssetType.h"
#include "MorphLog.h"
#include "MorphMath.h"
#include "MorphSerializer.h"

bool morphSceneSave(Entities *scene, const char *filepath)
{
    MorphFile file = morphFileOpenWrite(filepath);
    if (!file.isValid)
        return false;

    MorphSceneHeader header = {0};
    header.assetHeader.magic = MORPH_MAGIC;
    header.assetHeader.assetType = ASSET_SCENE;
    header.version = MORPH_SCENE_VERSION;
    header.entityCount = scene->count;

    if (!morphFileWrite(&file, &header, sizeof(MorphSceneHeader), 1)                     ||
        !morphFileWrite(&file, scene->type, sizeof(EntityType), scene->count)            ||
        !morphFileWrite(&file, scene->position, sizeof(Vec2), scene->count)              ||
        !morphFileWrite(&file, scene->rotation, sizeof(f32), scene->count)               ||
        !morphFileWrite(&file, scene->size, sizeof(Vec2), scene->count)                  ||
        !morphFileWrite(&file, scene->velocity, sizeof(Vec2), scene->count)              ||
        !morphFileWrite(&file, scene->spriteID, sizeof(u32), scene->count)               ||
        !morphFileWrite(&file, scene->entityFlags, sizeof(ComponentFlags), scene->count) ||
        !morphFileWrite(&file, scene->generation, sizeof(u32), scene->count))
    {
        morphLog(LOG_ERROR, "Error during saving scene to: %s!", filepath);
        morphFileClose(&file);
        return false;
    }

    morphFileClose(&file);

    morphLog(LOG_MESSAGE, "Scene saved successfully to %s", filepath);
    return true;
}

bool morphSceneLoad(Entities *scene, const char *filepath)
{
    MorphFile file = morphFileOpenRead(filepath);
    if (!file.isValid)
        return false;

    MorphSceneHeader header = {0};
    
    if (!morphFileRead(&file, &header, sizeof(MorphSceneHeader), 1) ||
        header.assetHeader.magic != MORPH_MAGIC                           ||
        header.version != MORPH_SCENE_VERSION)
        
    {
        morphLog(LOG_ERROR, "Failed to load scene from %s! (magic/version missmatch)", filepath);
        morphFileClose(&file);
        return false;
    }

    if (!morphFileRead(&file, &scene->type, sizeof(EntityType), header.entityCount)            ||
        !morphFileRead(&file, &scene->position, sizeof(Vec2), header.entityCount)              ||
        !morphFileRead(&file, &scene->rotation, sizeof(f32), header.entityCount)               ||
        !morphFileRead(&file, &scene->size, sizeof(Vec2), header.entityCount)                  ||
        !morphFileRead(&file, &scene->velocity, sizeof(Vec2), header.entityCount)              ||
        !morphFileRead(&file, &scene->spriteID, sizeof(u32), header.entityCount)               ||
        !morphFileRead(&file, &scene->entityFlags, sizeof(ComponentFlags), header.entityCount) ||
        !morphFileRead(&file, &scene->generation, sizeof(u32), header.entityCount))
    {
        morphLog(LOG_ERROR, "Failed to load scene from %s!", filepath);
        morphFileClose(&file);
        return false;
    }

    scene->count = header.entityCount;

    morphFileClose(&file);

    morphLog(LOG_MESSAGE, "Scene loaded successfully from %s", filepath);
    return true;
}

EntityHandle morphSceneSpawnEntity(Entities* scene, EntityType type, Vec2 position, Vec2 size)
{
    if (scene->count == MAX_ENTITIES)
    {
        morphLog(LOG_ERROR, "Can not spawn entity. Max limit reached");
        return (EntityHandle){ .index = MAX_ENTITIES, .generation = 0 };
    }

    u32 claimedIndex = scene->count++;

    scene->type[claimedIndex] = type;
    scene->position[claimedIndex] = position;
    scene->size[claimedIndex] = size;

    switch (type)
    {
        case ENTITY_BLOCK:
            scene->entityFlags[claimedIndex] = COMPONENT_POSITION | COMPONENT_TEXTURE | COMPONENT_SIZE;
            break;
        
        case ENTITY_PLAYER:
            scene->entityFlags[claimedIndex] = COMPONENT_POSITION | COMPONENT_VELOCITY | COMPONENT_TEXTURE | COMPONENT_SIZE;
            break;
        
        case ENTITY_PARTICLE:
            scene->entityFlags[claimedIndex] = COMPONENT_POSITION | COMPONENT_VELOCITY;
            break;
        
        default:
            break;
    }

    scene->generation[claimedIndex]++;

    return (EntityHandle){ .index = claimedIndex, .generation = scene->generation[claimedIndex] };
}

void morphSceneUpdateMovement(Entities* scene, f32 deltaTime)
{
    for (u32 i = 0; i < scene->count; i++)
    {
        if (scene->entityFlags[i] & COMPONENT_VELOCITY)
        {
            scene->position[i] = vec2Add(scene->position[i], vec2Scale(scene->velocity[i], deltaTime));
        }
    }

    return;
}