#include "game.h"

#include "draw_data.h"

float lightCastResult( b2ShapeId shapeId, b2Vec2 point, b2Vec2 normal, float fraction, void* context );
float playerCastResult( b2ShapeId shapeId, b2Vec2 point, b2Vec2 normal, float fraction, void* context );

vec2 get_input_direction(const Input& input);

struct PlayerCastContext {
    Transform* transform;
    double timeStep;
};

struct LightCastContext {
    vec2 pos;
    LineDrawData* draw_data;
};

bool GameState::initialize()
{
    grid.initialize(100, 100, 100);

    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = { 0.0f, 0.0f };
    worldId = b2CreateWorld(&worldDef);

    return true;
}

bool GameState::reinitialize()
{
    grid.clear_entries();
    return true;
}

void load_test_level(GameState* state)
{
    const vec2 playerPosition = vec2(500, 500);
    Player player;
    Transform player_transform;
    DrawData player_draw_data;
    b2Filter playerFilter = make_filter(CategoryPlayer, CategoryStatic | CategoryDynamic | CategoryLight, 0);
    player.speed = 100;
    player_transform.body = make_body_circle(state->worldId, playerPosition, 10, BodyKinematic, playerFilter);
    player_draw_data.color = ColorF(0.6, 0.7, 0.6, 1.0);

#if PHYSICS_DEBUG
    for (int i = 0; i < 8; i++)
        player.contacts[i] = {};
    player.contact_count = {};
#endif
    GameObject player_object = GameObject(GOT_Player, player_transform, player_draw_data);
    player_object.data.player = player;

    state->add_object(player_object);

    state->add_wall(vec2(100, 100), vec2(100, 100));
    state->add_wall(vec2(400, 400), vec2(400, 100));

    b2Filter dynamicFilter = make_filter(CategoryDynamic, CategoryPlayer | CategoryStatic | CategoryLight, 1);

    GameObject ball(GOT_Ball);
    ball.transform = Transform(make_body_circle(state->worldId, vec2(400, 100), 30.0, BodyDynamic, dynamicFilter));
    state->add_object(GameObject(ball));
    Ball ball2;
    ball.transform.body = make_body_circle(state->worldId, vec2(500, 100), 20.0, BodyDynamic, dynamicFilter);
    state->add_object(GameObject(ball));

    b2Filter emitter_filter = make_filter(CategoryDynamic, CategoryPlayer | CategoryStatic | CategoryDynamic | CategoryLight, 1);
    Transform emitter_transform = Transform(make_body_circle(state->worldId, vec2(600, 600), 10, BodyDynamic, emitter_filter));
    GameObject emitter_object(GOT_LaserEmitter, emitter_transform);
    emitter_object.data.emitter.direction = vec2(0,-1);
    emitter_object.data.emitter.draw_data = {};
    state->add_object(emitter_object);
}

void GameState::update(double elapsed_time, double delta_time, const Input& input)
{
    const int max_iterations = 10;
    int iterations = 0;
    const int targetTicksPerSecond = 60;
    const double tickTime = 1.0 / double(targetTicksPerSecond);
    double simulationTime = this->ticks * tickTime;
    while (elapsed_time > simulationTime)
    {
        fixed_update(ticks, tickTime, input);

        this->ticks += 1;
        simulationTime = this->ticks * tickTime;

        iterations += 1;
        if (iterations > max_iterations) break;
    }

    frame_update(elapsed_time, delta_time, input);
}

void GameState::frame_update(double elapsed_time, double delta_time, const Input& input)
{
    for (auto& object : game_objects)
    {
        switch (object.type)
        {
            case GOT_Wall: { break; }
            case GOT_Player: { break; }
            case GOT_LaserEmitter: { break; }
            case GOT_LaserCollector: { break; }
            case GOT_Mirror: { break; }
            case GOT_LaserReflector: { break; }
            case GOT_WavelengthShifter: { break; }
            case GOT_LaserSplitter: { break; }
            case GOT_EnergyGate: { break; }
            case GOT_EnergySource: { break; }
        }
    }

    this->frames += 1;
}

