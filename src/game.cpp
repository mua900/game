#include "game.h"

float playerCastResult( b2ShapeId shapeId, b2Vec2 point, b2Vec2 normal, float fraction, void* context );

vec2 get_input_direction(const Input& input);

bool GameState::initialize()
{
    grid.initialize(100, 100);

    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = { 0.0f, 0.0f };
    worldId = b2CreateWorld(&worldDef);

    const vec2 playerPosition = vec2(500, 500);
    Player player;
    player.speed = 100;
    b2Filter playerFilter = make_filter(CategoryPlayer, CategoryStatic | CategoryDynamic, 0);
    player.transform.body = make_body_circle(worldId, playerPosition, 10.0, b2_kinematicBody, playerFilter);
    player.draw.color = ColorF(0.6, 0.7, 0.6, 1.0);
    GameObject player_object = GameObject(player);

    add_object(player_object);

    add_wall(vec2(100, 100), vec2(100, 100));
    add_wall(vec2(400, 400), vec2(400, 100));

    b2Filter dynamicFilter = make_filter(CategoryDynamic, CategoryPlayer | CategoryStatic, 0);

    Ball ball;
    ball.transform.body = make_body_circle(worldId, vec2(400, 100), 30.0, b2_dynamicBody, dynamicFilter);
    add_object(GameObject(ball));
    Ball ball2;
    ball.transform.body = make_body_circle(worldId, vec2(500, 100), 20.0, b2_dynamicBody, dynamicFilter);
    add_object(GameObject(ball));

    return true;
}

void GameState::update(double elapsed_time, double delta_time, const Input& input)
{
    const int targetTicksPerSecond = 60;
    const double tickTime = 1.0 / double(targetTicksPerSecond);
    double simulationTime = this->ticks * tickTime;
    while (elapsed_time > simulationTime)
    {
        fixed_update(ticks, tickTime, input);

        this->ticks += 1;
        simulationTime = this->ticks * tickTime;
    }

    frame_update(elapsed_time, delta_time, input);
}

void GameState::frame_update(double elapsed_time, double delta_time, const Input& input)
{
    /*
    for (auto& object : game_objects)
    {
        switch (object.type)
        {
            case GOT_Wall: { break; }
            case GOT_Player: {
                break;
            }
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
    */
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

                b2QueryFilter filter = b2DefaultQueryFilter();
                filter.categoryBits = CategoryPlayer;
                filter.maskBits = CategoryStatic | CategoryDynamic;
                b2Vec2 pos = {position.x, position.y};
                b2Vec2 vel = { velocity.x, velocity.y };
                b2TreeStats stats = b2World_CastRay( worldId, pos, vel, filter, playerCastResult, &player );

#if PHYSICS_DEBUG
                player.contact_count = b2Body_GetContactData(player.transform.body, player.contacts, 8);
#endif
                break;
            }
            case GOT_LaserEmitter: {
                break;
            }
            case GOT_LaserCollector: { break; }
            case GOT_Mirror: { break; }
            case GOT_LaserReflector: { break; }
            case GOT_WavelengthShifter: { break; }
            case GOT_LaserSplitter: { break; }
            case GOT_EnergyGate: { break; }
            case GOT_EnergySource: { break; }
        }
    }

    b2World_Step(worldId, timeStep, 4);

    // laser update
    calculate_light();
}

void GameState::cleanup()
{
    b2DestroyWorld(worldId);
}

void calculate_light()
{
    // @todo
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
    b2Filter staticFilter = make_filter(CategoryStatic, CategoryDynamic | CategoryPlayer, 0);

    AABB wallBB;
    wallBB.min = position - scale / 2;
    wallBB.max = position + scale / 2;
    b2BodyId wall_body = make_body_box(this->worldId, position, scale, b2_staticBody, staticFilter);
    this->add_object(GameObject(Wall(wall_body, wallBB)));
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

int spatial_hash(vec2 pos)
{
    return int(floor(pos.x) * 962623) ^ int(floor(pos.y) * 1193771);
}

float playerCastResult( b2ShapeId shapeId, b2Vec2 point, b2Vec2 normal, float fraction, void* context )
{
    Player* player = (Player*) context;

    vec2 position = player->transform.get_position();
    vec2 normal_vector = vec2(normal.x, normal.y);
    vec2 point_vector = vec2(point.x, point.y);
    vec2 velocity = player->transform.get_velocity();
    float velocity_scale = velocity.magnitude();

    if (velocity_scale < 1e-6)
    {
        return 0;
    }

    float distance = (point_vector - position).magnitude();
    // vec2 along_vector = velocity - normal_vector * dot2(normal_vector, velocity);
    vec2 reflect = reflect2(velocity, normal_vector).normalized();

    float remaining_distance = velocity_scale - distance;
    if (remaining_distance < 0)
    {
        return 0;
    }

    velocity += reflect * remaining_distance;
    player->transform.set_velocity(velocity);

    return 0;
}
