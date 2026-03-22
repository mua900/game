#ifndef _GAME_H
#define _GAME_H

#include "common.h"
#include "template.h"

#include "input.h"

#include "box2d/box2d.h"

#define PHYSICS_DEBUG 0

enum GameObjectType {
    GOT_Wall,
    GOT_Ball,
    GOT_Player,
    GOT_LaserEmitter,
    GOT_LaserCollector,
    GOT_Mirror,
    GOT_LaserReflector,
    GOT_WavelengthShifter,
    GOT_LaserSplitter,
    GOT_EnergyGate,
    GOT_EnergySource,
};

using ObjectId = u32;

struct Transform {
    b2BodyId body;
    vec2 velocity;

    vec2 get_position() const
    {
        b2Transform transform = b2Body_GetTransform(body);
        return vec2(transform.p.x, transform.p.y);
    }

    vec2 get_direction() const
    {
        b2Transform transform = b2Body_GetTransform(body);
        return vec2(transform.q.c, transform.q.s);
    }

    b2ShapeId get_shape() const
    {
        b2ShapeId shape = b2_nullShapeId;
        b2Body_GetShapes(body, &shape, 1);
        return shape;
    }

    void set_velocity(vec2 vel)
    {
        b2Body_SetLinearVelocity(body, b2Vec2 { vel.x, vel.y });
    }
};

struct AABB {
    vec2 min;
    vec2 max;
};

struct LaserCollector {
    vec2 position;
	vec2 direction;

	LaserCollector(vec2 pos, vec2 dir) : position(pos), direction(dir) {}
};

struct LaserEmitter {
    vec2 position;
	vec2 direction;

	LaserEmitter(vec2 pos, vec2 dir) : position(pos), direction(dir) {}
};

struct LaserReflector {
    vec2 position;
	LaserEmitter* source = nullptr;
	LaserCollector* target = nullptr;

	LaserReflector(vec2 pos, LaserEmitter* src, LaserCollector* dst) : position(pos), source(src), target(dst) {}
};

struct WavelengthShifter {
    vec2 position;
	float shift;

	WavelengthShifter(vec2 pos, float shift) : position(pos), shift(shift) {}
};

struct LaserSplitter {
    vec2 position;
	float scatter;
	int nbeams;

	LaserSplitter(vec2 pos, float scat, int beam_count) : position(pos), scatter(scat), nbeams(beam_count) {}
};

struct EnergySource {
    vec2 position;

	EnergySource(vec2 position) : position(position) {}
};

struct EnergyGate {
    vec2 position;
	vec2 scale;
    b2BodyId body;
	EnergySource* source = nullptr;

	EnergyGate(vec2 pos, vec2 scale, b2BodyId body) : position(pos), scale(scale), body(body) {}
};

struct Mirror {
    Transform transform;

	Mirror(Transform transform) : transform(transform) {}
};

struct Ball {
    Transform transform;
};

// static world geometry
struct Wall {
    b2BodyId body;
    AABB bounding_box;

    Wall(b2BodyId body_id, AABB bb)
        : body(body_id), bounding_box(bb)
    {}
};

static const vec2 playerScale = vec2(50, 50);

struct Player {
#if PHYSICS_DEBUG
    b2ContactData contacts[8];
    int contact_count;
#endif

    float speed;
    Transform transform;

    Player() {}
};

struct GameObject {
    GameObjectType type;

    union {
        Player player;
        Wall wall;
        Ball ball;
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
    GameObject(Wall wall)
        : type(GOT_Wall), wall(wall)
    {}
    GameObject(Ball ball)
        : type(GOT_Ball), ball(ball)
    {}
    GameObject(Player& player)
        : type(GOT_Player), player(player)
    {}
};

int spatial_hash(vec2 pos);

#define OVERFLOW_CELL_INDEX_SENTINEL -1
#define CELL_CAPACITY 8
struct SpatialGridCell {
    ObjectId objects[CELL_CAPACITY];
    int count;
    int overflow_cell;
};

// @todo are we going to need this? If not just remove
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

#if PHYSICS_DEBUG
    vec2 target_move_pos = {};
#endif

    bool initialize();
    void cleanup();
    void update(double elapsed_time, double delta_time, const Input& input);
    void frame_update(double elapsed_time, double delta_time, const Input& input);
    void fixed_update(u32 tick, double timeStep, const Input& input);

    ObjectId add_object(const GameObject& obj)
    {
        int object_index = game_objects.add(obj);
        return ObjectId(object_index);
    }

    void remove_object(ObjectId object)
    {
        game_objects.remove(object);
    }

    void add_wall(vec2 position, vec2 scale);
};

// laser update
void calculate_light();

b2BodyId make_body_box(b2WorldId worldId, vec2 position, vec2 scale, b2BodyType body_type);
b2BodyId make_body_circle(b2WorldId worldId, vec2 position, float radius, b2BodyType body_type);

void translate(vec2& pos, vec2 translate, b2BodyId body = b2_nullBodyId);
void rotate(vec2& direction, float amount, b2BodyId body = b2_nullBodyId);

void translate_body(b2BodyId body, vec2 translate, double timeStep);
void rotate_body(b2BodyId body, float amount, double timeStep);

void translate(Transform& transform, vec2 translate, double timeStep);
void rotate(Transform& transform, float amount, double timeStep);

AABB translate_bounding_box(AABB original, vec2 translation);
AABB scale_bounding_box(AABB original, vec2 scale);

#endif // _GAME_H
