
#ifndef SOUND_HELPERS_H
#define SOUND_HELPERS_H


enum class AlarmPreferredType {
  Beeps,
  Melody,
  Music,
};


void playTone(int freq, int duration);

#if defined(ES8311_PA)
void copyStarWars30Data(bool (*checkStopCallback)());
#endif

void playAlarmBeep();
void soundAlarm(bool (*checkStopCallback)(), AlarmPreferredType preferType = AlarmPreferredType::Beeps, int param = -1);


void soundSetup();

#endif