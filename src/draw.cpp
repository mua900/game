#include "draw.h"

void draw_game(RenderContext context, const GameState& state)
{
#if PHYSICS_DEBUG
    SDL_SetRenderDrawColor(context.renderer, 0x44, 0x66, 0x55, 0xff);

    for (const auto body_id : state.bodies)
    {
        b2Vec2 position = b2Body_GetPosition(body_id);
        b2Rot rotation = b2Body_GetRotation(body_id);

        SDL_FRect area = { position.x, position.y, 60, 60 };
        SDL_RenderFillRect(context.renderer, &area);
    }
#endif

    for (const auto object : state.game_objects)
    {
        switch (object.type)
        {
            case GOT_Wall:
            {
                SDL_FRect area = { object.position.x, object.position.y, 100, 100 };
                SDL_SetRenderDrawColor(context.renderer, 0x55, 0x88, 0x55, 0xff);
                SDL_RenderFillRect(context.renderer, &area);
                break;
            }
            case GOT_Player:
            {
                SDL_FRect area = { object.position.x, object.position.y, 50, 50 };
                SDL_SetRenderDrawColor(context.renderer, 0xAA, 0x66, 0x99, 0xff);
                SDL_RenderFillRect(context.renderer, &area);

                break;
            }
            case GOT_Enemy:
            {
                break;
            }
        }
    }
}

void draw_ui(RenderContext context)
{
    // @todo
}

void render_textured_rectangle(RenderContext context, Rectangle rect, SDL_Texture* texture, Color color)
{
    SDL_SetRenderDrawColor(context.renderer, COLOR_ARG(color));
    SDL_FRect area = { rect.x, rect.y, rect.w, rect.h };
    SDL_RenderFillRect(context.renderer, &area);

    float tex_w, tex_h;
    SDL_GetTextureSize(texture, &tex_w, &tex_h);
    SDL_FRect src = {0,0,tex_w,tex_h};
    SDL_FRect dst = area;
    SDL_RenderTexture(context.renderer, texture, &src, &dst);
}

void render_text_size(SDL_Renderer* renderer, Text text, vec2 where, vec2 absolute_scale)
{
    float tex_w, tex_h;
    SDL_GetTextureSize(text.texture, &tex_w, &tex_h);

    if (!absolute_scale.x)
    {
        absolute_scale = vec2(tex_w, tex_h);
    }

    SDL_FRect src = { 0,0,tex_w,tex_h };
    SDL_FRect dst = {where.x - absolute_scale.x/2, where.y - absolute_scale.y/2, absolute_scale.x, absolute_scale.y};

    SDL_RenderTexture(renderer, text.texture, &src, &dst);
}

void render_text_scale(SDL_Renderer* renderer, Text text, vec2 where, vec2 scale_factor)
{
    float tex_w, tex_h;
    SDL_GetTextureSize(text.texture, &tex_w, &tex_h);

    if (!scale_factor.x)
    {
        scale_factor = vec2(1,1);
    }

    vec2 scale = vec2(tex_w * scale_factor.x, tex_h * scale_factor.y);

    SDL_FRect src = { 0,0,tex_w,tex_h };
    SDL_FRect dst = {where.x - scale.x/2, where.y - scale.y/2, scale.x, scale.y};

    SDL_RenderTexture(renderer, text.texture, &src, &dst);
}

void draw_arrowhead(SDL_Renderer* renderer, vec2 position, vec2 direction, float scale, ColorF color)
{
    SDL_Vertex vertices[3] = {};

    vec2 dir = direction.normalized();
    vec2 dir_ortho = vec2(-dir.y, dir.x);

    vertices[0].position.x = position.x + dir.x * scale;
    vertices[0].position.y = position.y + dir.y * scale;
    vertices[0].color = SDL_FColor{ color.r, color.g, color.b, color.a };

    vertices[1].position.x = position.x + dir_ortho.x * scale;
    vertices[1].position.y = position.y + dir_ortho.y * scale;
    vertices[1].color = SDL_FColor{ color.r, color.g, color.b, color.a };

    vertices[2].position.x = position.x - dir_ortho.x * scale;
    vertices[2].position.y = position.y - dir_ortho.y * scale;
    vertices[2].color = SDL_FColor{ color.r, color.g, color.b, color.a };

    int indices[3] = {0, 1, 2};

    SDL_RenderGeometry(renderer, NULL, vertices, 3, indices, 3);
}
