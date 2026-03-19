#ifndef _DRAW_H
#define _DRAW_H

#include "common.h"
#include "ui.h"
#include "game.h"
#include <SDL3/SDL.h>

using Texture = SDL_Texture*;  // @todo will probably change later

struct RenderContext {
    vec2 render_size = vec2();
    SDL_Renderer* renderer = nullptr;
};

void render_textured_rectangle(RenderContext context, Rectangle rect, SDL_Texture* texture, Color color);

void draw_game(RenderContext context, const GameState& state);
void draw_ui(RenderContext context);

void render_text_size(SDL_Renderer* renderer, Text text, vec2 where, vec2 absolute_scale = vec2(0, 0));
void render_text_scale(SDL_Renderer* renderer, Text text, vec2 where, vec2 scale_factor = vec2(0,0));

// polygon drawing
void draw_arrowhead(SDL_Renderer* renderer, vec2 position, vec2 direction, float scale, ColorF color);

#endif // _DRAW_H