#include "audio.h"
#include "common.h"

#include <math.h>

// @todo pan

SDL_AudioStream* create_audio_stream(SDL_AudioDeviceID device, SDL_AudioSpec spec, int freq, int channels, double volume);

// adjust for latency
#define QUEUE_SAMPLE_SIZE DESIRED_AUDIO_SAMPLE_RATE / 16
#define QUEUE_FRAME_SIZE QUEUE_SAMPLE_SIZE / DESIRED_AUDIO_CHANNEL_COUNT

void AudioPlayer::put_audio_data()
{
    if (!this->audio_data)
        return;

    if (paused)
        return;

    int queued_bytes = SDL_GetAudioStreamQueued(stream);

    float* audio_samples = (float*) this->audio_data->samples;
    int queued_samples = queued_bytes / sizeof(float);
    int queued_frames = queued_samples / DESIRED_AUDIO_CHANNEL_COUNT;

    if (queued_frames >= QUEUE_FRAME_SIZE)
    {
        return;
    }

    int put_amount = MIN(
        QUEUE_FRAME_SIZE - queued_frames,
        this->audio_data->frame_count - playback_position
    );

    if (put_amount <= 0)
        return;

    SDL_PutAudioStreamData(stream, audio_samples + playback_position * this->audio_data->channel_count, put_amount * this->audio_data->channel_count * sizeof(float));
    playback_position += put_amount;

    if (playback_position >= this->audio_data->frame_count)
    {
        playback_position = 0;
        pause();
    }
}

bool AudioPlayer::initialize(int freq, int channels, double vol)
{
    SDL_AudioSpec spec;
    spec.format = SDL_AUDIO_F32;
    spec.freq = freq;
    spec.channels = channels;

    SDL_AudioDeviceID device_id = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec);

    if (!device_id)
    {
        fprintf(stderr, "Could not open audio device %s. %s\n", SDL_GetAudioDeviceName(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK), SDL_GetError());
        return false;
    }

    SDL_PauseAudioDevice(device_id);
    SDL_SetAudioDeviceGain(device_id, 1.0);

    SDL_AudioStream* astream = create_audio_stream(device_id, spec, freq, channels, vol);
    if (!astream)
    {
        return false;
    }

    this->playback_position = 0;
    this->device = device_id;
    this->stream = astream;
    this->volume = vol;

    return true;
}

void AudioPlayer::destroy()
{
    SDL_DestroyAudioStream(stream);
    SDL_CloseAudioDevice(device);
}

bool AudioPlayer::set_audio_data(AudioData* data)
{
    if (!data->is_in_desired_spec())
    {
        fprintf(stderr, "Unexpected audio data format\n");
        return false;
    }

    if (device)
    {
        SDL_PauseAudioDevice(device);
    }
    if (stream)
    {
        SDL_FlushAudioStream(stream);
    }

    audio_data = data;

    // automatically resume with the new data?

    return true;
}

void AudioPlayer::reset_audio_data()
{
    audio_data = nullptr;
    playback_position = 0;
    pause();
}

void AudioPlayer::pause()
{
    paused = true;
    SDL_PauseAudioDevice(device);
}

void AudioPlayer::resume()
{
    paused = false;
    SDL_ResumeAudioDevice(device);
}

void AudioPlayer::toggle_pause()
{
    if (paused) {
        resume();
    }
    else {
        pause();
    }
}

double AudioPlayer::get_volume() const
{
    return volume;
}

void AudioPlayer::set_volume(double p_volume)
{
    volume = p_volume;
    SDL_SetAudioStreamGain(stream, volume);
}


SDL_AudioStream* create_audio_stream(SDL_AudioDeviceID device, SDL_AudioSpec spec, int freq, int channels, double volume)
{
    SDL_AudioSpec device_spec = {};
    if (!SDL_GetAudioDeviceFormat(device, &device_spec, NULL))
    {
        return nullptr;
    }

    SDL_AudioStream* stream = SDL_CreateAudioStream(&spec, &device_spec);
    if (!stream)
    {
        return nullptr;
    }

    SDL_SetAudioStreamGain(stream, volume);

    if (!SDL_BindAudioStream(device, stream))
    {
        SDL_DestroyAudioStream(stream);
        return nullptr;
    }

    return stream;
}


bool AudioPlayer2::create() {
    MIX_Mixer* mix_mixer = MIX_CreateMixerDevice(simple_player->device, nullptr);
    if (!mix_mixer)
    {
        fprintf(stderr, "Couldn't create MIX_Mixer object: %s\n", SDL_GetError());
        return false;
    }

    this->mixer = mix_mixer;
    return true;
}

void AudioPlayer2::cleanup() {
    MIX_DestroyMixer(mixer);
}

bool AudioPlayer2::load_audio(const char* path, bool p_predecode) {
    MIX_Audio* audio = MIX_LoadAudio(mixer, path, p_predecode);
    if (!audio)
    {
        return false;
    }

    sounds.add(audio);

    return true;
}
