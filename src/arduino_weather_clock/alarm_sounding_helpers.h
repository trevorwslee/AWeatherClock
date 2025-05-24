#ifndef ALARM_SOUNDING_HELPERS_H
#define ALARM_SOUNDING_HELPERS_H

#include "sys_config.h"

#if defined(USE_TASK_FOR_ALARM_SOUND)

bool isSoundingAlarmSound();
void setSoundingAlarmSoundIdx(int alarmSoundIdx);

void alarmSoundingSetup();

#endif


#endif