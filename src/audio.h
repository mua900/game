#pragma once

#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>

#include "common.h"
#include "template.h"

// @todo speed control for playback maybe? Take a look at FrequencyRatio.

#define AUDIO_MAX_CHANNELS 8

#define DESIRED_AUDIO_FORMAT SDL_AUDIO_F32
#define DESIRED_AUDIO_SAMPLE_RATE 48000
#define DESIRED_AUDIO_CHANNEL_COUNT 2

struct AudioData {
    void* samples = nullptr;
    SDL_AudioFormat format = DESIRED_AUDIO_FORMAT;
    int channel_count = 0;
    int frequency = 0;
    int frame_count = 0;

    void reset()
    {
        if (samples) {
            free(samples);
        }

        samples = nullptr;
        format = DESIRED_AUDIO_FORMAT;
        channel_count = 0;
        frequency = 0;
        frame_count = 0;
    }

    bool is_in_desired_spec()
    {
        return (samples != nullptr) && (format == DESIRED_AUDIO_FORMAT) && (channel_count == DESIRED_AUDIO_CHANNEL_COUNT) && (frequency == DESIRED_AUDIO_SAMPLE_RATE);
    }

    bool load_audio_file(String path);
};

// @todo a more complete audio player
struct AudioPlayer
{
    AudioData* audio_data = {};  // reference
    SDL_AudioDeviceID device = {};
    SDL_AudioStream* stream = {};
    int playback_position = 0;

    double volume = 0.0;
    double pan = 0.0;
    bool paused = true;

    bool initialize(int freq, int channels, double vol);
    void destroy();

    bool set_audio_data(AudioData* data);
    void reset_audio_data();

    void put_audio_data();

    void pause();
    void resume();
    void toggle_pause();
    void set_volume(double volume);
    double get_volume() const;
    double get_pan() const { return pan; }
    void set_pan(float p) { pan = p; }
};

// @todo
struct AudioPlayer2 {
    AudioPlayer* simple_player;
    MIX_Mixer* mixer;
    DArray<MIX_Track*> tracks;
    DArray<MIX_Audio*> sounds;

    bool create();
    void cleanup();
    int add_track();
    void remove_track(int index);
    int get_track_count() const { return tracks.size(); }
    bool load_audio(const char* path, bool p_predecode = false);
};