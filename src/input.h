#pragma once

#include <SDL3/SDL.h>

#define KEY_SPACE SDL_SCANCODE_SPACE
#define KEY_RETURN SDL_SCANCODE_RETURN
#define KEY_UP SDL_SCANCODE_UP
#define KEY_DOWN SDL_SCANCODE_DOWN
#define KEY_RIGHT SDL_SCANCODE_RIGHT
#define KEY_LEFT SDL_SCANCODE_LEFT

typedef SDL_MouseButtonFlags Mouse_Flags;

struct KeyboardState {
    const bool* keys = {};
    int num_keys = {};
    SDL_Keymod mod_state = {};
};

struct MouseState {
    vec2 pos = {};
    Mouse_Flags flags = {};
};

struct Input {
    KeyboardState keyboard;
    MouseState mouse;
};