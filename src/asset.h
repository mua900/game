#ifndef _ASSET_H
#define _ASSET_H

#include "common.h"
#include "template.h"

enum AssetKind {
    ASSET_KIND_IMAGE,
    ASSET_KIND_AUDIO,
    ASSET_KIND_SHADER,
    ASSET_KIND_COUNT,
};

struct Asset {
    AssetKind kind;
    String name;
    bool ready;
};

struct AssetCatalog {
    DArray<Asset> assets;
};

bool parse_assets(const char* description, AssetCatalog& catalog);

#endif // _ASSET_H