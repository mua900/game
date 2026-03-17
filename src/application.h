#pragma once

#include "common.h"
#include "template.h"
#include "audio.h"
#include "ui.h"
#include "game.h"
#include "input.h"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

enum ApplicationMode {
    AppModeMenu,
    AppModeGame,
};

struct Window {
    SDL_Window* window;
    SDL_Renderer* renderer;
};

struct Assets {
    Font font_small = {};
    Font font_medium = {};
    Font font_large = {};
};

#define NS_PER_SECONDS 1'000'000'000

struct Event_Timeout {
    s64 event = 0;
    bool active = false;
};

enum Events {
    EVENT_DUMMY,
    EVENT_COUNT,
};

struct AudioContext {
    AudioPlayer audio_player = {};
    AudioPlayer2 player = {};
    AudioData audio_data = {};
};

typedef float (*SampleGetter)(void* user, int frame, int channel);

class Application {
public:
    GameState game_state = {};

    ApplicationMode mode = ApplicationMode::AppModeMenu;

    Window m_window = {};
    Input m_input = {};

    Assets m_assets = {};
    AudioContext m_audio;

    Ui_State m_ui = {};
    Color m_background_color = DEFAULT_BACKGROUND_COLOR;

    s64 m_time = 0;
    double m_time_seconds = 0;
    s64 m_delta_time = 0;
    double m_delta_time_seconds = 0;

    Event_Timeout m_events[EVENT_COUNT] = {};

    Array<Text> m_rendered_text = {};

    bool quit = false;

    bool initialize();

    void handle_events();
    void update();
    void draw();

    void cleanup();
private:
    void timeout();
    void set_event_active(int event_index, double timeout_seconds);
    void set_event_deactive(int event_index);

    bool load_assets();
    bool game_load_assets(String_Builder& sb);  // helper

    void draw_game();
    void draw_ui();

    bool mouse_input();
    bool keyboard_input(SDL_KeyboardEvent keyboard);

    void update_keyboard_state();

    Text create_text(String text, Font font, Color color);

    void render_waveform(vec2 area_center, vec2 area_scale, int frame_count, int channel_count, Color color, SampleGetter sample_getter, void* user_data, bool draw_lines);

    void render_textured_rectangle(Rectangle rect, SDL_Texture* texture, Color color);

    void render_slider(Rectangle area, vec2 knob_scale, float value, Color slider_color, Color knob_color, const Text& text);
    void render_text_field(const Text_Field& text_field);
    void render_dropdown(const Drop_Down_List& list, Color title_color, Color option_color);
};

bool load_font(Font* font, String_Builder& path, String font_folder, String font_file, float size);

void render_text_size(SDL_Renderer* renderer, Text text, vec2 where, vec2 absolute_scale = vec2(0, 0));
void render_text_scale(SDL_Renderer* renderer, Text text, vec2 where, vec2 scale_factor = vec2(0,0));

// polygon drawing
void draw_arrowhead(SDL_Renderer* renderer, vec2 position, vec2 direction, float scale, ColorF color);
