#pragma once

#include "common.h"
#include "template.h"
#include "audio.h"
#include "ui.h"
#include "game.h"
#include "input.h"
#include "draw.h"
#include "asset.h"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

enum ApplicationMode {
    AppModeMenu,
    AppModeGame,
};

struct Window {
    SDL_Window* window;
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

class Application {
public:
    GameState game_state = {};

    ApplicationMode mode = ApplicationMode::AppModeMenu;

    Window m_window = {};
    RenderContext m_render = {};
    Input m_input = {};

    Assets m_assets = {};
    AssetCatalog m_catalog = {};
    AudioPlayer m_audio_player = {};

    Ui_State m_ui = {};
    Color m_background_color = DEFAULT_BACKGROUND_COLOR;

    s64 m_time = 0;
    double m_time_seconds = 0;
    s64 m_delta_time = 0;
    double m_delta_time_seconds = 0;

    Event_Timeout m_events[EVENT_COUNT] = {};

    Array<Text> m_rendered_text = {};

    bool quit = false;

#if PHYSICS_DEBUG
    b2DebugDraw phys_debug_draw;
#endif

    bool initialize();

    void handle_events();
    void update();
    void draw();

    void cleanup();
private:
    void timeout();
    void set_event_active(int event_index, double timeout_seconds);
    void set_event_deactive(int event_index);

    bool read_asset_catalog();

    bool mouse_input();
    bool keyboard_input(SDL_KeyboardEvent keyboard);

    void update_keyboard_state();

    Text create_text(String text, Font font, Color color);

    void render_slider(Rectangle area, vec2 knob_scale, float value, Color slider_color, Color knob_color, const Text& text);
    void render_text_field(const Text_Field& text_field);
    void render_dropdown(const Drop_Down_List& list, Color title_color, Color option_color);
};
