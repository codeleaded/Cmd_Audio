#if defined __linux__
#include "/home/codeleaded/System/Static/Library/AudioPlayer.h"
#elif defined _WINE
#include "/home/codeleaded/System/Static/Library/AudioPlayer.h"
#elif defined _WIN32
#include "F:/home/codeleaded/System/Static/Library/AudioPlayer.h"
#endif

#define SAMPLE_RATE             44100
#define CHANNELS                1
#define BITS_PER_SAMPLE         16
#define DURATION_SECONDS        5
#define FRAMES_PER_BUFFER       1024
#define FILENAME_IN             "./data/coin.mp3"
#define FILENAME_OUT            "./data/Rec_out.mp3"

/*
int main(int argc, char *argv[]) {
    IAudio a = IAudio_New(BITS_PER_SAMPLE,FRAMES_PER_BUFFER,CHANNELS,SAMPLE_RATE,500000);
    IAudio_Start(&a);
    
    sleep(DURATION_SECONDS);
    
    IAudio_Stop(&a);  
    IAudio_Write(&a,FILENAME_OUT);
    IAudio_Free(&a);
    printf("record done.\n");
    return 0;
}
*/

int main(int argc, char *argv[]) {
    AudioPlayer ap = AudioPlayer_New();
    if(argc > 1){
        for(int i = 1;i < argc;i++){
            AudioPlayer_Add(&ap,argv[i]);
        }
    }else{
        AudioPlayer_Add(&ap,"./data/coin.mp3");
        AudioPlayer_Add(&ap,"./data/coin.wav");
        AudioPlayer_Add(&ap,"./data/jump.wav");
    }

    AudioPlayer_Start(&ap);
    Thread_Sleep_M(10000);
    AudioPlayer_Free(&ap);

    printf("replay done.\n");
    return 0;
}
