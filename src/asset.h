#ifndef _ASSET_H
#define _ASSET_H

#include "common.h"
#include "template.h"

#include "ui.h"
#include "audio.h"
#include "draw.h"

enum AssetKind {
    ASSET_KIND_IMAGE,
    ASSET_KIND_AUDIO,
    ASSET_KIND_FONT,
    ASSET_KIND_SHADER,
    ASSET_KIND_COUNT,
};

struct AssetId {
    int id;
    int generation;

    bool valid() const { return id != -1 || generation != 0; }
};

static const AssetId NullAssetId = AssetId {-1, 0};

struct AssetLoadContext {
    SDL_Renderer* renderer;
    // other relevant stuff as the codebase evolves
};

struct Asset {
    AssetKind kind;
    String name;
    String path;
    bool is_folder = false;

    AssetId identifier;
    union {
        Font font;
        Texture texture;  // for image
        AudioData audio;
    } data;
};

struct AssetCatalog {
    String_Builder builder;
    AssetLoadContext load_context;
    DArray<Asset> assets;

    void add_asset(Asset& asset)
    {
        int index = assets.add(asset);
        assets.get_ref(index).identifier.id = index;
        assets.get_ref(index).identifier.generation = 0;
    }

    const Texture* get_texture(AssetId id)
    {
        if (!id.valid())
        {
            return nullptr;
        }

        const Asset& asset = assets.get_ref(id.id);
        if (asset.kind != ASSET_KIND_IMAGE || asset.identifier.generation != id.generation)
        {
            return nullptr;
        }

        return &asset.data.texture;
    }

    const Font* get_font(AssetId id)
    {
        if (!id.valid())
        {
            return nullptr;
        }

        const Asset& asset = assets.get_ref(id.id);
        if (asset.kind != ASSET_KIND_FONT || asset.identifier.generation != id.generation)
        {
            return nullptr;
        }

        return &asset.data.font;
    }

    const AudioData* get_audio(AssetId id)
    {
        if (!id.valid())
        {
            return nullptr;
        }

        const Asset& asset = assets.get_ref(id.id);
        if (asset.kind != ASSET_KIND_AUDIO || asset.identifier.generation != id.generation)
        {
            return nullptr;
        }

        return &asset.data.audio;
    }
};

bool parse_assets(const char* description, AssetCatalog& catalog);
AssetId get_asset(const char* name, AssetCatalog& catalog);

#endif // _ASSET_H