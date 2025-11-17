#ifndef _BEATBOX_H_
#define _BEATBOX_H_

void beatbox_init();
void beatbox_cleanup();

void beatbox_hi_hat();
void beatbox_base();
void beatbox_snare();
void beatbox_cc();
void beatbox_co();
void beatbox_splash();
void beatbox_tom();

int beatbox_bpm();
int beatbox_volume();
int beatbox_track();
void beatbox_set_bpm(int bpm);
void beatbox_set_volume(int volume);
void beatbox_set_track(int track);

#endif