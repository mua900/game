#include "game.h"

vec2 get_input_direction(const Input& input);

bool GameState::initialize()
{
    grid.initialize(100, 100);

    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = { 0.0f, 10.0f };
    worldId = b2CreateWorld(&worldDef);

    make_body_box(worldId, vec2(), vec2(10.0, 10.0), b2_dynamicBody);

    const vec2 playerPosition = vec2(500, 500);
    const vec2 playerScale = vec2(50, 50);
    Player player;
    player.speed = 100;
    player.transform.position = playerPosition;
    player.transform.scale = playerScale;
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
    const int targetTicksPerSecond = 40;
    const double tickTime = 1.0 / double(targetTicksPerSecond);
    double simulationTime = ticks * tickTime;
    while (elapsed_time > simulationTime)
    {
        fixed_update(targetTicksPerSecond);

        ticks += 1;
        simulationTime = ticks * tickTime;
    }


    for (auto& object : game_objects)
    {
        switch (object.type)
        {
            case GOT_Wall: {
                break;
            }
            case GOT_Player: {
                Player& player = object.player;

                player.transform.velocity = player.speed * get_input_direction(input);

                player.transform.position += player.transform.velocity * delta_time;

                break;
            }
            case GOT_Enemy: {
                Enemy& enemy = object.enemy;

                vec2 dir = get_direction_vector(enemy.transform.direction);
                vec2 velocity = dir * enemy.transform.velocity;
                enemy.transform.position += velocity * delta_time;

                break;
            }
        }
    }
}

void GameState::fixed_update(double timeStep)
{
    b2World_Step(worldId, timeStep, 4);
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

int spatial_hash(vec2 pos)
{
    return int(floor(pos.x) * 962623) ^ int(floor(pos.y) * 1193771);
}
