#include "application.h"

#include <iostream>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

bool Application::initialize()
{
    if (!game_state.initialize())
    {
        return false;
    }

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        fprintf(stderr, "Failed to init SDL\n");
        return false;
    }

    // window
    {
        float scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());

        SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE |
                                SDL_WINDOW_HIDDEN;  // show the window after the initialization is complete
        SDL_Window* window = SDL_CreateWindow("soundtoy", 1440, 810, flags);
        if (!window)
        {
            fprintf(stderr, "Failed to create window\n");
            return false;
        }

        SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
        if (!renderer)
        {
            fprintf(stderr, "Failed to create renderer\n");
            return false;
        }

        SDL_ShowWindow(window);

        m_window = { window, renderer };
    }

    // ttf
    {
        if (!TTF_Init())
        {
            fprintf(stderr, "Could not initialize TTF\n");
            return false;
        }
    }

    if (!load_assets()) {
        fprintf(stderr, "Could not load assets\n");
        return false;
    }

    if (!m_audio.audio_player.initialize(DESIRED_AUDIO_SAMPLE_RATE, 2, 0.5)) {
        fprintf(stderr, "Failed to initialize audio player: %s\n", SDL_GetError());
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

    SDL_Texture* texture = SDL_CreateTextureFromSurface(m_window.renderer, surface);

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

bool Application::load_assets()
{
    String_Builder sb(256);
    const char* base_path = SDL_GetBasePath();

    sb.append(make_string(base_path));

    bool load_from_base_path = game_load_assets(sb);
    if (!load_from_base_path) {
        return false;
    }

    return true;
}

bool Application::game_load_assets(String_Builder& sb) {
    printf("Searching for assets in %s\n", sb.c_string());

    sb.append(make_string("asset"));
    sb.append(path_separator);

    // font
    {
        String folder = make_string("font");
        sb.append(folder);
        sb.append(path_separator);

        bool small_font = load_font(&m_assets.font_small, sb, make_string("Fira_Sans"), make_string("FiraSans-Regular.ttf"), FONT_SIZE_SMALL);
        bool medium_font = load_font(&m_assets.font_medium, sb, make_string("Fira_Sans"), make_string("FiraSans-Regular.ttf"), FONT_SIZE_MEDIUM);
        bool large_font = load_font(&m_assets.font_large, sb, make_string("Fira_Sans"), make_string("FiraSans-Regular.ttf"), FONT_SIZE_LARGE);

        sb.remove(folder.size + 1);

        if (!(small_font && medium_font && large_font))
        {
            fprintf(stderr, "Could not load fonts\n");
            return false;
        }
    }

    return true;
}

bool load_font(Font* font, String_Builder& path, String font_folder, String font_file, float size)
{
    path.append(font_folder);
    path.append(path_separator);

    path.append(font_file);

    TTF_Font* ttf_font = TTF_OpenFont(path.c_string(), size);

    path.remove(font_folder.size + 1 + font_file.size);

    if (!ttf_font) {
        SCOPE_STRING(font_file, font_name);
        fprintf(stderr, "Could not load font %s\n", font_name);
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
                m_mouse.flags = SDL_GetMouseState(&m_mouse.pos.x, &m_mouse.pos.y);
                break;
            }
            case SDL_EVENT_WINDOW_RESIZED:
            {
                ivec2 ws;
                SDL_GetWindowSize(m_window.window, &ws.x, &ws.y);
                break;
            }
            default:
            {
                break;
            }
        }
    }
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
    }

    return false;
}

void Application::update()
{
    // update time
    SDL_Time time = SDL_GetTicksNS();
    double time_sec = (double)time / NS_PER_SECONDS;
    m_time = time;
    m_time_seconds = time_sec;

    timeout();
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
    m_audio.audio_player.destroy();

    SDL_Quit();
}

void Application::draw()
{
    SDL_Renderer* renderer = m_window.renderer;

    if (SDL_GetWindowFlags(m_window.window) & SDL_WINDOW_MINIMIZED) {
        // don't draw anything if the window is minimized
        return;
    }

    SDL_SetRenderDrawColor(renderer, COLOR_ARG(m_background_color));
    SDL_RenderClear(renderer);

    draw_game();
    draw_ui();

    SDL_RenderPresent(renderer);
}

