#include "hal/audioMixer.h"
#include "hal/time.h"
#include <stdio.h>

// beatbox loops through the currently selected 'track'
// basically just iterates through a list of actions at times
// each action occurs (60 / BPM / 2) * 1000 ms apart

enum Beat
{
    none,
    rock,
    custom
} beat = custom;

static int BPM = 180; // [40,300]


wavedata_t hi_hat;
wavedata_t base;
wavedata_t snare;
wavedata_t cc;
wavedata_t co;
wavedata_t splash;
wavedata_t tom;

void beatbox_hi_hat()
{
    AudioMixer_queueSound(&hi_hat);
}
void beatbox_base()
{
    AudioMixer_queueSound(&base);
}
void beatbox_snare()
{
    AudioMixer_queueSound(&snare);
}
void beatbox_cc()
{
    AudioMixer_queueSound(&cc);
}
void beatbox_co()
{
    AudioMixer_queueSound(&co);
}
void beatbox_splash()
{
    AudioMixer_queueSound(&splash);
}
void beatbox_tom()
{
    AudioMixer_queueSound(&tom);
}

static void rock_beat(float beat)
{
    beatbox_hi_hat();
    if (beat == 1 || beat == 3)
    {
        beatbox_base();
    }
    if (beat == 2 || beat == 4)
    {
        beatbox_snare();
    }
}

static void custom_beat(float beat)
{
    beatbox_hi_hat();

    if (beat == 1.5 || beat == 3 || beat == 4.5 || beat == 5.5 || beat == 7 || beat == 8.5)
    {
        beatbox_snare();
    }

    if (beat == 1.0 || beat == 2.5 || beat == 3.5 || beat == 5 || beat == 6.5 || beat == 7.5)
    {
        beatbox_base();
    }

    if (beat == 5)
    {
        beatbox_cc();
    }
}

static float current_beat_length = 4.5;

void beatboxInit()
{
    AudioMixer_init();
    AudioMixer_readWaveFileIntoMemory("beatbox-wave-files/100053__menegass__gui-drum-cc.wav", &hi_hat);
    AudioMixer_readWaveFileIntoMemory("beatbox-wave-files/100051__menegass__gui-drum-bd-hard.wav", &base);
    AudioMixer_readWaveFileIntoMemory("beatbox-wave-files/100059__menegass__gui-drum-snare-soft.wav", &snare);
    AudioMixer_readWaveFileIntoMemory("beatbox-wave-files/100053__menegass__gui-drum-cc.wav", &cc);
    AudioMixer_readWaveFileIntoMemory("beatbox-wave-files/100055__menegass__gui-drum-co.wav", &co);
    AudioMixer_readWaveFileIntoMemory("beatbox-wave-files/100060__menegass__gui-drum-splash-hard.wav", &splash);
    AudioMixer_readWaveFileIntoMemory("beatbox-wave-files/100062__menegass__gui-drum-tom-hi-hard.wav", &tom);

    float i = 1;
    while (1)
    {

        switch (beat)
        {
        case none:
            current_beat_length = 4.5;
            break;

        case rock:
            current_beat_length = 4.5;
            rock_beat(i);
            break;

        case custom:
            current_beat_length = 8.5;
            custom_beat(i);
            break;

        default:
            break;
        }

        i += 0.5;
        if (i > current_beat_length)
        {
            i = 1;
        }
        time_sleep_ms(1000 * (60.0 / BPM / 2.0));
    }

    printf("Cleaning up...\n");
    AudioMixer_freeWaveFileData(&hi_hat);
    AudioMixer_freeWaveFileData(&base);
    AudioMixer_freeWaveFileData(&snare);
    AudioMixer_freeWaveFileData(&cc);
    AudioMixer_freeWaveFileData(&co);
    AudioMixer_freeWaveFileData(&splash);
    AudioMixer_freeWaveFileData(&tom);
    AudioMixer_cleanup();
}