#include "application.h"
#include "log.h"

#include <iostream>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_mixer/SDL_mixer.h>

bool Application::initialize()
{
    if (!game_state.initialize())
    {
        return false;
    }

    // load test level
    load_test_level(&game_state);

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        SDL_Log("Failed to init SDL: %s\n", SDL_GetError());
        return false;
    }

    // window
    {
        float scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());

        SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE |
                                SDL_WINDOW_HIDDEN;  // show the window after the initialization is complete
        SDL_Window* window = SDL_CreateWindow("game", 1440, 810, flags);
        if (!window)
        {
            SDL_Log("Failed to create window with SDL: %s\n", SDL_GetError());
            return false;
        }

        SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
        if (!renderer)
        {
            SDL_Log("Failed to create renderer with SDL: %s\n", SDL_GetError());
            return false;
        }

        SDL_ShowWindow(window);

        m_window = { window };

        int render_size_x, render_size_y;
        SDL_GetRenderOutputSize(renderer, &render_size_x, &render_size_y);
        m_render = { vec2(render_size_x, render_size_y), renderer };
    }

    {
        if (!TTF_Init())
        {
            fprintf(stderr, "Could not initialize TTF: %s\n", SDL_GetError());
            return false;
        }

        if (!MIX_Init())
        {
            fprintf(stderr, "Could not initialize MIX: %s\n", SDL_GetError());
            return false;
        }
    }

    if (!read_asset_catalog()) {
        log_error("Could not load assets\n");
        return false;
    }

    if (!m_audio_player.create()) {
        log_error("Failed to initialize audio player: %s\n", SDL_GetError());
        return false;
    }

    quit = false;

    return true;
}

Text Application::create_text(String text, Font font, Color color)
{
    SDL_Color sdl_color = { color.r, color.g, color.b, color.a };
    SDL_Surface* surface = TTF_RenderText_Solid(font.font, text.data, text.size, sdl_color);

    if (!surface)
        return Text();

    SDL_Texture* texture = SDL_CreateTextureFromSurface(m_render.renderer, surface);

    if (!texture)
    {
        SDL_DestroySurface(surface);
        return Text();
    }

    return Text(texture, text);
}

#define FONT_SIZE_SMALL   18.0
#define FONT_SIZE_MEDIUM  32.0
#define FONT_SIZE_LARGE   72.0

#ifdef _WIN32
    String path_separator = make_string("\\");
#else
    String path_separator = make_string("/");
#endif

bool Application::read_asset_catalog()
{
    String_Builder sb(256);
    const char* base_path = SDL_GetBasePath();

    sb.append(make_string(base_path));

    const char* desc_name = "run_tree.txt";
    sb.append(make_string(desc_name));
    bool parse_description = parse_assets(sb.c_string(), m_catalog);

    return parse_description;
}

bool load_font(Font* font, String_Builder& path, String font_folder, String font_file, float size)
{
    path.append(font_folder);
    path.append(path_separator);

    path.append(font_file);

    TTF_Font* ttf_font = TTF_OpenFont(path.c_string(), size);

    bool success = load_font_file(font, path.c_string(), size);

    path.remove(font_folder.size + 1 + font_file.size);

    return success;
}

bool load_font_file(Font* font, const char* path, float size)
{
    TTF_Font* ttf_font = TTF_OpenFont(path, size);
    if (!ttf_font)
    {
        fprintf(stderr, "Could not load font %s\n", path);
        fprintf(stderr, "%s\n", SDL_GetError());
        return false;
    }

    font->font = ttf_font;
    font->size = size;

    return true;
}

void Application::handle_events()
{
    SDL_Event e = {};
    while (SDL_PollEvent(&e))
    {
        switch (e.type)
        {
            case SDL_EVENT_QUIT:
            {
                quit = true;
                break;
            }
            case SDL_EVENT_KEY_DOWN:
            {
                SDL_KeyboardEvent keyboard = e.key;

                keyboard_input(keyboard);

                break;
            }
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            {
                SDL_MouseButtonEvent mouse = e.button;
                if (mouse_input())
                {
                    break;
                }

                break;
            }
            case SDL_EVENT_MOUSE_MOTION:
            {
                m_input.mouse.flags = SDL_GetMouseState(&m_input.mouse.pos.x, &m_input.mouse.pos.y);
                break;
            }
            case SDL_EVENT_WINDOW_RESIZED:
            {
                int render_size_x, render_size_y;
                SDL_GetRenderOutputSize(m_render.renderer, &render_size_x, &render_size_y);
                m_render.render_size = vec2(render_size_x, render_size_y);
                break;
            }
            default:
            {
                break;
            }
        }
    }

    update_keyboard_state();
}

bool Application::keyboard_input(SDL_KeyboardEvent keyboard)
{
    switch (keyboard.scancode)
    {
        case SDL_SCANCODE_ESCAPE:
        {
            quit = true;
            break;
        }
        case SDL_SCANCODE_S:
        {
            if (keyboard.mod & SDL_KMOD_LCTRL)
            {
                File file;
                // @todo decide on an extension
                file.open("save.ls", "w");
                serialize_game_state(&game_state, file);
                log_info("Saved game state to file");
            }
            break;
        }
        case SDL_SCANCODE_L:
        {
            if (keyboard.mod & SDL_KMOD_LCTRL)
            {
                GameState state;
                File file;
                file.open("save.ls", "r");
                if (read_game_state(&state, file))
                {
                    game_state = state;
                    log_info("Read game state");
                }
                else
                {
                    log_info("Failed to read game state");
                }
            }

            break;
        }
    }

    return false;
}

