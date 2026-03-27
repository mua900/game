#ifndef _GAME_H
#define _GAME_H

#include "common.h"
#include "template.h"

#include "input.h"
#include "draw_data.h"

#include "box2d/box2d.h"

#define PHYSICS_DEBUG 1

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

// physics collision categories
enum GameCollisionCategories {
    CategoryPlayer   = 0x01,
    CategoryStatic   = 0x02,
    CategoryDynamic  = 0x04,
    CategoryLight    = 0x08,
};

using ObjectId = u32;

struct Transform {
    b2BodyId body;

    Transform() {}
    Transform(b2BodyId body) : body(body) {}

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

    vec2 get_velocity() const {
        b2Vec2 vel = b2Body_GetLinearVelocity(body);
        return vec2(vel.x, vel.y);
    }
};

struct AABB {
    vec2 min;
    vec2 max;
};

struct LaserCollector {
    Transform transform;

	LaserCollector(Transform transform) : transform(transform) {}
};

struct LaserEmitter {
    Transform transform;
    vec2 direction;
    LineDrawData draw_data;

	LaserEmitter(Transform transform) : transform(transform) {}
};

struct LaserReflector {
    Transform transform;
	LaserEmitter* source = nullptr;
	LaserCollector* target = nullptr;

	LaserReflector(Transform transform, LaserEmitter* src, LaserCollector* dst) : transform(transform), source(src), target(dst) {}
};

struct WavelengthShifter {
    Transform transform;
	float shift;

	WavelengthShifter(Transform transform, float shift) : transform(transform), shift(shift) {}
};

struct LaserSplitter {
    Transform transform;
	float scatter;
	int nbeams;

	LaserSplitter(Transform transform, float scat, int beam_count) : transform(transform), scatter(scat), nbeams(beam_count) {}
};

struct EnergySource {
    Transform transform;

	EnergySource(Transform transform) : transform(transform) {}
};

struct EnergyGate {
    Transform transform;
	EnergySource* source = nullptr;

	EnergyGate(Transform transform) : transform(transform) {}
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
    Transform transform;
    AABB bounding_box;

    Wall(Transform tr, AABB bb)
        : transform(tr), bounding_box(bb)
    {}
};

struct Player {
#if PHYSICS_DEBUG
    b2ContactData contacts[8];
    int contact_count;
#endif

    float speed;
    Transform transform;
    DrawData draw;

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
        EnergySource source;
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
    GameObject(LaserEmitter& emitter) : type(GOT_LaserEmitter), emitter(emitter)  {}
};

vec2 get_object_position(GameObject object);

#define OVERFLOW_CELL_INDEX_SENTINEL -1
#define CELL_CAPACITY 8
struct GridCell {
    ObjectId objects[CELL_CAPACITY];
    int count;
    int overflow_cell;
};

struct SpatialGrid {
    GridCell* cells;
    int dimension_x;
    int dimension_y;
    float cell_size;
    DArray<GridCell> overflow_cells;

    void initialize(int dim_x, int dim_y, float p_cell_size);
    int size();
    void add(vec2 position, ObjectId object);
    void remove(vec2 position, int object);
    GridCell* get_cell(vec2 position);
    int calculate_cell_index(vec2 position);
private:

    // if we want the entries to be unique we would need to check all of them which we can do in a separate function as an opt in way
    void add_to_cell(GridCell& cell, ObjectId object);
    void remove_from_cell(GridCell& cell, ObjectId object);
};

struct GameState {
    u32 ticks = 0;

    BucketList<GameObject> game_objects = {};
    SpatialGrid grid = {};

    b2WorldId worldId = {};

    bool initialize();
    void cleanup();
    void update(double elapsed_time, double delta_time, const Input& input);
    void frame_update(double elapsed_time, double delta_time, const Input& input);
    void fixed_update(u32 tick, double timeStep, const Input& input);

    ObjectId add_object(const GameObject& obj)
    {
        int object_index = game_objects.add(obj);
        grid.add(get_object_position(obj), object_index);
        return ObjectId(object_index);
    }

    void remove_object(ObjectId object)
    {
        grid.remove(get_object_position(game_objects.get(object)), object);
        game_objects.remove(object);
    }

    void add_wall(vec2 position, vec2 scale);
};

// light update
void calculate_light();
int calculate_light_beam(b2WorldId worldId, vec2 start, vec2 dir, LineDrawData* castContext, float range);

b2Filter make_filter(u64 categoryBits, u64 maskBits, int groupIndex);

b2BodyId make_body_box(b2WorldId worldId, vec2 position, vec2 scale, b2BodyType body_type, b2Filter filter);
b2BodyId make_body_circle(b2WorldId worldId, vec2 position, float radius, b2BodyType body_type, b2Filter filter);

void translate(vec2& pos, vec2 translate, b2BodyId body = b2_nullBodyId);
void rotate(vec2& direction, float amount, b2BodyId body = b2_nullBodyId);

void translate_body(b2BodyId body, vec2 translate, double timeStep);
void rotate_body(b2BodyId body, float amount, double timeStep);

void translate(Transform& transform, vec2 translate, double timeStep);
void rotate(Transform& transform, float amount, double timeStep);

AABB translate_bounding_box(AABB original, vec2 translation);
AABB scale_bounding_box(AABB original, vec2 scale);

#endif // _GAME_H
