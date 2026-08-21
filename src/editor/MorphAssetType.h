#pragma once

typedef enum
{
    ASSET_FOLDER,
    ASSET_TEXTURE,
    ASSET_SCENE,
    ASSET_ENTITY_PREFAB,
    ASSET_UNKNOWN,
    ASSET_COUNT,
} AssetType;

#ifdef __cplusplus
extern "C" {
#endif

AssetType morphClassifyAsset(const char* assetFilepath);
AssetType morphPeekAssetType(const char* assetFilepath);

#ifdef __cplusplus
}
#endif