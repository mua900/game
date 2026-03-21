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
    AudioPlayer* audio_player;
};

// resource types
// thin wrappers around more specific types

struct AudioResource {
    MIX_Audio* audio;
};

struct FontResource {
    Font font;
};

struct ImageResource {
    Texture texture;
};

struct ShaderResource {
    // @todo
};

struct Asset {
    AssetKind kind;
    String name;
    String path;
    bool is_folder = false;

    AssetId identifier;
    union {
        FontResource font;
        ImageResource image;
        AudioResource audio;
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

    const ImageResource* get_image(AssetId id)
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

        return &asset.data.image;
    }

    const FontResource* get_font(AssetId id)
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

    const AudioResource* get_audio(AssetId id)
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

// parse asset catalog file and add assets listed in it to the catalog
bool parse_asset_description(const char* description, AssetCatalog& catalog);
// load the file pointed to by path and call parse_asset_description on it and return it's result
bool parse_assets(const char* path, AssetCatalog& catalog);
// returns the existing handle if the asset is already loaded otherwise loads the asset on the fly and returns the handle
// returns null id if no asset with the given name is found or the asset load fails
AssetId get_asset(const char* name, AssetCatalog& catalog);

#endif // _ASSET_H