void GameState::fixed_update(u32 tick, double timeStep, const Input& input)
{
    for (auto& object : game_objects)
    {
        switch (object.type)
        {
            case GOT_Wall: { break; }
            case GOT_Player: {
                Player& player = object.data.player;
                vec2 position = object.transform.get_position();
                vec2 velocity = player.speed * get_input_direction(input);
                object.transform.set_velocity(velocity);

                float velocity_magnitude = velocity.magnitude();
                if (velocity_magnitude < 1e-6)
                {
                    break;
                }

                vec2 velocity_normal = velocity / velocity_magnitude;
                b2QueryFilter filter = b2DefaultQueryFilter();
                filter.categoryBits = CategoryPlayer;
                filter.maskBits = CategoryStatic | CategoryDynamic;
                vec2 cast_start = position;// + velocity_normal * playerRadius;
                b2Vec2 pos = { cast_start.x, cast_start.y };
                b2Vec2 vel = { velocity.x, velocity.y };

                PlayerCastContext context = { &object.transform, timeStep };
                b2World_CastRay( worldId, pos, vel, filter, playerCastResult, &context );

#if PHYSICS_DEBUG
                player.contact_count = b2Body_GetContactData(player.transform.body, player.contacts, 8);
#endif
                break;
            }
            case GOT_LaserEmitter: {
                LaserEmitter& emitter = object.data.emitter;
                vec2 position = object.transform.get_position();
                emitter.draw_data.points.discard_data();
                emitter.draw_data.points.add(position);
                calculate_light_beam(worldId, position + emitter.direction * 10, emitter.direction, &emitter.draw_data , 1000);
                break;
            }
            case GOT_LaserCollector: {
                break;
            }
            case GOT_Mirror: { break; }
            case GOT_LaserReflector: { break; }
            case GOT_WavelengthShifter: { break; }
            case GOT_LaserSplitter: { break; }
            case GOT_EnergyGate: { break; }
            case GOT_EnergySource: { break; }
        }
    }

    b2World_Step(worldId, timeStep, 4);
}

void GameState::cleanup()
{
    b2DestroyWorld(worldId);
    grid.cleanup();
}

int calculate_light_beam(b2WorldId worldId, vec2 start, vec2 dir, LineDrawData* line_draw_data, float range)
{
    b2QueryFilter filter = b2DefaultQueryFilter();
    filter.categoryBits = CategoryLight;
    filter.maskBits = CategoryStatic | CategoryDynamic | CategoryPlayer;
    b2Vec2 pos = {  start.x, start.y };
    vec2 direction = dir * range;
    b2Vec2 end = { start.x + direction.x, start.y + direction.y };

    LightCastContext context = { start, line_draw_data };
    b2World_CastRay( worldId, pos, end, filter, lightCastResult, &context );
    return context.draw_data->points.size();
}

vec2 get_input_direction(const Input& input)
{
    // @todo joystick

    vec2 dir = vec2();
    if (input.keyboard.keys[KEY_LEFT])
    {
        dir.x = -1;
    }
    else if (input.keyboard.keys[KEY_RIGHT])
    {
        dir.x = 1;
    }
    else {
        dir.x = 0;
    }

    if (input.keyboard.keys[KEY_UP])
    {
        dir.y = -1;
    }
    else if (input.keyboard.keys[KEY_DOWN])
    {
        dir.y = 1;
    }
    else {
        dir.y = 0;
    }

    if (!(dir.x == 0 && dir.y == 0))
        dir = dir.normalized();

    return dir;
}

void GameState::add_wall(vec2 position, vec2 scale)
{
    b2Filter staticFilter = make_filter(CategoryStatic, CategoryDynamic | CategoryPlayer | CategoryLight, 0);

    AABB wallBB;
    wallBB.min = position - scale / 2;
    wallBB.max = position + scale / 2;
    b2BodyId wall_body = make_body_box(this->worldId, position, scale, BodyStatic, staticFilter);
    GameObject object = GameObject(GOT_Wall, Transform(wall_body));
    object.data.wall.bounding_box = wallBB;
    this->add_object(object);
}

b2BodyId make_body(b2WorldId worldId, vec2 pos, BodyType body_type)
{
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2BodyType(body_type);
    bodyDef.position = { pos.x, pos.y };
    b2BodyId body = b2CreateBody(worldId, &bodyDef);
    return body;
}

b2BodyId make_body_box(b2WorldId worldId, vec2 position, vec2 scale, BodyType body_type, b2Filter filter)
{
    b2BodyId body = make_body(worldId, position, body_type);
    b2Polygon polygon = b2MakeBox(scale.x / 2, scale.y / 2);
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.filter = filter;
    b2CreatePolygonShape(body, &shapeDef, &polygon);

    return body;
}

b2BodyId make_body_circle(b2WorldId worldId, vec2 position, float radius, BodyType body_type, b2Filter filter)
{
    b2BodyId body = make_body(worldId, position, body_type);

    b2Circle circle = {};
    circle.radius = radius;
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.filter = filter;
    b2CreateCircleShape(body, &shapeDef, &circle);

    return body;
}

b2Filter make_filter(u64 categoryBits, u64 maskBits, int groupIndex)
{
    b2Filter filter = b2DefaultFilter();
    filter.categoryBits = categoryBits;
    filter.maskBits = maskBits;
    filter.groupIndex = groupIndex;
    return filter;
}

void translate_body(b2BodyId body, vec2 translate, double timeStep) {
    b2Transform transform = b2Body_GetTransform(body);
    transform.p.x += translate.x;
    transform.p.y += translate.y;
    b2Body_SetTargetTransform(body, transform, timeStep);
}