void Application::draw_game()
{
    // @todo
}

void Application::draw_ui()
{
    // @todo
}

void Application::render_slider(Rectangle area, vec2 knob_scale, float value, Color slider_color, Color knob_color, const Text& text)
{
    float slider_knob_width = area.w * knob_scale.x;
    float slider_knob_height = area.h * knob_scale.y;

    SDL_SetRenderDrawColor(m_window.renderer, COLOR_ARG(slider_color));
    SDL_FRect slider = { area.x, area.y, area.w, area.h };
    SDL_RenderFillRect(m_window.renderer, &slider);
    float percentage = value;
    SDL_SetRenderDrawColor(m_window.renderer, COLOR_ARG(knob_color));
    SDL_FRect slider_knob = {
        slider.x - (slider_knob_width / 2) + (slider.w * percentage), slider.y + slider.h / 2 - slider_knob_height / 2,
        slider_knob_width, slider_knob_height
    };
    SDL_RenderFillRect(m_window.renderer, &slider_knob);

    // text
    {
        const int margin = 10;
        render_text_scale(m_window.renderer, text,
            vec2(slider.x + slider.w / 2, slider.y + slider.h * 2 + margin), vec2(0.6, 0.6));
    }
}

void Application::render_text_field(const Text_Field& text_field)
{
    SDL_FRect tf_area = { text_field.m_area.x, text_field.m_area.y, text_field.m_area.w, text_field.m_area.h };
    SDL_RenderFillRect(m_window.renderer, &tf_area);

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
        SDL_RenderTexture(m_window.renderer, text_texture, &texture_area, &string_area);

        SDL_SetRenderDrawColor(m_window.renderer, 0x33, 0x56, 0x74, 0xff);

        SDL_FRect cursor = (SDL_FRect){ tf_area.x + text_field.m_cursor_pixel_x,
                                        tf_area.y + text_field.m_cursor_pixel_y,
                                        tf_area.w / 100, font_size };
    }
}

void Application::render_dropdown(const Drop_Down_List& list, Color title_color, Color option_color) {
    SDL_SetRenderDrawColor(m_window.renderer, COLOR_ARG(title_color));

    SDL_FRect header_area = {
        list.pos.x - list.scale.x/2, list.pos.y - list.scale.y / 2,
        list.scale.x, list.scale.y
    };
    SDL_RenderFillRect(m_window.renderer, &header_area);
    render_text_size(m_window.renderer, list.title,
        vec2(header_area.x + header_area.w / 2, header_area.y + header_area.h / 2), vec2(header_area.w, header_area.h));

    if (list.open) {
        SDL_SetRenderDrawColor(m_window.renderer, COLOR_ARG(option_color));

        for (int i = 0; i < list.options.size(); i++) {
            SDL_FRect area = header_area;
            area.y += area.h * (i + 1);
            SDL_RenderFillRect(m_window.renderer, &area);
            render_text_size(m_window.renderer, list.get_option_label(i),
                vec2(area.x + area.w/2, area.y + area.h/2), vec2(area.w, area.h));
        }
    }
}

bool Application::mouse_input()
{
    return false;
}

