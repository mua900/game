#include "draw.h"

void draw_game(RenderContext context, const GameState& state)
{
    for (const auto object : state.game_objects)
    {
        switch (object.type)
        {
            case GOT_Wall:
            {
                const Wall& wall = object.wall;
                float width = wall.bounding_box.max.x - wall.bounding_box.min.x;
                float height = wall.bounding_box.max.y - wall.bounding_box.min.y;
                Rectangle box = Rectangle(wall.bounding_box.min.x + width / 2, wall.bounding_box.min.y + height / 2, width, height);
                render_rectangle(context, box, Color(0x55, 0x88, 0x55, 0xff));
                break;
            }
            case GOT_Player:
            {
                const Player& player = object.player;
                vec2 position = player.transform.get_position();
                draw_circle(context.renderer, position, 20.0, player.draw.color);
#if PHYSICS_DEBUG
                for (int i = 0; i < player.contact_count; i++)
                {
                    vec2 normal = { player.contacts[i].manifold.normal.x,player.contacts[i].manifold.normal.y };
                    draw_arrow(context.renderer, position, position + normal * 20.0, 20.0, ColorF(1.0, 0, 0, 1.0));

                }

                vec2 velocity = player.transform.get_velocity();
                draw_arrow(context.renderer, position, position + velocity, 20, ColorF(0.4, 0.5, 0.7, 1.0));
#endif
                break;
            }
            case GOT_Ball:
            {
                const Ball& ball = object.ball;
                vec2 position = ball.transform.get_position();
                b2ShapeId shape = ball.transform.get_shape();
                b2Circle circle = b2Shape_GetCircle(shape);
                draw_circle(context.renderer, position, circle.radius, ColorF(0.5, 0.5, 0.5, 1.0));
                break;
            }
            case GOT_LaserEmitter:
            {
                const LaserEmitter& emitter = object.emitter;
                vec2 pos = emitter.transform.get_position();
                draw_circle(context.renderer, pos, 10, ColorF(0.9, 0.4, 0.3, 1.0));
                break;
            }
        }
    }
}

void draw_ui(RenderContext context)
{
    // @todo
}

void render_rectangle(RenderContext context, Rectangle rect, Color color)
{
    SDL_SetRenderDrawColor(context.renderer, COLOR_ARG(color));
    SDL_FRect area = { rect.x - rect.w / 2, rect.y - rect.h / 2, rect.w, rect.h  };
    SDL_RenderFillRect(context.renderer, &area);
}

void render_textured_rectangle(RenderContext context, Rectangle rect, SDL_Texture* texture, Color color)
{
    render_rectangle(context, rect, color);

    float tex_w, tex_h;
    SDL_GetTextureSize(texture, &tex_w, &tex_h);
    SDL_FRect src = {0,0,tex_w,tex_h};
    SDL_FRect dst = { rect.x - rect.w / 2, rect.y - rect.h / 2, rect.w, rect.h  };
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

    const int indices[3] = {0, 1, 2};

    SDL_RenderGeometry(renderer, NULL, vertices, 3, indices, 3);
}

void draw_arrow(SDL_Renderer* renderer, vec2 start, vec2 end, float thickness, ColorF color)
{
    // 3 for the arrow head, 4 for the quadrilateral below
    SDL_Vertex vertices[7];

    for (int i = 0; i < 7; i++) vertices[i].color = SDL_FColor {color.r, color.g, color.b, color.a};

    vec2 dir = end - start;
    float total_length = dir.magnitude();

    if (total_length < 1)
    {
        // subpixel arrow?
        return;
    }

    const float head_percentage = 0.2;  // 1 / 5 of the length is head
    const float head_width = thickness * 2;
    const float base_width = thickness;

    float head_size = total_length * head_percentage;
    dir = dir.normalized();
    vec2 ortho = vec2(-dir.y, dir.x);

    vec2 head_start = end - dir * head_size;
    vec2 arrow_left = head_start + ortho * head_width;
    vec2 arrow_right = head_start - ortho * head_width;
    vertices[0].position = SDL_FPoint { end.x, end.y };
    vertices[1].position = SDL_FPoint { arrow_left.x, arrow_left.y };
    vertices[2].position = SDL_FPoint { arrow_right.x, arrow_right.y };

    vec2 upper_base_left = head_start + ortho * base_width;
    vec2 upper_base_right = head_start - ortho * base_width;
    vec2 lower_base_left = upper_base_left - dir * total_length * (1.0 - head_percentage);
    vec2 lower_base_right = upper_base_right - dir * total_length * (1.0 - head_percentage);
    vertices[3].position = SDL_FPoint { upper_base_left.x, upper_base_left.y };
    vertices[4].position = SDL_FPoint { upper_base_right.x, upper_base_right.y };
    vertices[5].position = SDL_FPoint { lower_base_left.x, lower_base_left.y };
    vertices[6].position = SDL_FPoint { lower_base_right.x, lower_base_right.y };

    const int indices[9] = {
        0, 1, 2,  // head
        3, 5, 4,
        4, 5, 6
    };

    SDL_RenderGeometry(renderer, nullptr, vertices, 7, indices, ARRAY_SIZE(indices));
}

