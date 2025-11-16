#include "hal/audioMixer.h"
#include "hal/time.h"
#include "hal/encoder.h"
#include "hal/adc.h"
#include <stdio.h>
#include <pthread.h>
#include <math.h>

static pthread_t BeatboxThread;
static pthread_mutex_t BeatboxMutex = PTHREAD_MUTEX_INITIALIZER; // not sure if necessary rn

// beatbox loops through the currently selected 'track'
// basically just iterates through a list of actions at times
// each action occurs (60 / BPM / 2) * 1000 ms apart

enum Beat
{
    none,
    rock,
    custom
} beat = rock;

static int BPM = 120;   // [40,300]
static int Volume = 80; // [0,100]

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

void *beatboxLoop()
{
    float i = 1;
    int lastEncoderPos = Encoder_read();
    int lastEncoderPress = 0;
    AudioMixer_setVolume(Volume);
    while (1)
    {
        // config from hardware input:
        // Encoder Relative BPM change [40,300], increments of 5
        int encoderPos = Encoder_read();
        int delta = encoderPos - lastEncoderPos;
        if (delta != 0)
        {
            BPM += delta * 5;
        }
        lastEncoderPos = encoderPos;

        if (BPM < 40)
        {
            BPM = 40;
        }
        if (BPM > 300)
        {
            BPM = 300;
        }

        // Encoder Press Track Cycle
        int encoderPress = Encoder_button_pressing();
        if (encoderPress && !lastEncoderPress)
        {
            beat++;
            if (beat > 2)
            {
                beat = 0;
            }
        }
        lastEncoderPress = encoderPress;

        // Joystick Volume control [0,100], increments of 5
        // accelerometer -> new acel play beat, hysteresis?
        static int changingVolume;
        static long long changingVolumeT0;
        float joystick = ADC_read_joystick();
        if (joystick > 0.3 || joystick < -0.3)
        {
            if (!changingVolume)
            {
                changingVolumeT0 = time_get_ms();
                Volume += (joystick / fabs(joystick)) * 5;
                if (Volume > 100)
                {
                    Volume = 100;
                }
                if (Volume < 0)
                {
                    Volume = 0;
                }
                AudioMixer_setVolume(Volume);
            }

            long long t = time_get_ms() - changingVolumeT0;
            if (t > 200)
            {
                Volume += (joystick / fabs(joystick)) * 5;
                if (Volume > 100)
                {
                    Volume = 100;
                }
                if (Volume < 0)
                {
                    Volume = 0;
                }
                AudioMixer_setVolume(Volume);
                changingVolumeT0 = time_get_ms();
            }

            changingVolume = 1;
        }
        else
        {
            changingVolume = 0;
        }

        printf("BPM: %d, %d, M%d, %.2f, X %5.2f, Y %5.2f, Z: %5.2f\n", BPM, Volume, beat, ADC_read_joystick(), ADC_read_acel_x(), ADC_read_acel_y(), ADC_read_acel_z());

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

        // ensure BPM isn't changed at a critical point
        pthread_mutex_lock(&BeatboxMutex);
        time_sleep_ms(1000 * (60.0 / BPM / 2.0));
        pthread_mutex_unlock(&BeatboxMutex);
    }
}

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

    pthread_create(&BeatboxThread, NULL, beatboxLoop, NULL);
}

void beatboxCleanup()
{
    printf("Cleaning up...\n");
    AudioMixer_freeWaveFileData(&hi_hat);
    AudioMixer_freeWaveFileData(&base);
    AudioMixer_freeWaveFileData(&snare);
    AudioMixer_freeWaveFileData(&cc);
    AudioMixer_freeWaveFileData(&co);
    AudioMixer_freeWaveFileData(&splash);
    AudioMixer_freeWaveFileData(&tom);
    AudioMixer_cleanup();
    pthread_mutex_destroy(&BeatboxMutex);
    pthread_cancel(BeatboxThread);
}