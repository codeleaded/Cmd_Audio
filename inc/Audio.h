#ifndef AUDIO_H
#define AUDIO_H

#include "Thread.h"
#include "AlxTime.h"
#include "DataStream.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alsa/asoundlib.h>


typedef struct WavRiffHeader {
    char chunkId[4]; // "RIFF"
    uint32_t chunkSize;
    char format[4]; // "WAVE"
} WavRiffHeader;

typedef struct WavSubchunkHeader {
    char subchunkId[4]; // "fmt " oder "data"
    uint32_t subchunkSize;
} WavSubchunkHeader;

typedef struct WavFmtChunk {
    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
} WavFmtChunk;

typedef struct WavFile{
    WavRiffHeader riffHeader;
    WavSubchunkHeader sch;
    WavFmtChunk fmtChunk;
    uint32_t dataSize;
    uint32_t frame_size;
    long dataOffset;
    int foundFmt;
    int foundData;
    char *buffer;
} WavFile;

WavFile WavFile_Null(){
    WavFile wf;
    memset(&wf,0,sizeof(WavFile));
    return wf;
}
WavFile WavFile_New(int sample_rate,int bits_per_sample,int channels){
    WavFile wf;

    int byte_rate = sample_rate * channels * bits_per_sample / 8;
    int block_align = channels * bits_per_sample / 8;

    uint32_t chunk_size = 36;

    memcpy(wf.riffHeader.chunkId,"RIFF",4);
    wf.riffHeader.chunkSize = chunk_size;
    memcpy(wf.riffHeader.format,"WAVE",4);

    uint32_t subchunk1_size = 16;
    uint16_t audio_format = 1; // PCM

    memcpy(wf.sch.subchunkId,"fmt ",4);
    wf.sch.subchunkSize = subchunk1_size;

    wf.fmtChunk.audioFormat = audio_format;
    wf.fmtChunk.numChannels = channels;
    wf.fmtChunk.sampleRate = sample_rate;
    wf.fmtChunk.byteRate = byte_rate;
    wf.fmtChunk.blockAlign = block_align;
    wf.fmtChunk.bitsPerSample = bits_per_sample;

    wf.frame_size = 0;
    wf.dataSize = 0;
    wf.buffer = NULL;

    return wf;
}
WavFile WavFile_Make(int sample_rate,int bits_per_sample,int channels,char* data,int frames_recorded,int frame_size){
    WavFile wf = WavFile_New(sample_rate,bits_per_sample,channels);

    wf.frame_size = frame_size;

    int subchunk2_size = frames_recorded * frame_size;
    uint32_t chunk_size = 36 + subchunk2_size;

    wf.riffHeader.chunkSize = chunk_size;
    wf.dataSize = subchunk2_size;

    wf.buffer = malloc(wf.dataSize);
    memcpy(&wf.buffer,data,wf.dataSize);

    return wf;
}
WavFile WavFile_Move(int sample_rate,int bits_per_sample,int channels,char* data,int frames_recorded,int frame_size){
    WavFile wf = WavFile_New(sample_rate,bits_per_sample,channels);

    wf.frame_size = frame_size;

    int subchunk2_size = frames_recorded * frame_size;
    uint32_t chunk_size = 36 + subchunk2_size;

    wf.riffHeader.chunkSize = chunk_size;
    wf.dataSize = subchunk2_size;

    wf.buffer = data;
    return wf;
}
void WavFile_Write(WavFile* wf,char* Path){
    FILE* file = fopen(Path,"wb");
    if (!file) {
        printf("[WavFile]: Write -> file \"%s\" couldn't open!\n",Path);
        return;
    }

    fwrite(wf->riffHeader.chunkId,1,4,file);
    fwrite(&wf->riffHeader.chunkSize,4,1,file);
    fwrite(wf->riffHeader.format,1,4,file);

    fwrite(&wf->sch.subchunkId,1,4,file);
    fwrite(&wf->sch.subchunkSize,4,1,file);
    fwrite(&wf->fmtChunk.audioFormat,2,1,file);
    fwrite(&wf->fmtChunk.numChannels,2,1,file);
    fwrite(&wf->fmtChunk.sampleRate,4,1,file);
    fwrite(&wf->fmtChunk.byteRate,4,1,file);
    fwrite(&wf->fmtChunk.blockAlign,2,1,file);
    fwrite(&wf->fmtChunk.bitsPerSample,2,1,file);

    fwrite("data",1,4,file);
    fwrite(&wf->dataSize,4,1,file);
    fwrite(wf->buffer,wf->frame_size,wf->dataSize / wf->frame_size,file);

    fclose(file);
}
WavFile WavFile_Read(char* Path,int frame_size){
    WavFile wf = WavFile_Null();
    
    FILE* file = fopen(Path,"rb");
    if (!file) {
        printf("[WavFile]: Read -> Couldn't open \"%s\"!\n", Path);
        return WavFile_Null();
    }

    long a = fread(&wf.riffHeader, sizeof(WavRiffHeader), 1, file);
    if (strncmp(wf.riffHeader.chunkId, "RIFF", 4) != 0 || strncmp(wf.riffHeader.format, "WAVE", 4) != 0) {
        printf("[WavFile]: Read -> no valid wave or riff: audio file \"%s\"!\n",Path);
        fclose(file);
        return WavFile_Null();
    }

    wf.fmtChunk = (WavFmtChunk){0};
    wf.dataSize = 0;
    wf.dataOffset = 0;
    wf.foundFmt = 0,
    wf.foundData = 0;
    wf.frame_size = frame_size;

    while(!wf.foundFmt || !wf.foundData){
        WavSubchunkHeader subchunk;
        if (fread(&subchunk, sizeof(WavSubchunkHeader), 1, file) != 1) {
            printf("[WavFile]: Read -> error during read of subchunk of audio file \"%s\"!\n",Path);
            fclose(file);
            return WavFile_Null();
        }

        if (strncmp(subchunk.subchunkId, "fmt ", 4) == 0) {
            a = fread(&wf.fmtChunk, sizeof(WavFmtChunk), 1, file);
            wf.foundFmt = 1;
            if (subchunk.subchunkSize > sizeof(WavFmtChunk)) {
                fseek(file, subchunk.subchunkSize - sizeof(WavFmtChunk), SEEK_CUR);
            }
        } else if (strncmp(subchunk.subchunkId, "data", 4) == 0) {
            wf.dataSize = subchunk.subchunkSize;
            wf.dataOffset = ftell(file);
            wf.foundData = 1;
            fseek(file,wf.dataSize,SEEK_CUR);
        } else {
            fseek(file,subchunk.subchunkSize,SEEK_CUR);
        }
    }

    if (!wf.foundFmt || !wf.foundData) {
        fprintf(stderr, "[WavFile]: Read -> didn't find fmt or data Chunk of audio file \"%s\"!\n",Path);
        fclose(file);
        return WavFile_Null();
    }

    wf.buffer = malloc(wf.dataSize);
    fseek(file, wf.dataOffset, SEEK_SET);
    if (fread(wf.buffer, 1, wf.dataSize, file) != wf.dataSize) {
        fprintf(stderr, "[WavFile]: Read -> audio read failed during read of audio file \"%s\"!\n",Path);
        return WavFile_Null();
    }

    return wf;
}
void WavFile_Free(WavFile* wf){
    if(wf->buffer) free(wf->buffer);
    memset(wf,0,sizeof(wf));
}


