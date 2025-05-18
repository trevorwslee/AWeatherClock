
#ifndef SOUND_HELPERS_H
#define SOUND_HELPERS_H


void playTone(int freq, int duration);

#if defined(ES8311_PA)
void copyStarWars30Data(bool (*checkStopCallback)());
#endif

long soundAlarmBeepWithBuzzer();
void soundAlarm(bool (*checkStopCallback)(), bool preferMusic = false);


void soundSetup();

#endif