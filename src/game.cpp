#include "game.h"

vec2 get_input_direction(const Input& input);

bool GameState::initialize()
{
    grid.initialize(100, 100);

    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = { 0.0f, 10.0f };
    worldId = b2CreateWorld(&worldDef);

    add_body_box(vec2( 0.0, 10.0 ), vec2(50, 10), b2_staticBody);

    vec2 playerPosition = vec2(500, 500);
    add_body_circle(playerPosition, 10.0, b2_kinematicBody);

    add_body_box(vec2(), vec2(10.0, 10.0), b2_dynamicBody);

    Player player;
    player.speed = 100;
    GameObject player_object = GameObject(playerPosition, player);
    GameObject wall = GameObject(vec2(100, 100), Wall(vec2(100, 100)));

    add_object(wall);
    wall.position.x += 100;
    add_object(wall);
    wall.position.y += 100;
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

                player.velocity = player.speed * get_input_direction(input);

                object.position += player.velocity * delta_time;

                break;
            }
            case GOT_Enemy: {
                Enemy& enemy = object.enemy;

                object.position += enemy.velocity * delta_time;

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

int spatial_hash(vec2 pos)
{
    return int(floor(pos.x) * 962623) ^ int(floor(pos.y) * 1193771);
}
