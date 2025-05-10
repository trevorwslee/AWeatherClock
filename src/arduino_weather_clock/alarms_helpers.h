#ifndef ALARMS_HELPERS_H
#define ALARMS_HELPERS_H

#include <Arduino.h>


struct Alarm {
  bool enabled = false;
  int16_t hour = 0;        // 0-23
  int16_t minute = 0;      // 0-59
  int16_t weekDayMask = 0; // 0: not repeated ... 1: Sun, 2: Mon, 4: Tue, 8: Wed, 16: Thu, 32: Fri, 64: Sat
};


//int getAlarmCount();
Alarm getAlarm(int idx);
void setAlarm(int idx, Alarm& alarm, const char* printPrefix = nullptr, bool saveAsWell = false);
//void onAlarmsChanged();

void ackAlarmDue();

void alarmsSetup();
bool alarmsLoop();



#endif