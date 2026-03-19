#ifndef _GAME_H
#define _GAME_H

// debug flags
#define PHYSICS_DEBUG 1

#include "common.h"
#include "template.h"

#include "input.h"

#include "box2d/box2d.h"

enum GameObjectType {
    GOT_Wall,
    GOT_Player,
    GOT_Enemy,
    GOT_LaserEmitter,
    GOT_LaserCollector,
    GOT_Mirror,
    GOT_LaserReflector,
    GOT_WavelengthShifter,
    GOT_LaserSplitter,
    GOT_EnergyGate,
    GOT_EnergySource,
};

struct LaserCollector {
	vec2 direction;

	LaserCollector(vec2 dir) : direction(dir) {}
};

struct LaserEmitter {
	vec2 direction;

	LaserEmitter(vec2 dir) : direction(dir) {}
};

struct LaserReflector {
	LaserEmitter* source = nullptr;
	LaserCollector* target = nullptr;

	LaserReflector(LaserEmitter* src, LaserCollector* dst) : source(src), target(dst) {}
};

struct WavelengthShifter {
	float shift;
};

struct LaserSplitter {
	float scatter;
	int nbeams;
};

struct EnergySource {

	EnergySource() {}
};

struct EnergyGate {
	vec2 scale;
	EnergySource* source = nullptr;

	EnergyGate(vec2 scale) : scale(scale) {}
};

struct Mirror {
	vec2 direction;

	Mirror(vec2 dir) : direction(dir) {}
};

struct Wall {
    vec2 scale;

    Wall(vec2 scale)
        : scale(scale)
    {}
};

struct Player {
    vec2 velocity = {};
    float speed = 0;
    float rotation = {};
};

struct Enemy {
    vec2 velocity = {};
    float rotation = {};
};

struct GameObject {
    GameObjectType type;
    vec2 position;

    union {
        Player player;
        Enemy enemy;
        Wall wall;
        LaserEmitter emitter;
        LaserCollector collector;
        Mirror mirror;
        LaserReflector reflector;
        EnergyGate gate;
        WavelengthShifter shifter;
        LaserSplitter splitter;
    };

    GameObject()
    {}
    GameObject(vec2 position, Wall& wall)
        : type(GOT_Wall), position(position), wall(wall)
    {}
    GameObject(vec2 position, Wall&& wall)
        : type(GOT_Wall), position(position), wall(wall)
    {}
    GameObject(vec2 position, Player& player)
        : type(GOT_Player), position(position), player(player)
    {}
    GameObject(vec2 position, Enemy& enemy)
        : type(GOT_Enemy), position(position), enemy(enemy)
    {}
};

int spatial_hash(vec2 pos);

// generation ids if they will be needed
using ObjectId = int;

#define OVERFLOW_CELL_INDEX_SENTINEL -1
#define CELL_CAPACITY 8
struct SpatialGridCell {
    ObjectId objects[CELL_CAPACITY];
    int count;
    int overflow_cell;
};

struct SpatialGrid {
    SpatialGridCell* cells;
    int dimension_x;
    int dimension_y;
    DArray<SpatialGridCell> overflow_cells;

    void initialize(int dim_x, int dim_y)
    {
        dimension_x = dim_x;
        dimension_y = dim_y;

        cells = new SpatialGridCell[dim_x * dim_y];
    }

    int size()
    {
        return dimension_x * dimension_y;
    }

    int calculate_cell_index(vec2 position)
    {
        int hash = spatial_hash(position);
        hash = abs(hash);
        hash %= size();
        return hash;
    }

    void add(vec2 position, ObjectId object)
    {
        int cell_index = calculate_cell_index(position);
        add_to_cell(cells[cell_index], object);
    }

    void remove(vec2 position, int object)
    {
        int cell_index = calculate_cell_index(position);
        remove_from_cell(cells[cell_index], object);
    }

    SpatialGridCell get_cell(vec2 position)
    {
        int cell_index = calculate_cell_index(position);
        return cells[cell_index];
    }

    // if we want the entries to be unique we would need to check all of them which we can do in a separate function as an opt in way
    void add_to_cell(SpatialGridCell& cell, ObjectId object)
    {
        if (cell.count == CELL_CAPACITY)
        {
            if (cell.overflow_cell == OVERFLOW_CELL_INDEX_SENTINEL)
            {
                int oc = overflow_cells.add(SpatialGridCell());
                cell.overflow_cell = oc;
            }

            add_to_cell(overflow_cells.get_ref(cell.overflow_cell), object);
            return;
        }

        cell.objects[cell.count] = object;
        cell.count += 1;
    }

    void remove_from_cell(SpatialGridCell& cell, ObjectId object)
    {
        for (int i = 0; i < cell.count; i++)
        {
            if (cell.objects[i] == object)
            {
                cell.objects[i] = cell.objects[cell.count - 1];
                cell.count -= 1;
                return;
            }
        }

        if (cell.overflow_cell != OVERFLOW_CELL_INDEX_SENTINEL)
        {
            remove_from_cell(overflow_cells.get_ref(cell.overflow_cell), object);
        }
    }
};

struct GameState {
    u32 ticks = 0;

    BucketList<GameObject> game_objects = {};
    SpatialGrid grid = {};

    b2WorldId worldId = {};
    DArray<b2BodyId> bodies = {};

    bool initialize();
    void cleanup();
    void update(double elapsed_time, double delta_time, const Input& input);
    void fixed_update(double timeStep);

    b2BodyId add_body_box(vec2 position, vec2 scale, b2BodyType body_type)
    {
        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = body_type;
        b2BodyId body = b2CreateBody(worldId, &bodyDef);
        b2Polygon polygon = b2MakeBox(scale.x, scale.y);
        b2ShapeDef shapeDef = b2DefaultShapeDef();
        b2CreatePolygonShape(body, &shapeDef, &polygon);

        bodies.add(body);
        return body;
    }

    b2BodyId add_body_circle(vec2 position, float radius, b2BodyType body_type)
    {
        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = body_type;
        bodyDef.position = {position.x, position.y};
        b2BodyId body = b2CreateBody(worldId, &bodyDef);

        b2Circle circle = {};
        circle.radius = radius;
        b2ShapeDef shapeDef = b2DefaultShapeDef();
        b2CreateCircleShape(body, &shapeDef, &circle);

        bodies.add(body);
        return body;
    }

    void add_object(GameObject& obj)
    {
        int object_index = game_objects.add(obj);
        grid.add(obj.position, object_index);
    }

    void remove_object(int object)
    {
        vec2 pos = game_objects.get(object).position;
        game_objects.remove(object);
        grid.remove(pos, object);
    }
};

#endif // _GAME_H