void Application::render_waveform(vec2 area_center, vec2 area_scale, int frame_count, int channel_count, Color color, SampleGetter sample_getter, void* user_data, bool draw_lines)
{
    SDL_SetRenderDrawColor(m_window.renderer, COLOR_ARG(color));

    #define BLOCK_SIZE 256
    static SDL_FPoint points[AUDIO_MAX_CHANNELS][BLOCK_SIZE];

    float vertical_step = area_scale.y / channel_count;
    float step = (area_scale.x / float(frame_count));
    float base_height = area_center.y;

    int iter_count = frame_count / BLOCK_SIZE;

    for (int block = 0; block < iter_count; block++)
    {
        int block_start = block * BLOCK_SIZE;

        for (int i = 0; i < BLOCK_SIZE; i+=1)
        {
            int frame_index = block * BLOCK_SIZE + i;

            for (int ch = 0; ch < channel_count; ch++)
            {
                float sample = sample_getter(user_data, frame_index, ch);

                points[ch][i].x = area_center.x - (area_scale.x / 2) + (step * frame_index);
                points[ch][i].y = (base_height + sample * (area_scale.y / 2.0)) / channel_count + (ch * vertical_step);
            }
        }

        for (int ch = 0; ch < channel_count; ch++)
        {
            if (draw_lines) {
                SDL_RenderLines(m_window.renderer, points[ch], BLOCK_SIZE);
            }
            else {
                SDL_RenderPoints(m_window.renderer, points[ch], BLOCK_SIZE);
            }
        }
    }

    int remaining = frame_count - (iter_count * BLOCK_SIZE);

    ASSERT(remaining < BLOCK_SIZE);

    for (int i = 0; i < remaining; i+=1)
    {
        int frame_index = iter_count * BLOCK_SIZE + i;

        for (int ch = 0; ch < channel_count; ch++)
        {
            float sample = sample_getter(user_data, frame_index, ch);

            points[ch][i].x = area_center.x - (area_scale.x / 2) + (step * frame_index);
            points[ch][i].y = base_height + sample * (area_scale.y / 2.0) / channel_count + (ch * vertical_step);
        }
    }

    for (int ch = 0; ch < channel_count; ch++)
    {
        if (draw_lines) {
            SDL_RenderLines(m_window.renderer, points[ch], remaining);
        }
        else {
            SDL_RenderPoints(m_window.renderer, points[ch], remaining);
        }
    }

    #undef BLOCK_SIZE
}

bool Application::load_audio_file(String path) {
    SCOPE_STRING(path, path_c_str);

    u8* output_buffer;
    int output_length = 0;

    // the spec we want
    SDL_AudioSpec desired_spec;
    desired_spec.channels = 2;
    desired_spec.format = SDL_AUDIO_F32;
    desired_spec.freq = 48000;

    {
        // @todo other file formats than wav
        // @todo don't change channel counts

        SDL_AudioSpec spec;  // output parameter
        u8* buffer = nullptr;
        u32 audio_length = 0;
        if (!SDL_LoadWAV(path_c_str, &spec, &buffer, &audio_length)) {
            fprintf(stderr, "Couldn't load audio file %s: %s\n", path_c_str, SDL_GetError());
            return false;
        }

        printf("%s\n", SDL_GetAudioFormatName(spec.format));

        bool convert_success = SDL_ConvertAudioSamples(&spec, buffer, audio_length, &desired_spec, &output_buffer, &output_length);

        SDL_free(buffer);
        audio_length = 0;

        if (!convert_success)
        {
            fprintf(stderr, "Couldn't convert audio samples to desired spec. %s\n", SDL_GetError());
            return false;
        }
    }

    m_audio.audio_data.samples = output_buffer;
    m_audio.audio_data.channel_count = desired_spec.channels;
    m_audio.audio_data.format = desired_spec.format;
    m_audio.audio_data.frequency = desired_spec.freq;
    m_audio.audio_data.frame_count = output_length / (SDL_AUDIO_BYTESIZE(desired_spec.format) * desired_spec.channels);

    ASSERT(m_audio.audio_data.is_in_desired_spec());

    return true;
}

void Application::render_textured_rectangle(Rectangle rect, SDL_Texture* texture, Color color) {
    SDL_SetRenderDrawColor(m_window.renderer, COLOR_ARG(color));
    SDL_FRect area = { rect.x, rect.y, rect.w, rect.h };
    SDL_RenderFillRect(m_window.renderer, &area);

    float tex_w, tex_h;
    SDL_GetTextureSize(texture, &tex_w, &tex_h);
    SDL_FRect src = {0,0,tex_w,tex_h};
    SDL_FRect dst = area;
    SDL_RenderTexture(m_window.renderer, texture, &src, &dst);
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
