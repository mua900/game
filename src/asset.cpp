#include "asset.h"

bool asset_parse_line(String line)
{
    SCOPE_STRING(line, cstr);
    printf("%s\n", cstr);

    Asset asset;

    line.trim();

    int cursor = 0;

    for (int i = 0; i < ASSET_KIND_COUNT; i++)
    {
        String image = make_string("image");
        String audio = make_string("audio");
        String shader = make_string("shader");

        if (string_starts_with(line, image))
        {
            asset.kind = ASSET_KIND_IMAGE;
            cursor += image.size;
        }
        else if (string_starts_with(line, audio))
        {
            asset.kind = ASSET_KIND_AUDIO;
            cursor += audio.size;
        }
        else if (string_starts_with(line, shader))
        {
            asset.kind = ASSET_KIND_SHADER;
            cursor += shader.size;
        }
        else {
            return false;
        }

        // space required after a field
        if (line.data[cursor] != ' ' && line.data[cursor] != '\t')
        {
            return false;
        }
    }

    // file or folder (or assume file if nothing is provided)

    return true;
}

bool parse_assets(const char* description, AssetCatalog& catalog)
{
    String_Builder file_contents;
    printf("%s\n", description);
    bool success = load_file_text(description, file_contents);
    if (!success)
    {
        return false;
    }

    int cursor = 0;
    while (cursor < file_contents.size)
    {
        int next_line = cursor;
        while (file_contents.buffer[next_line] != '\n') {
            next_line += 1;
        }

        asset_parse_line(file_contents.slice(cursor, next_line));

        cursor = next_line + 1;
    }

    return true;
}