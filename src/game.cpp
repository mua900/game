#include "game.h"

#include "draw_data.h"

float lightCastResult( b2ShapeId shapeId, b2Vec2 point, b2Vec2 normal, float fraction, void* context );
float playerCastResult( b2ShapeId shapeId, b2Vec2 point, b2Vec2 normal, float fraction, void* context );

vec2 get_input_direction(const Input& input);

struct PlayerCastContext {
    Player* player;
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

    const vec2 playerPosition = vec2(500, 500);
    Player player;
    player.speed = 100;
    b2Filter playerFilter = make_filter(CategoryPlayer, CategoryStatic | CategoryDynamic | CategoryLight, 0);
    player.transform.body = make_body_circle(worldId, playerPosition, 10, b2_kinematicBody, playerFilter);
    player.draw.color = ColorF(0.6, 0.7, 0.6, 1.0);

#if PHYSICS_DEBUG
    for (int i = 0; i < 8; i++)
        player.contacts[i] = {};
    player.contact_count = {};
#endif
    GameObject player_object = GameObject(player);

    add_object(player_object);

    add_wall(vec2(100, 100), vec2(100, 100));
    add_wall(vec2(400, 400), vec2(400, 100));

    b2Filter dynamicFilter = make_filter(CategoryDynamic, CategoryPlayer | CategoryStatic | CategoryLight, 1);

    Ball ball;
    ball.transform.body = make_body_circle(worldId, vec2(400, 100), 30.0, b2_dynamicBody, dynamicFilter);
    add_object(GameObject(ball));
    Ball ball2;
    ball.transform.body = make_body_circle(worldId, vec2(500, 100), 20.0, b2_dynamicBody, dynamicFilter);
    add_object(GameObject(ball));

    b2Filter emitter_filter = make_filter(CategoryDynamic, CategoryPlayer | CategoryStatic | CategoryDynamic | CategoryLight, 1);
    Transform emitter_transform = Transform(make_body_circle(worldId, vec2(600, 600), 10, b2_dynamicBody, emitter_filter));
    LaserEmitter emitter(emitter_transform);
    emitter.direction = vec2(0,-1);
    add_object(GameObject(emitter));

    return true;
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
                Player& player = object.player;
                vec2 position = player.transform.get_position();
                vec2 velocity = player.speed * get_input_direction(input);
                player.transform.set_velocity(velocity);

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

                PlayerCastContext context = { &player, timeStep };
                b2World_CastRay( worldId, pos, vel, filter, playerCastResult, &context );

#if PHYSICS_DEBUG
                player.contact_count = b2Body_GetContactData(player.transform.body, player.contacts, 8);
#endif
                break;
            }
            case GOT_LaserEmitter: {
                LaserEmitter& emitter = object.emitter;
                vec2 position = emitter.transform.get_position();
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
    b2BodyId wall_body = make_body_box(this->worldId, position, scale, b2_staticBody, staticFilter);
    this->add_object(GameObject(Wall(Transform(wall_body), wallBB)));
}

b2BodyId make_body_box(b2WorldId worldId, vec2 position, vec2 scale, b2BodyType body_type, b2Filter filter)
{
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = body_type;
    bodyDef.position = b2Vec2{position.x, position.y};
    b2BodyId body = b2CreateBody(worldId, &bodyDef);
    b2Polygon polygon = b2MakeBox(scale.x / 2, scale.y / 2);
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.filter = filter;
    b2CreatePolygonShape(body, &shapeDef, &polygon);

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

b2BodyId make_body_circle(b2WorldId worldId, vec2 position, float radius, b2BodyType body_type, b2Filter filter)
{
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = body_type;
    bodyDef.position = {position.x, position.y};
    b2BodyId body = b2CreateBody(worldId, &bodyDef);

    b2Circle circle = {};
    circle.radius = radius;
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.filter = filter;
    b2CreateCircleShape(body, &shapeDef, &circle);

    return body;
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
    Player* player = castContext->player;

    vec2 position = player->transform.get_position();
    vec2 normal_vector = vec2(normal.x, normal.y);
    vec2 point_vector = vec2(point.x, point.y);
    vec2 velocity = player->transform.get_velocity() * castContext->timeStep;
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
    player->transform.set_velocity(velocity);

    return 0;
}


// @todo these should be replaced with calls to transform.set/get

void set_object_position(GameObject& object, vec2 pos)
{
    switch (object.type)
    {
        case GOT_Wall:              { object.wall.transform.set_position(pos); }
        case GOT_Ball:              { object.ball.transform.set_position(pos); }
        case GOT_Player:            { object.player.transform.set_position(pos); }
        case GOT_LaserEmitter:      { object.emitter.transform.set_position(pos); }
        case GOT_LaserCollector:    { object.collector.transform.set_position(pos); }
        case GOT_Mirror:            { object.mirror.transform.set_position(pos); }
        case GOT_LaserReflector:    { object.reflector.transform.set_position(pos); }
        case GOT_WavelengthShifter: { object.shifter.transform.set_position(pos); }
        case GOT_LaserSplitter:     { object.splitter.transform.set_position(pos); }
        case GOT_EnergyGate:        { object.gate.transform.set_position(pos); }
        case GOT_EnergySource:      { object.source.transform.set_position(pos); }
    }
}

vec2 get_object_position(GameObject object)
{
    switch (object.type)
    {
        case GOT_Wall:              { return object.wall.transform.get_position(); }
        case GOT_Ball:              { return object.ball.transform.get_position(); }
        case GOT_Player:            { return object.player.transform.get_position(); }
        case GOT_LaserEmitter:      { return object.emitter.transform.get_position(); }
        case GOT_LaserCollector:    { return object.collector.transform.get_position(); }
        case GOT_Mirror:            { return object.mirror.transform.get_position(); }
        case GOT_LaserReflector:    { return object.reflector.transform.get_position(); }
        case GOT_WavelengthShifter: { return object.shifter.transform.get_position(); }
        case GOT_LaserSplitter:     { return object.splitter.transform.get_position(); }
        case GOT_EnergyGate:        { return object.gate.transform.get_position(); }
        case GOT_EnergySource:      { return object.source.transform.get_position(); }
    }

    return vec2(0,0);
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
