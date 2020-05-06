/*	tinywav.h - v0.1 - public domain wav loader
								no warranty implied; use at your own risk

	Make sure to
		#define TINY_WAV_IMPLEMENTATION
		#include <tiny_wav.h>

	before inclusion in exactly one source file.

	To use, simply call tw_load_mem to get the wav file parameters.

*/

#ifndef __TINYWAV_H
#define __TINYWAV_H
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
void* tw_malloc(size_t);
void tw_free(void*);

char* tw_load_mem(const char* mem, size_t size, int* channels, int* samplerate, int* bps, int* alloc_size);
const char* tw_get_error();
const int tw_has_error();
#ifdef __cplusplus
}
#endif// __cplusplus

#if defined(TINY_WAV_IMPLEMENTATION)

#if defined(_MSC_VER) || defined(_MSVC_LANG)	// if this is msvc
#pragma warning(disable:4996)					// disable the _CRT_SECURE_NO_WARNINGS warning so we can use sprintf
#endif	// #if is msvc

#if !defined(TINY_WAV_ALLOC_OVERRIDE)
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct
{
	char chunk_id[4];	// big endian "RIFF"
	uint32_t chunk_size;
	char format[4];		// big endian "WAVE"
} WavRIFFHeader;

typedef struct
{
	char subchunk_id[4];// big endian "fmt(0x20)"
	uint32_t size;
	uint16_t format;
	uint16_t channels;
	uint32_t sample_rate;
	uint32_t byte_rate;
	uint16_t block_align;
	uint16_t bits_per_sample;
} WavFormatChunk;


typedef struct
{
	char subchunk_id[4];//big endian "data"
	uint32_t size;
} WavDataChunk;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
void* tw_malloc(size_t size)
{
	return malloc(size);
}

void tw_free(void* ptr)
{
	free(ptr);
}
#ifdef __cplusplus
}
#endif // __cplusplus
#endif // !defined(TINY_WAV_ALLOC_OVERRIDE)

static int cmp_chunk_id_l(const char* lhs, const char* rhs, int len)
{
	for (int i = 0; i < len; ++i)
	{
		if (lhs[i] != rhs[i])
			return 1;
	}
	return 0;
}

static int cmp_chunk_id(const char* lhs, const char* rhs)
{
	return cmp_chunk_id_l(lhs, rhs, 4);
}

static char tw_error_str[256] = {0};
static const char* format_strings[] = {
	"invalid",
	"PCM",
	"IEEE float",
	"ALAW",
	"MULAW",
	"Extensible"
};

enum _WAV_FORMATS
{
	WAV_FORMAT_INVALID,
	WAV_FORMAT_PCM,
	WAV_FORMAT_IEEE_FLOAT,
	WAV_FORMAT_ALAW,
	WAV_FORMAT_MULAW,
	WAV_FORMAT_EXTENSIBLE,
	WAV_FORMAT_COUNT
};

#ifdef __cplusplus
extern "C" {
#endif	// __cplusplus
const char* tw_get_error()
{
	
	return tw_error_str;
}

const int tw_has_error()
{
	return tw_error_str[0];
}

char* tw_load_mem(const char* mem, size_t size, int* channels, int* samplerate, int* bps, int* alloc_size)
{
	// Reset error
	memset(tw_error_str, 0, sizeof(tw_error_str));
	
	WavRIFFHeader riff;
	WavFormatChunk format;
	WavDataChunk data;

	// Read RIFF chunk
	size_t offset = 0;
	memcpy(&riff, mem + offset, sizeof(WavRIFFHeader));
	offset += sizeof(WavRIFFHeader);

	if (cmp_chunk_id(riff.chunk_id, "RIFF") != 0)
	{
		sprintf(tw_error_str, "RIFF chunk invalid");
		return NULL;
	}

	if (cmp_chunk_id(riff.format, "WAVE") != 0)
	{
		sprintf(tw_error_str, "Not a WAV file - RIFF chunk not in WAVE format");
		return NULL;
	}

	// Read format chunk
	memcpy(&format, mem + offset, sizeof(WavFormatChunk));
	offset += sizeof(WavDataChunk);
	offset += format.size;

	if (cmp_chunk_id(format.subchunk_id, "fmt\x20") != 0)
	{
		sprintf(tw_error_str, "Malformed WAV file - format chunk invalid");
		return NULL;
	}

	// TODO: Implement other wav formats
	if (format.format != 1)
	{
		if (format.format >= WAV_FORMAT_COUNT)
		{
			sprintf(tw_error_str, "Unknown wav format - only PCM currently supported");
		}
		else
		{
			sprintf(tw_error_str, "Unsupported wav format %s - only PCM currently supported", format_strings[format.format]);
		}
		return NULL;
	}

	// Read data chunk (sans data)
	memcpy(&data, mem + offset, sizeof(WavDataChunk));
	offset += sizeof(WavDataChunk);

	// If we have any junk chunks, ignore them
	while (cmp_chunk_id(data.subchunk_id, "junk") == 0)
	{
		// TODO: Figure out junk chunk
		offset += data.size;
		memcpy(&data, mem + offset, sizeof(WavDataChunk));
		offset += sizeof(WavDataChunk);
	}

	// Ignore any pad chunks
	while (cmp_chunk_id_l(data.subchunk_id, "pad", 3) == 0)
	{
		// TODO: Figure out junk chunk
		offset += data.size;
		memcpy(&data, mem + offset, sizeof(WavDataChunk));
		offset += sizeof(WavDataChunk);
	}

	if (cmp_chunk_id(data.subchunk_id, "data") != 0)
	{
		sprintf(tw_error_str, "Malformed WAV file - data chunk invalid");
		return NULL;
	}

	if (offset + data.size > size)
	{
		sprintf(tw_error_str, "Attempted to read out of memory bounds");
		return NULL;
	}

	char* data_ptr = (char*)tw_malloc(data.size);
	memcpy(data_ptr, mem + offset, data.size);

	if (channels)
	{
		*channels = (int)format.channels;
	}

	if (samplerate)
	{
		*samplerate = (int)format.sample_rate;
	}

	if (bps)
	{
		*bps = (int)format.bits_per_sample;
	}

	if (alloc_size)
	{
		*alloc_size = (int)data.size;
	}

	return data_ptr;
}
#ifdef __cplusplus
}
#endif	// __cplusplus
#endif	// TINY_WAV_IMPLEMENTATION
#endif	// __TINYWAV_H