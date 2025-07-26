#include "../inc/Audio.h"

#define SAMPLE_RATE         44100
#define CHANNELS            1
#define BITS_PER_SAMPLE     16
#define FORMAT              SND_PCM_FORMAT_S16_LE
#define DURATION_SECONDS    5
#define FRAMES_PER_BUFFER   1024
#define FILENAME            "./data/recording.wav"

// int main(int argc, char *argv[]) {
//     IAudio a = IAudio_New(FORMAT,BITS_PER_SAMPLE,FRAMES_PER_BUFFER,CHANNELS,SAMPLE_RATE,500000);
//     IAudio_Start(&a);

//     sleep(DURATION_SECONDS);

//     IAudio_Stop(&a);
    
//     IAudio_Write(&a,FILENAME);
//     IAudio_Free(&a);

//     printf("record done.\n");

//     return 0;
// }


int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("use: %s <wav-file .wav>\n",argv[0]);
        return 1;
    }

    OAudio a = OAudio_New(FORMAT,BITS_PER_SAMPLE,FRAMES_PER_BUFFER,2,SAMPLE_RATE);
    WavFile wf = WavFile_Read(argv[1],FRAMES_PER_BUFFER);

    OAudio_Play(&a,&wf);

    WavFile_Free(&wf);
    OAudio_Free(&a);

    printf("replay done.\n");
    return 0;
}