void draw_circle(SDL_Renderer* renderer, vec2 position, float radius, ColorF color)
{
    // change the number of vertices to use to configure how fine of an approximation we get
    #define NVERTICES 32
    SDL_Vertex vertices[NVERTICES + 1];

    SDL_Vertex center;
    center.position = SDL_FPoint {.x = position.x, .y = position.y};
    center.color = SDL_FColor { COLOR_ARG(color) };

    vertices[0] = center;

    // the angle between vertices and it's sin and cos
    const float angle = (2.0 * M_PI) / float(NVERTICES);
    const float c = std::cosf(angle);
    const float s = std::sinf(angle);

    float xcomp = 1.0;
    float ycomp = 0.0;
    for (int i = 1; i <= NVERTICES; i++)
    {
        vertices[i].position.x = center.position.x + xcomp * radius;
        vertices[i].position.y = center.position.y + ycomp * radius;
        vertices[i].color = SDL_FColor { color.r, color.g, color.b, color.a };

        // rotate the vector
        float n_xcomp = xcomp * c - ycomp * s;
        float n_ycomp = xcomp * s + ycomp * c;
        xcomp = n_xcomp;
        ycomp = n_ycomp;
    }

    int indices[NVERTICES * 3];
    for (int i = 0; i < NVERTICES - 1; i++)
    {
        indices[i * 3 + 0] = 0;
        indices[i * 3 + 1] = i + 1;
        indices[i * 3 + 2] = i + 2;
    }

    indices[(NVERTICES - 1) * 3 + 0] = 0;
    indices[(NVERTICES - 1) * 3 + 1] = NVERTICES;
    indices[(NVERTICES - 1) * 3 + 2] = 1;

    SDL_RenderGeometry(renderer, NULL, vertices, ARRAY_SIZE(vertices), indices, ARRAY_SIZE(indices));
    #undef NVERTICES
}

// @todo
void draw_rounded_rectangle(SDL_Renderer* renderer, vec2 position, vec2 base_scale, float corner_radius, ColorF color);

void draw_rounded_polygon(SDL_Renderer* renderer, vec2 center, vec2* corners, int corner_count, float radius, ColorF color);

void draw_capsule(SDL_Renderer* renderer, vec2 center0, vec2 center1, float radius, ColorF color)
{
    // total number of vertices used for either half circle sides of the capsule shape
    #define NVERTICES 32
    SDL_Vertex vertices[NVERTICES + 1];

    vec2 midpoint = (center0 + center1) / 2;

    vertices[0].position = { midpoint.x, midpoint.y };
    vertices[0].color = SDL_FColor { COLOR_ARG(color) };

    // the angle between vertices and it's sin and cos
    const float angle = (2.0 * M_PI) / float(NVERTICES);
    const float c = std::cosf(angle);
    const float s = std::sinf(angle);

    vec2 axis = (center1 - center0).normalized();

    // perpendicular vector
    float xcomp = -axis.y;
    float ycomp = axis.x;

    for (int i = 1; i <= NVERTICES / 2; i++)
    {
        vertices[i].position.x = center0.x + xcomp * radius;
        vertices[i].position.y = center0.y + ycomp * radius;
        vertices[i].color = SDL_FColor { color.r, color.g, color.b, color.a };

        float n_xcomp = xcomp * c - ycomp * s;
        float n_ycomp = xcomp * s + ycomp * c;

        xcomp = n_xcomp;
        ycomp = n_ycomp;
    }

    for (int i = NVERTICES / 2 + 1; i <= NVERTICES; i++)
    {
        vertices[i].position.x = center1.x + xcomp * radius;
        vertices[i].position.y = center1.y + ycomp * radius;
        vertices[i].color = SDL_FColor { color.r, color.g, color.b, color.a };

        float n_xcomp = xcomp * c - ycomp * s;
        float n_ycomp = xcomp * s + ycomp * c;

        xcomp = n_xcomp;
        ycomp = n_ycomp;
    }

    int indices[NVERTICES * 3];
    for (int i = 0; i < NVERTICES - 1; i++)
    {
        indices[i * 3 + 0] = 0;
        indices[i * 3 + 1] = i + 1;
        indices[i * 3 + 2] = i + 2;
    }

    indices[(NVERTICES - 1) * 3 + 0] = 0;
    indices[(NVERTICES - 1) * 3 + 1] = NVERTICES;
    indices[(NVERTICES - 1) * 3 + 2] = 1;

    SDL_RenderGeometry(renderer, NULL, vertices, ARRAY_SIZE(vertices), indices, ARRAY_SIZE(indices));
    #undef NVERTICES
}
