#include "MorphScene.h"
#include "MorphAssetType.h"
#include "MorphLog.h"
#include "MorphMath.h"
#include "MorphSerializer.h"

bool morphSceneSave(MorphScene* scene, const char* filepath)
{
    MorphFile file = morphFileOpenWrite(filepath);
    if (!file.isValid)
        return false;

    MorphSceneHeader header = {0};
    header.assetHeader.magic = MORPH_MAGIC;
    header.assetHeader.assetType = ASSET_SCENE;
    header.version = MORPH_SCENE_VERSION;
    header.entityCount = scene->entitiesCount;

    if (!morphFileWrite(&file, &header, sizeof(MorphSceneHeader), 1)                                     ||
        !morphFileWrite(&file, scene->entitiesType, sizeof(EntityType), scene->entitiesCount)            ||
        !morphFileWrite(&file, scene->entitiesPosition, sizeof(Vec2), scene->entitiesCount)              ||
        !morphFileWrite(&file, scene->entitiesRotation, sizeof(f32), scene->entitiesCount)               ||
        !morphFileWrite(&file, scene->entitiesSize, sizeof(Vec2), scene->entitiesCount)                  ||
        !morphFileWrite(&file, scene->entitiesVelocity, sizeof(Vec2), scene->entitiesCount)              ||
        !morphFileWrite(&file, scene->entitiesSpriteID, sizeof(u32), scene->entitiesCount)               ||
        !morphFileWrite(&file, scene->entitiesFlags, sizeof(ComponentFlags), scene->entitiesCount)       ||
        !morphFileWrite(&file, scene->entitiesGeneration, sizeof(u32), scene->entitiesCount))
    {
        morphLog(LOG_ERROR, "Error during saving scene to: %s!", filepath);
        morphFileClose(&file);
        return false;
    }

    morphFileClose(&file);

    morphLog(LOG_MESSAGE, "Scene saved successfully to %s", filepath);
    return true;
}

bool morphSceneLoad(MorphScene* scene, const char* filepath)
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

    if (!morphFileRead(&file, &scene->entitiesType, sizeof(EntityType), header.entityCount)            ||
        !morphFileRead(&file, &scene->entitiesPosition, sizeof(Vec2), header.entityCount)              ||
        !morphFileRead(&file, &scene->entitiesRotation, sizeof(f32), header.entityCount)               ||
        !morphFileRead(&file, &scene->entitiesSize, sizeof(Vec2), header.entityCount)                  ||
        !morphFileRead(&file, &scene->entitiesVelocity, sizeof(Vec2), header.entityCount)              ||
        !morphFileRead(&file, &scene->entitiesSpriteID, sizeof(u32), header.entityCount)               ||
        !morphFileRead(&file, &scene->entitiesFlags, sizeof(ComponentFlags), header.entityCount) ||
        !morphFileRead(&file, &scene->entitiesGeneration, sizeof(u32), header.entityCount))
    {
        morphLog(LOG_ERROR, "Failed to load scene from %s!", filepath);
        morphFileClose(&file);
        return false;
    }

    scene->entitiesCount = header.entityCount;

    morphFileClose(&file);

    morphLog(LOG_MESSAGE, "Scene loaded successfully from %s", filepath);
    return true;
}

EntityHandle morphSceneSpawnEntity(MorphScene* scene, EntityType type, Vec2 position, Vec2 size)
{
    if (scene->entitiesCount == MAX_ENTITIES)
    {
        morphLog(LOG_ERROR, "Can not spawn entity. Max limit reached");
        return (EntityHandle){ .index = MAX_ENTITIES, .generation = 0 };
    }

    u32 claimedIndex = scene->entitiesCount++;

    scene->entitiesType[claimedIndex] = type;
    scene->entitiesPosition[claimedIndex] = position;
    scene->entitiesSize[claimedIndex] = size;

    switch (type)
    {
        case ENTITY_BLOCK:
            scene->entitiesFlags[claimedIndex] = COMPONENT_POSITION | COMPONENT_TEXTURE | COMPONENT_SIZE;
            break;
        
        case ENTITY_PLAYER:
            scene->entitiesFlags[claimedIndex] = COMPONENT_POSITION | COMPONENT_VELOCITY | COMPONENT_TEXTURE | COMPONENT_SIZE;
            break;
        
        case ENTITY_PARTICLE:
            scene->entitiesFlags[claimedIndex] = COMPONENT_POSITION | COMPONENT_VELOCITY;
            break;
        
        default:
            break;
    }

    scene->entitiesGeneration[claimedIndex]++;

    return (EntityHandle){ .index = claimedIndex, .generation = scene->entitiesGeneration[claimedIndex] };
}

bool morphSceneRemoveEntity(MorphScene* scene, EntityHandle handle)
{
    if (handle.generation != scene->entitiesGeneration[handle.index])
        return false;

    scene->entitiesFlags[handle.index] = scene->entitiesFlags[scene->entitiesCount - 1];
    scene->entitiesPosition[handle.index] = scene->entitiesPosition[scene->entitiesCount - 1];
    scene->entitiesRotation[handle.index] = scene->entitiesRotation[scene->entitiesCount - 1];
    scene->entitiesSize[handle.index] = scene->entitiesSize[scene->entitiesCount - 1];
    scene->entitiesVelocity[handle.index] = scene->entitiesVelocity[scene->entitiesCount - 1];
    scene->entitiesType[handle.index] = scene->entitiesType[scene->entitiesCount - 1];
    scene->entitiesSpriteID[handle.index] = scene->entitiesSpriteID[scene->entitiesCount - 1];

    scene->entitiesGeneration[handle.index]++;
    scene->entitiesCount--;

    return true;    
}

void morphSceneUpdateMovement(MorphScene* scene, f32 deltaTime)
{
    for (u32 i = 0; i < scene->entitiesCount; i++)
    {
        if (scene->entitiesFlags[i] & COMPONENT_VELOCITY)
        {
            scene->entitiesPosition[i] = vec2Add(scene->entitiesPosition[i], vec2Scale(scene->entitiesVelocity[i], deltaTime));
        }
    }

    return;
}