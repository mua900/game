#include "game.h"

vec2 get_input_direction(const Input& input);

bool GameState::initialize()
{
    grid.initialize(100, 100);

    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = { 0.0f, 10.0f };
    worldId = b2CreateWorld(&worldDef);

    const vec2 playerPosition = vec2(500, 500);
    Player player;
    player.speed = 100;
    player.transform.body = make_body_circle(worldId, playerPosition, 10.0, b2_kinematicBody);
    GameObject player_object = GameObject(player);

    vec2 wallPosition = vec2(100, 100);
    vec2 wallScale = vec2(100, 100);
    AABB wallBB;
    wallBB.min = wallPosition;
    wallBB.max = wallPosition + wallScale;
    b2BodyId wall_body = make_body_box(worldId, vec2( wallPosition ), wallScale, b2_staticBody);
    GameObject wall = GameObject(Wall(wall_body, wallBB));

    add_object(wall);
    add_object(player_object);

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
            case GOT_Enemy: { break; }
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
    b2World_Step(worldId, timeStep, 4);

    for (auto& object : game_objects)
    {
        switch (object.type)
        {
            case GOT_Wall: { break; }
            case GOT_Player: {
                Player& player = object.player;
                vec2 velocity = player.speed * get_input_direction(input);
                player.transform.set_velocity(velocity);
#if PHYSICS_DEBUG
                target_move_pos = player.transform.get_position() + velocity;
#endif
                break;
            }
            case GOT_Enemy: { break; }
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
}

void GameState::cleanup()
{
    b2DestroyWorld(worldId);
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

b2BodyId make_body_box(b2WorldId worldId, vec2 position, vec2 scale, b2BodyType body_type)
{
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = body_type;
    b2BodyId body = b2CreateBody(worldId, &bodyDef);
    b2Polygon polygon = b2MakeBox(scale.x, scale.y);
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    b2CreatePolygonShape(body, &shapeDef, &polygon);

    return body;
}

b2BodyId make_body_circle(b2WorldId worldId, vec2 position, float radius, b2BodyType body_type)
{
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = body_type;
    bodyDef.position = {position.x, position.y};
    b2BodyId body = b2CreateBody(worldId, &bodyDef);

    b2Circle circle = {};
    circle.radius = radius;
    b2ShapeDef shapeDef = b2DefaultShapeDef();
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