typedef short AudioSample;

typedef struct OAudio{
    snd_pcm_t *pcm_handle;
    snd_pcm_hw_params_t *params;
    int err;
    int numChannels;
    int rate;
    int bits;
    snd_pcm_uframes_t frames;
    size_t bytes_per_frame;
    size_t total_frames;
    snd_pcm_uframes_t buffer_size;
} OAudio;

OAudio OAudio_Null(){
    OAudio a;
    a.pcm_handle = NULL;
    a.params = NULL;
    a.err = 0;
    a.numChannels = 0;
    a.rate = 0;
    a.bits = 0;
    a.frames = 0;
    a.bytes_per_frame = 0;
    a.total_frames = 0;
    a.buffer_size = 0;
    return a;
}
OAudio OAudio_New(enum _snd_pcm_format format,int bits,int frames,unsigned int channels,unsigned int rate){
    OAudio a = OAudio_Null();

    if ((a.err = snd_pcm_open(&a.pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        fprintf(stderr, "[OAudio]: Couldn't open PCM-Device: %s\n", snd_strerror(a.err));
        return OAudio_Null();
    }

    snd_pcm_hw_params_alloca(&a.params);
    if ((a.err = snd_pcm_hw_params_any(a.pcm_handle, a.params)) < 0) {
        fprintf(stderr, "[OAudio]: Couldn't open PCM-Device: %s\n", snd_strerror(a.err));
        return OAudio_Null();
    }

    a.bits = bits;
    a.numChannels = channels;

    if ((a.err = snd_pcm_hw_params_set_access(a.pcm_handle, a.params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        fprintf(stderr, "[OAudio]: Couldn't open PCM-Device: %s\n", snd_strerror(a.err));
        return OAudio_Null();
    }
    if ((a.err = snd_pcm_hw_params_set_format(a.pcm_handle, a.params, format)) < 0) {
        fprintf(stderr, "[OAudio]: Couldn't open PCM-Device: %s\n", snd_strerror(a.err));
        return OAudio_Null();
    }
    if ((a.err = snd_pcm_hw_params_set_channels(a.pcm_handle, a.params, a.numChannels)) < 0) {
        fprintf(stderr, "[OAudio]: Couldn't open PCM-Device: %s\n", snd_strerror(a.err));
        return OAudio_Null();
    }

    a.rate = rate;
    if ((a.err = snd_pcm_hw_params_set_rate_near(a.pcm_handle, a.params, &a.rate, 0)) < 0) {
        fprintf(stderr, "[OAudio]: Couldn't open PCM-Device: %s\n", snd_strerror(a.err));
        return OAudio_Null();
    }

    if ((a.err = snd_pcm_hw_params_set_periods(a.pcm_handle, a.params, 4, 0)) < 0) {
        fprintf(stderr, "[OAudio]: Couldn't open PCM-Device: %s\n", snd_strerror(a.err));
        return OAudio_Null();
    }
    
    a.frames = frames;
    a.buffer_size = frames * channels * (bits / 8);
    if ((a.err = snd_pcm_hw_params_set_buffer_size_near(a.pcm_handle, a.params, &a.buffer_size)) < 0) {
        fprintf(stderr, "[OAudio]: Couldn't open PCM-Device: %s\n", snd_strerror(a.err));
        return OAudio_Null();
    }

    if ((a.err = snd_pcm_hw_params(a.pcm_handle, a.params)) < 0) {
        fprintf(stderr, "Kann Hardware-Parameter nicht setzen: %s\n", snd_strerror(a.err));
        snd_pcm_close(a.pcm_handle);
        return OAudio_Null();
    }
    
    return a;
}
void OAudio_Write(OAudio* a,char* buffer,int dataSize){
    a->bytes_per_frame = a->numChannels * (a->bits / 8);
    
    if(a->bytes_per_frame==0){
        printf("[OAudio]: Write -> bytes_per_frame are 0\n");
        return;
    }
    a->total_frames = dataSize / a->bytes_per_frame;

    for (size_t i = 0; i < a->total_frames; i += a->frames) {
        size_t frames_to_write = (i + a->frames > a->total_frames) ? a->total_frames - i : a->frames;
        a->err = snd_pcm_writei(a->pcm_handle, buffer + i * a->bytes_per_frame, frames_to_write);
        if (a->err < 0) {
            if (a->err == -EPIPE) {
                printf("[OAudio]: Write -> bufferoverflow, reseting...\n");
                snd_pcm_prepare(a->pcm_handle);
            } else {
                printf("[OAudio]: Write -> error during write: %s\n", snd_strerror(a->err));
                break;
            }
        } else if ((size_t)a->err != frames_to_write) {
            printf("[OAudio]: Write -> short write: %d of %ld frames\n", a->err, frames_to_write);
        }
    }
}
void OAudio_Adapt(OAudio* a,WavFile* wf){
    a->numChannels = wf->fmtChunk.numChannels;
    a->frames = wf->frame_size;
    a->bits =wf->fmtChunk.bitsPerSample;

    // snd_pcm_hw_params_set_format(a->pcm_handle, a->params, wf->fmtChunk.bitsPerSample == 16 ? SND_PCM_FORMAT_S16_LE : SND_PCM_FORMAT_U8);
    // snd_pcm_hw_params_set_channels(a->pcm_handle, a->params, wf->fmtChunk.numChannels);
    // unsigned int rate = wf->fmtChunk.sampleRate;
    // snd_pcm_hw_params_set_rate_near(a->pcm_handle, a->params, &rate, 0);
}
void OAudio_Play(OAudio* a,WavFile* wf){
    OAudio_Adapt(a,wf);
    OAudio_Write(a,wf->buffer,wf->dataSize);
}
void OAudio_Free(OAudio* a){
    snd_pcm_drain(a->pcm_handle);
    snd_pcm_close(a->pcm_handle);
}


typedef struct IAudio{
    snd_pcm_t *pcm_handle;
    Thread thread;
    char running;
    DataStream buffer;
    enum _snd_pcm_format format;
    int bits;
    int frames_buffer;
    unsigned int channels;
    unsigned int rate;
    unsigned int latency;
} IAudio;

IAudio IAudio_Null(){
    IAudio a;
    a.pcm_handle = NULL;
    a.thread = Thread_Null();
    a.running = 0;
    a.buffer = DataStream_Null();
    a.format = 0;
    a.bits = 0;
    a.frames_buffer = 0;
    a.channels = 0;
    a.rate = 0;
    a.latency = 0;
    return a;
}
IAudio IAudio_New(enum _snd_pcm_format format,int bits,int frames_buffer,unsigned int channels,unsigned int rate,unsigned int latency){
    IAudio a = IAudio_Null();

    a.thread = Thread_Null();
    a.running = 0;
    a.buffer = DataStream_New(sizeof(char));
    a.format = format;
    a.bits = bits;
    a.frames_buffer = frames_buffer;
    a.channels = channels;
    a.rate = rate;
    a.latency = latency;

    snd_pcm_open(&a.pcm_handle, "default", SND_PCM_STREAM_CAPTURE, 0);
    snd_pcm_set_params(a.pcm_handle,
                       format,
                       SND_PCM_ACCESS_RW_INTERLEAVED,
                       channels,
                       rate,
                       1,
                       latency); // 0.5 Sekunden Latenz
    
    return a;
}
void* IAudio_Execute(IAudio* a){
    int frame_size = a->bits / 8 * a->channels;
    int16_t* buffer = malloc(a->frames_buffer * a->channels * sizeof(int16_t));

    Timepoint start = Time_Nano();

    while (a->running) {
        int frames_to_read = a->frames_buffer;
        int err = snd_pcm_readi(a->pcm_handle, buffer, frames_to_read);
        if (err < 0) {
            fprintf(stderr, "Error reading audio: %s\n", snd_strerror(err));
            break;
        }
        DataStream_PushCount(&a->buffer,buffer,frame_size * err);
    }

    if(buffer) free(buffer);
    buffer = NULL;
    return NULL;
}
void IAudio_Start(IAudio* a){
    if(a->running==0){
        a->running = 1; 
        a->thread = Thread_New(NULL,(void*)IAudio_Execute,a);
        Thread_Start(&a->thread);
    }else{
        printf("[IAudio]: Start -> can't start because its already running!\n");
    }
}
void IAudio_Stop(IAudio* a){
    if(a->running==1){
        a->running = 0;
        Thread_Join(&a->thread,NULL);
        Thread_Cancel(&a->thread,NULL);
    }else{
        printf("[IAudio]: Stop -> can't stop because it already stopped!\n");
    }
}
void IAudio_Clear(IAudio* a){
    DataStream_Clear(&a->buffer);
}
void IAudio_Write(IAudio* a,char* Path){
    int frame_size = a->bits / 8 * a->channels;
    WavFile wf = WavFile_Move(a->rate,a->bits,a->channels,a->buffer.Memory,a->buffer.size / frame_size,frame_size);
    WavFile_Write(&wf,Path);
}
void IAudio_Free(IAudio* a){
    DataStream_Free(&a->buffer);
    snd_pcm_close(a->pcm_handle);
}


#endif//!AUDIO_H