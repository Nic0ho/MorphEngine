#pragma once

#include "MorphSerializer.h"
#include "MorphTypes.h"
#include "MorphMath.h"
#include "MorphBuffer.h"

#define MORPH_SCENE_VERSION 1

#define MAX_ENTITIES 200

typedef enum
{
    COMPONENT_POSITION = 1 << 0,
    COMPONENT_VELOCITY = 1 << 1,
    COMPONENT_TEXTURE  = 1 << 2,
    COMPONENT_SIZE = 1 << 3,
} ComponentFlags;

typedef enum
{
    ENTITY_PLAYER,
    ENTITY_BLOCK,
    ENTITY_PARTICLE,
} EntityType;

typedef struct
{
    MorphAssetHeader assetHeader;
    u32 version;
    u32 entityCount;
} MorphSceneHeader;

typedef struct
{
    u32 index;
    u32 generation;
} EntityHandle;

typedef struct
{
    u32            count;
    EntityType     type[MAX_ENTITIES];
    Vec2           position[MAX_ENTITIES];
    f32            rotation[MAX_ENTITIES];
    Vec2           size[MAX_ENTITIES];
    Vec2           velocity[MAX_ENTITIES];
    u32            spriteID[MAX_ENTITIES];
    ComponentFlags entityFlags[MAX_ENTITIES];
    u32            generation[MAX_ENTITIES];
} Entities;

bool morphSceneSave(Entities* scene, const char* filepath);
bool morphSceneLoad(Entities* scene, const char* filepath);

EntityHandle morphSceneSpawnEntity(Entities* scene, EntityType type, Vec2 position, Vec2 size);

void morphSceneUpdateMovement(Entities* scene, f32 deltaTime);