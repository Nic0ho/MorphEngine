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
    Vec4 bgColor;
} MorphSceneSettings;

typedef struct
{
    MorphSceneSettings settings;
    u32                entitiesCount;
    EntityType         entitiesType[MAX_ENTITIES];
    Vec2               entitiesPosition[MAX_ENTITIES];
    f32                entitiesRotation[MAX_ENTITIES];
    Vec2               entitiesSize[MAX_ENTITIES];
    Vec2               entitiesVelocity[MAX_ENTITIES];
    u32                entitiesSpriteID[MAX_ENTITIES];
    ComponentFlags     entitiesFlags[MAX_ENTITIES];
    u32                entitiesGeneration[MAX_ENTITIES];
} MorphScene;

bool morphSceneSave(MorphScene* scene, const char* filepath);
bool morphSceneLoad(MorphScene* scene, const char* filepath);

EntityHandle morphSceneSpawnEntity(MorphScene* scene, EntityType type, Vec2 position, Vec2 size);
bool morphSceneRemoveEntity(MorphScene* scene, EntityHandle handle);

void morphSceneUpdateMovement(MorphScene* scene, f32 deltaTime);