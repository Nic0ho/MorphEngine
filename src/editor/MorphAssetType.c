#include "MorphAssetType.h"
#include "MorphSerializer.h"
#include "MorphLog.h"
#include <string.h>

AssetType morphClassifyAsset(const char *assetFilepath)
{
    const char* extension = strrchr(assetFilepath, '.');
    if (extension == NULL) return ASSET_UNKNOWN;

    if (strcmp(extension, ".png") == 0)
        return ASSET_TEXTURE;
    else if (strcmp(extension, ".mrph") == 0)
        return morphPeekAssetType(assetFilepath);
    else
        return ASSET_UNKNOWN;
}

AssetType morphPeekAssetType(const char *assetFilepath)
{
    MorphFile file = morphFileOpenRead(assetFilepath);
    if (!file.isValid)
        return ASSET_UNKNOWN;

    MorphAssetHeader header = {0};

    if (!morphFileRead(&file, &header, sizeof(MorphAssetHeader), 1) ||
        header.magic != MORPH_MAGIC)
    {
        morphLog(LOG_ERROR, "Failed to peek the asset type from %s!", assetFilepath);
        morphFileClose(&file);
        return ASSET_UNKNOWN;
    }

    morphFileClose(&file);
    return header.assetType;
}