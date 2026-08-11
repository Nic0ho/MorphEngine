#include "MorphScene.h"
#include "MorphLog.h"
#include "MorphMath.h"

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