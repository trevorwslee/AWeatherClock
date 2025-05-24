#ifndef ALARMS_HELPERS_H
#define ALARMS_HELPERS_H

#include <Arduino.h>
#include "sys_config.h"

struct AlarmV0 {
  bool enabled = false;
  int16_t hour = 0;        // 0-23
  int16_t minute = 0;      // 0-59
  int16_t weekDayMask = 0; // 0: not repeated ... 1: Sun, 2: Mon, 4: Tue, 8: Wed, 16: Thu, 32: Fri, 64: Sat
};

// struct Alarm {
//   bool enabled = false;
//   int16_t hour = 0;        // 0-23
//   int16_t minute = 0;      // 0-59
//   int16_t weekDayMask = 0; // 0: not repeated ... 1: Sun, 2: Mon, 4: Tue, 8: Wed, 16: Thu, 32: Fri, 64: Sat
// };

struct Alarm {
  bool enabled = false;
  int16_t hour = 0;        // 0-23
  int16_t minute = 0;      // 0-59
  int16_t weekDayMask = 0; // 0: not repeated ... 1: Sun, 2: Mon, 4: Tue, 8: Wed, 16: Thu, 32: Fri, 64: Sat
  int8_t alarmSoundIdx = 0;
};



//int getAlarmCount();
Alarm getAlarm(int idx);
void setAlarm(int idx, Alarm& alarm, const char* printPrefix = nullptr, bool saveAsWell = false);
//void onAlarmsChanged();

void ackAlarmDue();


int getAlarmSoundSelectCount();
const char* getAlarmSoundSelectText(int alarmSoundIdx);


#if defined(USE_TASK_FOR_ALARM_SOUND)
// bool isSoundingAdhocMelody();
// bool startAdhocSoundMelody(int melodyIdx);
// void stopAdhocSoundMelody();
void adhocStartAlarmSound(int alarmSoundIdx);
bool isSoundingAlarm();
void stopAlarmSound();
#endif

void alarmsSetup();
bool alarmsLoop();



#endif