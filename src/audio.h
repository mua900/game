#pragma once

#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>

#include "common.h"
#include "template.h"

using TrackId = u32;
static const TrackId NullTrackId = -1;

struct AudioPlayer {
    SDL_AudioDeviceID device = {};
    MIX_Mixer* mixer;
    // each track owns a single audio to play and we can query from SDL_mixer the audio object so we don't need to store it separately
    DArray<MIX_Track*> tracks;

    bool create();
    void cleanup();
    MIX_Audio* load_audio(const char* path);
    TrackId add_track(const char* path, const char* track_tag);
    TrackId make_track(MIX_Audio* audio, const char* track_tag);
    int get_track_count() const { return tracks.size(); }
    void remove_track(int index);
};
