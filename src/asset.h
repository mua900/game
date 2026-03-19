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
    String path;
    bool ready = false;
    bool is_folder = false;

    void* asset_data;
};

struct AssetId {
    int id;
    int generation;

    bool valid() const { return id != -1; }
};

struct AssetCatalog {
    String_Builder builder;
    DArray<Asset> assets;
};

bool parse_assets(const char* description, AssetCatalog& catalog);
AssetId load_asset(const char* name, AssetCatalog& catalog);

#endif // _ASSET_H