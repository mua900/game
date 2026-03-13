#include "game.h"

bool GameState::initialize()
{
    return true;
}

void GameState::update(double delta_time)
{

}

void GameState::fixed_update(int ticks_per_second)
{
    double dt = 1.0 / double(ticks_per_second);
}