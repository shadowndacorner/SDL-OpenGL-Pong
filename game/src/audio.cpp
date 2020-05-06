#include <audio.hpp>
#define TINY_WAV_IMPLEMENTATION
#include "tinywav.h"

void init_audio(audio_context* ctxt)
{
    ctxt->device = alcOpenDevice(0);
    ctxt->context = alcCreateContext(ctxt->device, 0);
    alcMakeContextCurrent(ctxt->context);
    alGenSources(16, &ctxt->sources[0]);

    ctxt->available.reserve(16);
    for(int i = 0; i < 16; ++i)
    {
        ctxt->available.push_back(ctxt->sources[i]);
    }
}

void shutdown_audio(audio_context* ctxt)
{
    alDeleteSources(16, ctxt->sources);
    alDeleteBuffers(ALsizei(ctxt->loaded_clips.size()), ctxt->loaded_clips.data());
    alcDestroyContext(ctxt->context);
    alcCloseDevice(ctxt->device);
}

void audio_update(audio_context* ctxt)
{
    alcProcessContext(ctxt->context);
    for(auto it = ctxt->leased.begin(); it != ctxt->leased.end(); ++it)
    {
        int val;
        alGetSourcei(*it, AL_SOURCE_STATE, &val);
        if (val == AL_STOPPED)
        {
            ctxt->available.push_back(*it);
            ctxt->leased.erase(it);
            break;
        }
    }
}

audio_clip_handle load_sound(audio_context* ctxt, const char* path)
{
    auto id = audio_clip_handle(ctxt->loaded_clips.size());
    ALuint clip;
    alGenBuffers(1, &clip);
    ctxt->loaded_clips.push_back(clip);

    std::vector<char> buf;

    {
        auto f = fopen(path, "rb");
        if (!f)
        {
            fprintf(stderr, "Failed to open file %s\n", path);
            exit(-5);
        }
        fseek(f, 0, SEEK_END);
        buf.reserve(size_t(ftell(f)));
        fseek(f, 0, SEEK_SET);
        fread(buf.data(), sizeof(char), buf.capacity(), f);
        fclose(f);
    }

    ALenum format = 0;
    int channels, samplerate, bps, alloc_size;
    
    auto audioDataBuffer = tw_load_mem(buf.data(), buf.capacity(), &channels, &samplerate, &bps, &alloc_size);
    switch(channels)
    {
        case 1:
        {
            switch(bps)
            {
                case 8:
                {
                    format = AL_FORMAT_MONO8;
                    break;
                }
                case 16:
                {
                    format = AL_FORMAT_MONO16;
                    break;
                }
            }
            break;
        }
        case 2:
        {
            switch(bps)
            {
                case 8:
                {
                    format = AL_FORMAT_STEREO8;
                    break;
                }
                case 16:
                {
                    format = AL_FORMAT_STEREO16;
                    break;
                }
            }
            break;
        }
    }

    if (format == 0)
    {
        fprintf(stderr, "Invalid audio clip %s.  Must be mono or stereo with 8 or 16 bits per sample.\n", path);
        exit(-5);
    }

    alBufferData(clip, format, audioDataBuffer, alloc_size, samplerate);
    tw_free(audioDataBuffer);
    return id + 1;
}

void play_sound(audio_context* ctxt, audio_clip_handle clip)
{
    // just miss a sound if we can't play any more
    if (ctxt->available.size() == 0)
    {
        fprintf(stderr, "Audio source pool empty!  Skipping sound.\n");
        return;
    }
    
    auto src = ctxt->available.back();
    ctxt->available.pop_back();
    ctxt->leased.push_back(src);
    alSourcei(src, AL_BUFFER, ctxt->loaded_clips[clip - 1]);
    alSourcePlay(src);
}