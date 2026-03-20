#include "asset.h"
#include "log.h"

#include <SDL3_image/SDL_image.h>

String next_word(String source, int& offset)
{
    offset += string_match_character(source, offset, ' ');
    String word = string_slice_to_character(source, offset, ' ');
    offset += word.size;
    return word;
}

#define ASSET_LINE_IS_VALID              BIT(0)
#define ASSET_LINE_IS_COMMENT            BIT(1)
#define ASSET_LINE_IS_OPTIONAL           BIT(2)
#define ASSET_LINE_HAS_TRAILING_TOKENS   BIT(3)

using AssetParseLineResult = u32;

// return false on failure or comment
// <kind> <scope optional> <name> <path> optional
AssetParseLineResult asset_parse_line(String line, Asset& pointer)
{
#if LOG_VERBOSE
    SCOPE_STRING(line, cstr);
    log_info("Asset entry: %s\n", cstr);
#endif

    u32 result = 0;

    line.trim();

    if (string_starts_with(line, make_string("#")))
    {
        return ASSET_LINE_IS_COMMENT;
    }

    String image = make_string("image");
    String audio = make_string("audio");
    String shader = make_string("shader");

    int cursor = 0;
    String kind = string_slice_to_character(line, 0, ' ');

    AssetKind asset_kind;

    if (string_starts_with(line, image))
    {
        asset_kind = ASSET_KIND_IMAGE;
        cursor += image.size;
    }
    else if (string_starts_with(line, audio))
    {
        asset_kind = ASSET_KIND_AUDIO;
        cursor += audio.size;
    }
    else if (string_starts_with(line, shader))
    {
        asset_kind = ASSET_KIND_SHADER;
        cursor += shader.size;
    }
    else {
        return 0;
    }

    // space required after a field
    if (line.data[cursor] != ' ')
    {
        return 0;
    }

    bool is_folder = false;

    String name = {};
    String scope = next_word(line, cursor);
    if (string_compare(scope, make_string("file")))
    {
        is_folder = false;
        name = next_word(line, cursor);
    }
    else if (string_compare(scope, make_string("folder")))
    {
        is_folder = true;
        name = next_word(line, cursor);
    }
    else
    {
        // scope is an optional argument and it's missing
        is_folder = false;

        name = scope;
        scope = {};
    }

    String path = next_word(line, cursor);

    if (name.size == 0 || path.size == 0)
    {
        return 0;
    }

    String optional = next_word(line, cursor);
    if (string_compare(optional, make_string("optional")))
    {
        result |= ASSET_LINE_IS_OPTIONAL;
    }

    String trail = string_slice_to_character(line, cursor, '\n');
    trail.trim();
    if (trail.size != 0)
    {
        result |= ASSET_LINE_HAS_TRAILING_TOKENS;
    }

    pointer.kind = asset_kind;
    pointer.is_folder = is_folder;
    pointer.name = name;
    pointer.path = path;
    pointer.identifier = NullAssetId;

    result |= ASSET_LINE_IS_VALID;
    return result;
}

bool parse_assets(const char* description, AssetCatalog& catalog)
{
    String_Builder file_contents;
    bool success = load_file_text(description, file_contents);
    if (!success)
    {
        return false;
    }

    int cursor = 0;
    int line_number = 0;

    String file = file_contents.to_string();
    while (cursor < file.size)
    {
        String line = string_slice_to_character(file, cursor, '\n');
        int next_line_offset = cursor + line.size;
        next_line_offset += string_match_character(file, next_line_offset, '\n');

        Asset asset = {};
        auto result = asset_parse_line(line, asset);

        line_number += 1;

        if (result & ASSET_LINE_IS_VALID)
        {
            catalog.add_asset(asset);

            if (result & ASSET_LINE_HAS_TRAILING_TOKENS)
            {
                log_warning("Asset description file %s has trailing tokens on line %d\n", description, line_number);
            }
        }
        else
        {
            if (result & ASSET_LINE_IS_COMMENT)
            {
                // do nothing
            }
            else if (!(result & ASSET_LINE_IS_OPTIONAL))
            {
                SCOPE_STRING(line, line_cstr);
                log_error("Could not parse line %d in %s asset description", line_number, description);
                log_info("Line: %s", line_cstr);
                return false;
            }
        }

        cursor = next_line_offset;
    }

    return true;
}

bool load_asset(Asset& asset, AssetLoadContext& load_context);

AssetId get_asset(const char* name_cstr, AssetCatalog& catalog)
{
    // @todo maybe this is slow
    String name = make_string(name_cstr);
    for (auto& asset : catalog.assets)
    {
        if (string_compare(asset.name, name))
        {
            if (asset.identifier.valid())
            {
                return asset.identifier;
            }
            else
            {
                bool load = load_asset(asset, catalog.load_context);
                if (!load)
                    return NullAssetId;

                return asset.identifier;
            }
        }
    }

    return NullAssetId;
}

bool load_asset(Asset& asset, AssetLoadContext& load_context)
{
    SCOPE_STRING(asset.path, path);

    switch (asset.kind)
    {
        case ASSET_KIND_IMAGE: {
            SDL_Texture* texture = IMG_LoadTexture(load_context.renderer, path);
            if (!texture)
            {
                asset.identifier.id = -1;
                return false;
            }

            asset.data.image.texture = texture;
            asset.identifier.generation += 1;

            return true;
        }
        case ASSET_KIND_AUDIO: {
            MIX_Audio* audio = load_context.audio_player->load_audio(path);
            if (!audio)
            {
                asset.identifier.id = -1;
                return false;
            }

            asset.data.audio.audio = audio;
            asset.identifier.generation += 1;

            return true;
        }
        case ASSET_KIND_FONT: {
            bool success = load_font_file(&asset.data.font.font, path, asset.data.font.font.size);
            if (!success)
            {
                asset.identifier.id = -1;
                return false;
            }

            asset.identifier.generation += 1;
            return true;
        }
        case ASSET_KIND_SHADER: {
            log_warning("Shader loading or support for shaders not implemented but trying to load shader %s", path);
            // fallthrough
        }
        default: {
            asset.identifier.id = -1;
            return false;
        }
    }
}
