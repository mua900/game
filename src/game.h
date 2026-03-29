#ifndef _GAME_H
#define _GAME_H

#include "common.h"
#include "template.h"

#include "input.h"
#include "draw_data.h"

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

    GOT_Sentinel
};

enum BodyType {
    BodyStatic = b2_staticBody,
    BodyKinematic = b2_kinematicBody,
    BodyDynamic = b2_dynamicBody,
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

    BodyType get_body_type() const
    {
        b2BodyType type = b2Body_GetType(body);
        return BodyType(type);
    }

    vec2 get_position() const
    {
        b2Transform transform = b2Body_GetTransform(body);
        return vec2(transform.p.x, transform.p.y);
    }

    void set_position(vec2 pos)
    {
        b2Transform transform = b2Body_GetTransform(body);
        b2Body_SetTransform(body, b2Vec2 {pos.x, pos.y}, transform.q);
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

	LaserCollector() {}
};

struct LaserEmitter {
    vec2 direction;
    LineDrawData draw_data;

	LaserEmitter() {}
};

struct LaserReflector {
	ObjectId source;
	ObjectId target;

	LaserReflector(ObjectId src, ObjectId dst) : source(src), target(dst) {}
};

struct WavelengthShifter {
	float shift;

	WavelengthShifter(float shift) : shift(shift) {}
};

struct LaserSplitter {
	float scatter;
	int nbeams;

	LaserSplitter(float scat, int beam_count) : scatter(scat), nbeams(beam_count) {}
};

struct EnergySource {

	EnergySource() {}
};

struct EnergyGate {
	ObjectId energy_source;

	EnergyGate() {}
};

struct Mirror {

	Mirror() {}
};

struct Ball {
    Ball() {}
};

struct Wall {
    AABB bounding_box;

    Wall(AABB bb)
        : bounding_box(bb)
    {}
};

struct Player {
#if PHYSICS_DEBUG
    b2ContactData contacts[8];
    int contact_count;
#endif

    float speed;

    Player() {}
};

union ObjectData {
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

struct GameObject {
    GameObjectType type = {};
    Transform transform = {};
    DrawData draw = {};
    ObjectData data = {};

    GameObject()
    {}
    GameObject(GameObjectType p_type) : type(p_type) {}
    GameObject(GameObjectType p_type, Transform tr) : type(p_type), transform(tr)
    {}
    GameObject(GameObjectType p_type, Transform tr, DrawData draw_data) : type(p_type), transform(tr), draw(draw_data)
    {}
};

vec2 get_object_position(GameObject object);
void set_object_position(GameObject& object, vec2 pos);

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
    void cleanup();
    void clear_entries();
    int size();
    void add(vec2 position, ObjectId object);
    void remove(vec2 position, int object);
    GridCell* get_cell(vec2 position);
    int calculate_cell_index(vec2 position);
private:

    void add_to_cell(GridCell& cell, ObjectId object);
    void remove_from_cell(GridCell& cell, ObjectId object);
};

struct GameState {
    u32 ticks = 0;
    u32 frames = 0;

    BucketList<GameObject> game_objects = {};
    SpatialGrid grid = {};

    b2WorldId worldId = {};

    bool initialize();
    bool reinitialize();
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

void load_test_level(GameState* state);

// serialization
bool serialize_game_state(GameState* state, File& file);
bool read_game_state(GameState* state, File& file);

// light update
void calculate_light();
int calculate_light_beam(b2WorldId worldId, vec2 start, vec2 dir, LineDrawData* castContext, float range);

b2Filter make_filter(u64 categoryBits, u64 maskBits, int groupIndex);

b2BodyId make_body(b2WorldId worldId, vec2 pos, BodyType body_type);
b2BodyId make_body_box(b2WorldId worldId, vec2 position, vec2 scale, BodyType body_type, b2Filter filter);
b2BodyId make_body_circle(b2WorldId worldId, vec2 position, float radius, BodyType body_type, b2Filter filter);

void translate(vec2& pos, vec2 translate, b2BodyId body = b2_nullBodyId);
void rotate(vec2& direction, float amount, b2BodyId body = b2_nullBodyId);

void translate_body(b2BodyId body, vec2 translate, double timeStep);
void rotate_body(b2BodyId body, float amount, double timeStep);

void translate(Transform& transform, vec2 translate, double timeStep);
void rotate(Transform& transform, float amount, double timeStep);

AABB translate_bounding_box(AABB original, vec2 translation);
AABB scale_bounding_box(AABB original, vec2 scale);

#endif // _GAME_H