void Application::update_keyboard_state()
{
    m_input.keyboard.keys = SDL_GetKeyboardState(&m_input.keyboard.num_keys);
    m_input.keyboard.mod_state = SDL_GetModState();
}

void Application::update()
{
    // update time
    SDL_Time time = SDL_GetTicksNS();
    double time_sec = (double)time / NS_PER_SECONDS;
    m_delta_time = time - m_time;
    m_delta_time_seconds = time_sec - m_time_seconds;
    m_time = time;
    m_time_seconds = time_sec;

    timeout();

    game_state.update(m_time_seconds, m_delta_time_seconds, m_input);
}

void Application::timeout()
{
    for (int i = 0; i < ARRAY_SIZE(m_events); i++)
    {
        if (m_events[i].active)
        {
            if (m_events[i].event < m_time)
            {
                m_events[i].active = false;
            }
        }
    }
}

void Application::set_event_active(int event_index, double timeout_seconds)
{
    s64 timeout = (s64)(timeout_seconds * NS_PER_SECONDS);
    m_events[event_index].active = true;
    m_events[event_index].event = m_time + timeout;
}

void Application::set_event_deactive(int event_index)
{
    m_events[event_index].active = false;
}

void Application::cleanup()
{
    m_audio_player.cleanup();

    MIX_Quit();
    SDL_Quit();
}

void Application::draw()
{
    SDL_Renderer* renderer = m_render.renderer;

    if (SDL_GetWindowFlags(m_window.window) & SDL_WINDOW_MINIMIZED) {
        // don't draw anything if the window is minimized
        return;
    }

    SDL_SetRenderDrawColor(renderer, COLOR_ARG(m_background_color));
    SDL_RenderClear(renderer);

    draw_game(m_render, game_state);
    draw_ui(m_render);

    SDL_RenderPresent(renderer);
}

void Application::render_slider(Rectangle area, vec2 knob_scale, float value, Color slider_color, Color knob_color, const Text& text)
{
    float slider_knob_width = area.w * knob_scale.x;
    float slider_knob_height = area.h * knob_scale.y;

    SDL_SetRenderDrawColor(m_render.renderer, COLOR_ARG(slider_color));
    SDL_FRect slider = { area.x, area.y, area.w, area.h };
    SDL_RenderFillRect(m_render.renderer, &slider);
    float percentage = value;
    SDL_SetRenderDrawColor(m_render.renderer, COLOR_ARG(knob_color));
    SDL_FRect slider_knob = {
        slider.x - (slider_knob_width / 2) + (slider.w * percentage), slider.y + slider.h / 2 - slider_knob_height / 2,
        slider_knob_width, slider_knob_height
    };
    SDL_RenderFillRect(m_render.renderer, &slider_knob);

    // text
    {
        const int margin = 10;
        render_text_scale(m_render.renderer, text,
            vec2(slider.x + slider.w / 2, slider.y + slider.h * 2 + margin), vec2(0.6, 0.6));
    }
}

void Application::render_text_field(const Text_Field& text_field)
{
    SDL_FRect tf_area = { text_field.m_area.x, text_field.m_area.y, text_field.m_area.w, text_field.m_area.h };
    SDL_RenderFillRect(m_render.renderer, &tf_area);

    SDL_Texture* text_texture = text_field.m_texture;
    float texture_width;
    float texture_height;
    SDL_GetTextureSize(text_texture, &texture_width, &texture_height);

    if (text_texture)
    {
        int line_count = text_field.m_line_count;
        float font_size = text_field.m_font_size;

        SDL_FRect string_area = { tf_area.x, tf_area.y, texture_width, texture_height };
        SDL_FRect texture_area = { 0, 0, texture_width, texture_height };
        SDL_RenderTexture(m_render.renderer, text_texture, &texture_area, &string_area);

        SDL_SetRenderDrawColor(m_render.renderer, 0x33, 0x56, 0x74, 0xff);

        SDL_FRect cursor = (SDL_FRect){ tf_area.x + text_field.m_cursor_pixel_x,
                                        tf_area.y + text_field.m_cursor_pixel_y,
                                        tf_area.w / 100, font_size };
    }
}

void Application::render_dropdown(const Drop_Down_List& list, Color title_color, Color option_color) {
    SDL_SetRenderDrawColor(m_render.renderer, COLOR_ARG(title_color));

    SDL_FRect header_area = {
        list.pos.x - list.scale.x/2, list.pos.y - list.scale.y / 2,
        list.scale.x, list.scale.y
    };
    SDL_RenderFillRect(m_render.renderer, &header_area);
    render_text_size(m_render.renderer, list.title,
        vec2(header_area.x + header_area.w / 2, header_area.y + header_area.h / 2), vec2(header_area.w, header_area.h));

    if (list.open) {
        SDL_SetRenderDrawColor(m_render.renderer, COLOR_ARG(option_color));

        for (int i = 0; i < list.options.size(); i++) {
            SDL_FRect area = header_area;
            area.y += area.h * (i + 1);
            SDL_RenderFillRect(m_render.renderer, &area);
            render_text_size(m_render.renderer, list.get_option_label(i),
                vec2(area.x + area.w/2, area.y + area.h/2), vec2(area.w, area.h));
        }
    }
}

bool Application::mouse_input()
{
    return false;
}