void rotate_body(b2BodyId body, float amount, double timeStep) {
    b2Transform transform = b2Body_GetTransform(body);
    float rotation_angle = b2Rot_GetAngle(transform.q) + amount;
    b2Rot rotation = b2MakeRot(rotation_angle);
    transform.q = rotation;

    b2Body_SetTargetTransform(body, transform, timeStep);
}

void translate(Transform& transform, vec2 translation, double timeStep)
{
    translate_body(transform.body, translation, timeStep);
}

void rotate(Transform& transform, float amount, double timeStep)
{
    rotate_body(transform.body, amount, timeStep);
}

AABB translate_bounding_box(AABB original, vec2 translation)
{
    original.min += translation;
    original.max += translation;

    return original;
}

AABB scale_bounding_box(AABB original, vec2 scale)
{
    vec2 original_scale = (original.max - original.min);
    vec2 new_scale = original_scale * scale;
    vec2 difference = new_scale - original_scale;

    original.min -= difference / 2;
    original.max += difference / 2;

    return original;
}

float lightCastResult( b2ShapeId shapeId, b2Vec2 point, b2Vec2 normal, float fraction, void* context )
{
    LightCastContext* castContext = (LightCastContext*) context;
    vec2 p = { point.x, point.y };
    vec2 difference = castContext->pos - p;
    float len = difference.magnitude();
    if (len < 1e-1)
        return -1;

    castContext->draw_data->points.add(p);

    return 1;
}

float playerCastResult( b2ShapeId shapeId, b2Vec2 point, b2Vec2 normal, float fraction, void* context )
{
    PlayerCastContext* castContext = (PlayerCastContext*) context;
    Transform* transform = castContext->transform;

    vec2 position = transform->get_position();
    vec2 normal_vector = vec2(normal.x, normal.y);
    vec2 point_vector = vec2(point.x, point.y);
    vec2 velocity = transform->get_velocity() * castContext->timeStep;
    float speed = velocity.magnitude();

    if (speed < 1e-6)
    {
        return 0;
    }

    float distance = (point_vector - position).magnitude();
    float remaining = speed - distance;

    if (remaining < 0)
    {
        return 0;
    }

    vec2 reflect = reflect2(velocity, normal_vector).normalized();

    velocity += reflect;
    transform->set_velocity(velocity);

    return 0;
}


void set_object_position(GameObject& object, vec2 pos)
{
    object.transform.set_position(pos);
}

vec2 get_object_position(GameObject object)
{
    return object.transform.get_position();
}


int spatial_hash(vec2 pos)
{
    return int(floor(pos.x) * 962623) ^ int(floor(pos.y) * 1193771);
}

int cell_hash(vec2 pos, float cell_size)
{
    return spatial_hash(pos / cell_size);
}

void SpatialGrid::initialize(int dim_x, int dim_y, float p_cell_size)
{
    dimension_x = dim_x;
    dimension_y = dim_y;
    cell_size = p_cell_size;

    cells = new GridCell[dim_x * dim_y];
}

void SpatialGrid::cleanup()
{
    if (cells)
        delete[] cells;
}

void SpatialGrid::clear_entries()
{
    // why are functions and variables live in the same namespace?
    int size = this->size();
    for (int i = 0; i < size; i++)
    {
        cells[i].count = 0;
        cells[i].overflow_cell = 0;
    }

    overflow_cells.discard_data();
}

int SpatialGrid::size()
{
    return dimension_x * dimension_y;
}

int SpatialGrid::calculate_cell_index(vec2 position)
{
    int hash = cell_hash(position, cell_size);
    hash = abs(hash);
    hash %= size();
    return hash;
}

void SpatialGrid::add(vec2 position, ObjectId object)
{
    int cell_index = calculate_cell_index(position);
    add_to_cell(cells[cell_index], object);
}

void SpatialGrid::remove(vec2 position, int object)
{
    int cell_index = calculate_cell_index(position);
    remove_from_cell(cells[cell_index], object);
}

GridCell* SpatialGrid::get_cell(vec2 position)
{
    int cell_index = calculate_cell_index(position);
    return &cells[cell_index];
}

// if we want the entries to be unique we would need to check all of them which we can do in a separate function as an opt in way
void SpatialGrid::add_to_cell(GridCell& cell, ObjectId object)
{
    if (cell.count == CELL_CAPACITY)
    {
        if (cell.overflow_cell == OVERFLOW_CELL_INDEX_SENTINEL)
        {
            int oc = overflow_cells.add(GridCell());
            cell.overflow_cell = oc;
        }

        add_to_cell(overflow_cells.get_ref(cell.overflow_cell), object);
        return;
    }

    cell.objects[cell.count] = object;
    cell.count += 1;
}

void SpatialGrid::remove_from_cell(GridCell& cell, ObjectId object)
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
