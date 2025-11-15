#include <stdio.h>
#include <unistd.h>
#include "hal/audioMixer.h"

int main(void)
{
    printf("Initializing audio mixer...\n");
    AudioMixer_init();
    wavedata_t drum;
    AudioMixer_readWaveFileIntoMemory("beatbox-wave-files/100051__menegass__gui-drum-bd-hard.wav", &drum);
    while(1)
    {   
        AudioMixer_queueSound(&drum);
        sleep(50);
    }
    printf("Cleaning up...\n");
    AudioMixer_freeWaveFileData(&drum);
    AudioMixer_cleanup();
    return 0;
}