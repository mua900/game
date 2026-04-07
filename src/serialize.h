#ifndef _SERIALIZE_H
#define _SERIALIZE_H

#include "common.h"
#include "game.h"

bool serialize_game_state(GameState* state, File& file);
bool read_game_state(GameState* state, File& file);

#endif // _SERIALIZE_H