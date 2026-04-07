#include "game.h"

const char* game_object_type_name(GameObjectType type);
GameObjectType get_game_object_type_from_name(String name);

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

        file.write_integer(index);

        file.write_string(String(name));
        BodyType body_type = object.transform.get_body_type();
        file.write_number(body_type);
        file.write_number(position.x);
        file.write_number(position.y);

        b2ShapeId shapes[8];
        u64 shape_count = b2Body_GetShapes(object.transform.body, shapes, 8);
        file.write_integer(shape_count);
        for (int i = 0; i < shape_count; i++)
        {
            ShapeType shape_type = (ShapeType) b2Shape_GetType(shapes[i]);
            file.write_integer((u64)shape_type);

            b2Filter filter = b2Shape_GetFilter(shapes[i]);

            file.write_integer(filter.categoryBits);
            file.write_integer(filter.maskBits);
            file.write_int(filter.groupIndex);

            switch (shape_type)
            {
                case ShapeCircle:  {
                    b2Circle circle = b2Shape_GetCircle(shapes[i]);
                    // x y rad
                    file.write_number(circle.center.x);
                    file.write_number(circle.center.y);
                    file.write_number(circle.radius);
                    break;
                }
                case ShapeCapsule: {
                    b2Capsule capsule = b2Shape_GetCapsule(shapes[i]);
                    // c1x c1y c2x c2y rad
                    file.write_number(capsule.center1.x);
                    file.write_number(capsule.center1.y);
                    file.write_number(capsule.center2.x);
                    file.write_number(capsule.center2.y);
                    file.write_number(capsule.radius);
                    break;
                }
                case ShapeSegment: {
                    b2Segment segment = b2Shape_GetSegment(shapes[i]);
                    // p1x p1y p2x p2y
                    file.write_number(segment.point1.x);
                    file.write_number(segment.point1.y);
                    file.write_number(segment.point2.x);
                    file.write_number(segment.point2.y);
                    break;
                }
                case ShapePolygon: {
                    b2Polygon polygon = b2Shape_GetPolygon(shapes[i]);
                    // vertex_count for i in range vertex_count p[i].x p[i].y
                    const int max_vertices = B2_MAX_POLYGON_VERTICES;
                    vec2 points[max_vertices];
                    int count = polygon.count;
                    file.write_integer(count);
                    for (int i = 0; i < count; i++)
                    {
                        vec2 p = { polygon.vertices[i].x, polygon.vertices[i].y };
                        file.write_number(p.x);
                        file.write_number(p.y);
                    }
                    break;
                }
                default: {
                    return false;  // unknown shape type
                }
            }
        }

        // @todo serialize draw data and the rest

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

    state->initialize();

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

        u64 bt = file.read_integer();
        BodyType body_type ((BodyType)bt);
        if (body_type != BodyStatic && body_type != BodyKinematic && body_type != BodyDynamic)
        {
            return false;
        }

        float xcoord = file.read_number();
        float ycoord = file.read_number();
        if (!(std::isfinite(xcoord) && std::isfinite(ycoord)))
        {
            return false;
        }

        game_object.transform = Transform(make_body(state->worldId, vec2(xcoord, ycoord), body_type));

        u64 shape_count = file.read_integer();
        if (shape_count > 8)
        {
            return false;
        }

        for (int i = 0; i < shape_count; i++)
        {
            ShapeType shape_type = (ShapeType)file.read_integer();
            if (shape_type != ShapeCircle && shape_type != ShapeCapsule && shape_type != ShapeSegment && shape_type != ShapePolygon)
            {
                return false;
            }

            b2Filter filter = {};
            filter.categoryBits = file.read_integer();
            filter.maskBits = file.read_integer();
            filter.groupIndex = file.read_int();

            switch (shape_type)
            {
                case ShapeCircle:
                {
                    // x y rad
                    double x = file.read_number();
                    double y = file.read_number();
                    double radius = file.read_number();
                    if (!(std::isfinite(x) && std::isfinite(y) && std::isfinite(radius)))
                    {
                        return false;
                    }
                    make_shape_circle(game_object.transform.body, vec2(x, y), radius, filter);
                    break;
                }
                case ShapeCapsule:
                {
                    b2Capsule capsule;
                    // c1x c1y c2x c2y
                    double c1x = file.read_number();
                    double c1y = file.read_number();
                    double c2x = file.read_number();
                    double c2y = file.read_number();
                    double radius = file.read_number();
                    if (!(std::isfinite(c1x) && std::isfinite(c1y) && std::isfinite(c2x) && std::isfinite(c2y) && std::isfinite(radius)))
                    {
                        return false;
                    }
                    make_shape_capsule(game_object.transform.body, vec2(c1x, c1y), vec2(c2x, c2y), radius, filter);
                    break;
                }
                case ShapeSegment:
                {
                    // p1x p1y p2x p2y
                    double p1x = file.read_number();
                    double p1y = file.read_number();
                    double p2x = file.read_number();
                    double p2y = file.read_number();
                    if (!(std::isfinite(p1x) && std::isfinite(p1y) && std::isfinite(p2x) && std::isfinite(p2y)))
                    {
                        return false;
                    }
                    make_shape_segment(game_object.transform.body, vec2(p1x, p1y), vec2(p2x, p2y), filter);
                    break;
                }
                case ShapePolygon:
                {
                    u64 num_vertices = file.read_integer();
                    const int max_vertices = B2_MAX_POLYGON_VERTICES;
                    vec2 vertices[max_vertices];
                    if (num_vertices < 3 || num_vertices > max_vertices)
                    {
                        return false;
                    }
                    for (int i = 0; i < num_vertices; i++)
                    {
                        double x = file.read_number();
                        double y = file.read_number();
                        if (!(std::isfinite(x) && std::isfinite(y)))
                        {
                            return false;
                        }

                        vertices[i] = vec2(x, y);
                    }
                    make_shape_polygon(game_object.transform.body, vertices, num_vertices, filter);
                    break;
                }
                default: return false;
            }
        }

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

    state->game_objects = std::move(objects);

    return true;
}


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
