#include "game.h"

const char* game_object_type_name(GameObjectType type)
{
    switch (type)
    {
        case GOT_Wall:              return "Wall";
        case GOT_Ball:              return "Ball";
        case GOT_Player:            return "Player";
        case GOT_LaserEmitter:      return "LaserEmitter";
        case GOT_LaserCollector:    return "LaserCollector";
        case GOT_Mirror:            return "Mirror";
        case GOT_LaserReflector:    return "LaserReflector";
        case GOT_WavelengthShifter: return "WavelengthShifter";
        case GOT_LaserSplitter:     return "LaserSplitter";
        case GOT_EnergyGate:        return "EnergyGate";
        case GOT_EnergySource:      return "EnergySource";
        default:                    return "--Unknown--";
    }
}

GameObjectType get_game_object_type_from_name(String name)
{
    if (name == String("Wall"))                   return GOT_Wall;
    else if (name == String("Ball"))              return GOT_Ball;
    else if (name == String("Player"))            return GOT_Player;
    else if (name == String("LaserEmitter"))      return GOT_LaserEmitter;
    else if (name == String("LaserCollector"))    return GOT_LaserCollector;
    else if (name == String("Mirror"))            return GOT_Mirror;
    else if (name == String("LaserReflector"))    return GOT_LaserReflector;
    else if (name == String("WavelengthShifter")) return GOT_WavelengthShifter;
    else if (name == String("LaserSplitter"))     return GOT_LaserSplitter;
    else if (name == String("EnergyGate"))        return GOT_EnergyGate;
    else if (name == String("EnergySource"))      return GOT_EnergySource;
    else if (name == String("--Unknown--"))       return GOT_Sentinel;  // we know it's written by us
    else                                          return GOT_Sentinel;  // random garbage
}

static const int Version_Number = 0;
static const char* Magic = "LB";

static const int Game_Object_Chunk = 0;

bool serialize_game_state(GameState* state, File& file)
{
    if (!file.handle) return false;

    file.write_byte(Magic[0]);
    file.write_byte(Magic[1]);
    file.write_integer(Version_Number);

    int count = state->game_objects.count();
    file.write_integer(Game_Object_Chunk);
    file.write_integer(count);

    int index = 0;
    for (const GameObject& object : state->game_objects)
    {
        const char* name = game_object_type_name(object.type);
        vec2 position = get_object_position(object);

        file.write_number(index);

        file.write_string(String(name));
        file.write_number(position.x);
        file.write_number(position.y);

        switch (object.type)
        {
            case GOT_Wall:              { break; }
            case GOT_Ball:              { break; }
            case GOT_Player:            { break; }
            case GOT_LaserEmitter:      { break; }
            case GOT_LaserCollector:    { break; }
            case GOT_Mirror:            { break; }
            case GOT_LaserReflector:    { break; }
            case GOT_WavelengthShifter: { break; }
            case GOT_LaserSplitter:     { break; }
            case GOT_EnergyGate:        { break; }
            case GOT_EnergySource:      { break; }
        }

        index += 1;
    }

    return true;
}

bool read_game_state(GameState* state, File& file)
{
    if (!file.handle) return false;

    int magic_high = file.read_byte();
    int magic_low  = file.read_byte();
    if (magic_high != Magic[0] || magic_low != Magic[1])
        return false;

    u64 version_number = file.read_integer();
    if (version_number != Version_Number)
        return false;

    u64 chunck = file.read_integer();
    if (chunck != Game_Object_Chunk)
    {
        return false;
    }

    u64 object_count = file.read_integer();

    // doesn't make sense
    if (object_count > 1e3)
    {
        return false;
    }

    BucketList<GameObject> objects;

    for (int i = 0; i < object_count; i++)
    {
        u64 index = file.read_integer();
        if (index != i)
        {
            return false;
        }

        String object_type_name = file.read_string();
        GameObjectType type = get_game_object_type_from_name(object_type_name);
        if (type == GOT_Sentinel)
        {
            return false;
        }

        GameObject game_object(type);

        float xcoord = file.read_number();
        float ycoord = file.read_number();
        if (!(std::isfinite(xcoord) && std::isfinite(ycoord)))
        {
            return false;
        }

        set_object_position(game_object, vec2(xcoord, ycoord));

        switch (type)
        {
            case GOT_Wall:              { break; }
            case GOT_Ball:              { break; }
            case GOT_Player:            { break; }
            case GOT_LaserEmitter:      { break; }
            case GOT_LaserCollector:    { break; }
            case GOT_Mirror:            { break; }
            case GOT_LaserReflector:    { break; }
            case GOT_WavelengthShifter: { break; }
            case GOT_LaserSplitter:     { break; }
            case GOT_EnergyGate:        { break; }
            case GOT_EnergySource:      { break; }
        }

        objects.add(game_object);
    }

    state->initialize();
    state->game_objects = std::move(objects);

    return true;
}
