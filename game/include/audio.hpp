#pragma once
#include <AL/al.h>
#include <AL/alc.h>
#include <vector>

typedef uint32_t audio_clip_handle;

struct audio_context
{
    ALCcontext* context;
    ALCdevice* device;
    ALuint sources[16];

    std::vector<ALuint> leased;
    std::vector<ALuint> available;
    std::vector<ALuint> loaded_clips;
};

void init_audio(audio_context* ctxt);
void shutdown_audio(audio_context* ctxt);
void audio_update(audio_context* ctxt);
void play_sound(audio_context* ctxt, audio_clip_handle clip);
audio_clip_handle load_sound(audio_context* ctxt, const char* path);