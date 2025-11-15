#include <stdio.h>
#include <unistd.h>
#include "hal/audioMixer.h"
#include "hal/time.h"

int main(void)
{
    printf("Initializing audio mixer...\n");
    AudioMixer_init();
    wavedata_t drumbdhard;
    AudioMixer_readWaveFileIntoMemory("beatbox-wave-files/100051__menegass__gui-drum-bd-hard.wav", &drumbdhard);

    wavedata_t snare;
    AudioMixer_readWaveFileIntoMemory("beatbox-wave-files/100058__menegass__gui-drum-snare-hard.wav", &snare);

    while(1)
    {   
        AudioMixer_queueSound(&drumbdhard);
        AudioMixer_queueSound(&snare);
        time_sleep_ms(125);
        AudioMixer_queueSound(&snare);
        time_sleep_ms(125);
        
    }
    printf("Cleaning up...\n");
    AudioMixer_freeWaveFileData(&drumbdhard);
    AudioMixer_freeWaveFileData(&snare);
    AudioMixer_cleanup();
    return 0;
}