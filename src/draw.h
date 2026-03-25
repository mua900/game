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

    // vertex and index cache for functions that use lower level SDL utilities for rendering
    DArray<SDL_Vertex> vertices;
    DArray<int> indices;
};

void render_rectangle(RenderContext context, Rectangle rect, Color color);
void render_textured_rectangle(RenderContext context, Rectangle rect, SDL_Texture* texture, Color color);

void draw_game(RenderContext context, const GameState& state);
void draw_ui(RenderContext context);

void render_text_size(SDL_Renderer* renderer, Text text, vec2 where, vec2 absolute_scale = vec2(0, 0));
void render_text_scale(SDL_Renderer* renderer, Text text, vec2 where, vec2 scale_factor = vec2(0,0));

void draw_arrowhead(SDL_Renderer* renderer, vec2 position, vec2 direction, float scale, ColorF color);
void draw_arrow(SDL_Renderer* renderer, vec2 start, vec2 end, float thickness, ColorF color);
void draw_rounded_rectangle(SDL_Renderer* renderer, vec2 center, vec2 base_scale, float corner_radius, ColorF color);
void draw_rounded_polygon(RenderContext context, Array<vec2> corners, float radius, ColorF color);
void draw_capsule(SDL_Renderer* renderer, vec2 center0, vec2 center1, float radius, ColorF color);
void draw_circle(SDL_Renderer* renderer, vec2 position, float radius, ColorF color);
void draw_segment(SDL_Renderer* renderer, vec2 start, vec2 end, float thick, ColorF color);
void draw_lines(RenderContext context, Array<vec2> points, float thickness, ColorF color);

#endif // _DRAW_H