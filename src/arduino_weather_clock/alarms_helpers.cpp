#include "alarms_helpers.h"
#include "currtime_helpers.h"
#include "config.h"
#include "eeprom_helpers.h"
#include "sound_helpers.h"
#include "screen_helpers.h"


#if defined(FOR_TWATCH)  
  #include <LilyGoWatch.h>
#endif
#include "melody_helpers.h"
#include "alarm_sounding_helpers.h"


#define LOG_CHECK_ALARM               true
#define BLACK_OUT_MILLIS              (1000 * 30)
#define CHECK_ALARM_ALLOWANCE_SECONDS -15



Alarm _alarms[NUM_ALARMS];
int _nextAlarmDueIdx = -1;
int16_t _checkedNextAlarmIdxDay = -1;
int _dueAlarmIdx = -1;
long _alarmDueMillis = -1;
CurrTime _lastAlarmDueTime;
long _lastAlarmBeepMillis;
bool _lastDisplayInverted = false;




Alarm getAlarm(int idx) {
  return _alarms[idx];
}
void setAlarm(int idx, Alarm& alarm, const char* printPrefix, bool saveAsWell) {
  _alarms[idx] = alarm;
  _checkedNextAlarmIdxDay = -1;
  if (printPrefix != nullptr) {
    Serial.print(printPrefix);
    Serial.print("alarm [");
    Serial.print(idx);
    Serial.print("] -- ");
    Serial.print(alarm.enabled ? "enabled" : "disabled");
    Serial.print(", weekDayMask: ");
    Serial.print(alarm.weekDayMask, BIN);
    Serial.print(", alarmSoundIdx: ");
    Serial.print(alarm.alarmSoundIdx);
    Serial.print(", time: ");
    Serial.print(alarm.hour);
    Serial.print(":");
    Serial.println(alarm.minute);

  }
  if (saveAsWell) {
    eeprompt_saveAlarm(idx);
  }
}


void ackAlarmDue() {
#if defined(USE_TASK_FOR_ALARM_SOUND)
  //_alarmSoundingWithTask = false;
  setSoundingAlarmSoundIdx(-1);
#endif
  _dueAlarmIdx = -1;
  _alarmDueMillis = -1;
  _checkedNextAlarmIdxDay = -1;
  if (_lastDisplayInverted) {
    void* displayHandle = getDisplayHandle();
    invertDisplay(displayHandle, false);
    _lastDisplayInverted = false;
  }
 
}

int getAlarmSoundSelectCount() {
  int selectionCount = 1;
#if defined(USE_TASK_FOR_ALARM_SOUND)
  selectionCount += NumMelodies;
  #if defined(ES8311_PA)
  selectionCount += 1;
  #endif
#endif
  return selectionCount;
}
const char* getAlarmSoundSelectText(int alarmSoundIdx) {
#if defined(USE_TASK_FOR_ALARM_SOUND)
  if (alarmSoundIdx > 0) {
    int melodyIdx = alarmSoundIdx - 1;
    if (melodyIdx >= 0 && melodyIdx < NumMelodies) {
      return Melodies[melodyIdx].name;
    }
  #if defined(ES8311_PA)
    return "Star Wars";
  #endif  
  }
#endif  
  return "Beep"; 
}




#if defined(USE_TASK_FOR_ALARM_SOUND)
void adhocStartAlarmSound(int alarmSoundIdx) {
  Serial.print("!!! adhocStartAlarmSound ... alarmSoundIdx: ");
  Serial.println(alarmSoundIdx);
  setSoundingAlarmSoundIdx(alarmSoundIdx);
}
bool isSoundingAlarm() {
  return isSoundingAlarmSound();
}
void stopAlarmSound() {
  Serial.println("!!! stopAlarmSound");
  setSoundingAlarmSoundIdx(-1);
}
#endif




void alarmsSetup() {
#if defined(USE_TASK_FOR_ALARM_SOUND)
  alarmSoundingSetup();
#endif
}


int checkNextAlarmDueIdx(CurrTime& currTime) {
  if (LOG_CHECK_ALARM) {
    Serial.print("$$$ check alarms @ ");
    Serial.print(currTime.year);
    Serial.print("-");
    Serial.print(currTime.month);
    Serial.print("-");
    Serial.print(currTime.day);
    Serial.print(" ");
    Serial.print(currTime.hour);
    Serial.print(":");
    Serial.print(currTime.minute);
    Serial.print(":");
    Serial.print(currTime.second);
    Serial.print(", weekDay: ");
    Serial.println(currTime.weekDay);
  }
  int futureSeconds[NUM_ALARMS];
  for (int i = 0; i < NUM_ALARMS; i++) {
    Alarm& alarm = _alarms[i];
    int futureSecond = 100000;
    futureSeconds[i] = futureSecond;
    if (alarm.enabled) {
      if (alarm.weekDayMask == 0 || (alarm.weekDayMask & (1 << currTime.weekDay))) {
        int16_t checkAlarmMinute;
        if (currTime.day == _lastAlarmDueTime.day && alarm.hour == _lastAlarmDueTime.hour) {
          checkAlarmMinute = _lastAlarmDueTime.minute;
        } else {
          checkAlarmMinute = -1;
        }
        if (checkAlarmMinute == -1 || alarm.minute > checkAlarmMinute) {
          futureSecond = (alarm.hour - currTime.hour) * 3600 + (alarm.minute - currTime.minute) * 60;
        }
      }
    }
    if (futureSecond > CHECK_ALARM_ALLOWANCE_SECONDS) {
      futureSeconds[i] = futureSecond;
    }
  }
  int nextAlarmDueIdx = -1;
  int minFutureSecond = 100000;
  for (int i = 0; i < NUM_ALARMS; i++) {
    if (futureSeconds[i] < minFutureSecond) {
      minFutureSecond = futureSeconds[i];
      nextAlarmDueIdx = i;
    }
    if (LOG_CHECK_ALARM) {
      Alarm& alarm = _alarms[i];
      Serial.print("$$$ - check alarm [");
      Serial.print(i);
      Serial.print("]: ");
      Serial.print(alarm.enabled ? "enabled" : "disabled");
      Serial.print(", weekDayMask: ");
      Serial.print(alarm.weekDayMask, BIN);
      Serial.print(", time: ");
      Serial.print(alarm.hour);
      Serial.print(":");
      Serial.print(alarm.minute);
      Serial.print(", futureSecond: ");
      Serial.println(futureSeconds[i]);
    }
  }
  if (LOG_CHECK_ALARM) {
    if (nextAlarmDueIdx != -1) {
      Alarm& alarm = _alarms[nextAlarmDueIdx];
      Serial.print("$$$ ==> today's 'next alarm' [");
      Serial.print(nextAlarmDueIdx);
      Serial.print("] -- ");
      Serial.print(alarm.hour);
      Serial.print(":");
      Serial.println(alarm.minute);
    } else {
      Serial.println("$$$ ==> no 'next alarm' for today");
    }
  }
  return nextAlarmDueIdx;
}

void _forceSetDebugAlarms() {
  Serial.println("$$$ force set debug alarms");
  Serial.println("$$$ force set debug alarms");
  Serial.println("$$$ force set debug alarms");
  if (true) {
    Alarm& alarm = _alarms[0];
    CurrTime currTime = getCurrTime();
    alarm.enabled = true;
    alarm.weekDayMask = 0b1111111; // every day
    alarm.hour = currTime.hour;
    alarm.minute = currTime.minute - 2; // 2 mins before later
    if (alarm.minute < 0) {
      alarm.minute = 0;
    }
  }
  if (true) {
    Alarm& alarm = _alarms[1];
    CurrTime currTime = getCurrTime();
    alarm.enabled = true;
    alarm.weekDayMask = 0b1111111; // every day
    alarm.hour = currTime.hour + 1;  // 1 hour later
    if (alarm.hour > 23) {
      alarm.hour = 0;
    }
    alarm.minute = currTime.minute - 20; // 20 mins before
    if (alarm.minute < 0) {
      alarm.minute = 0;
    }
  }
  if (true) {
    Alarm& alarm = _alarms[2];
    CurrTime currTime = getCurrTime();
    alarm.enabled = true;
    alarm.weekDayMask = 1 << currTime.weekDay;
    alarm.hour = currTime.hour;
    alarm.minute = currTime.minute + 2; // 2 mins later
    if (alarm.minute > 59) {
      alarm.minute = 59;
    }
  }
  if (true) {
    Alarm& alarm = _alarms[3];
    CurrTime currTime = getCurrTime();
    alarm.enabled = true;
    alarm.weekDayMask = 0; // not repeated
    alarm.hour = currTime.hour + 1;  // 1 hour later
    if (alarm.hour > 23) {
      alarm.hour = 0;
    }
    alarm.minute = 0;
  }
  if (true) {
    Alarm& alarm = _alarms[4];
    CurrTime currTime = getCurrTime();
    alarm.enabled = true;
    alarm.weekDayMask = 1 << currTime.weekDay; // every day
    alarm.hour = currTime.hour;
    alarm.minute = currTime.minute + 4; // next minute
    if (alarm.minute > 59) {
      alarm.minute = 59;
    }
  }
  for (int i = 0; i < NUM_ALARMS; i++) {
    eeprompt_saveAlarm(i);
  }
  //eeprompt_saveAlarms();
}

bool alarmsLoop() {
  if (millis() < BLACK_OUT_MILLIS) {
    return false;
  }
  if (_alarmDueMillis != -1) {
    if (_lastAlarmBeepMillis == -1 || (millis() - _lastAlarmBeepMillis) > 1000) {
      // if (_adhocSoundingMelody) {
      //   return false;  // will not sound alarm if adhoc sounding melody is in progress
      // }
      _lastAlarmBeepMillis = millis();
      //Serial.println("!!! alarm beep !!!");
#if defined(FOR_TWATCH)  
      TTGOClass *ttgo = TTGOClass::getWatch();
      ttgo->shake();
#endif
      void* displayHandle = getDisplayHandle();
      _lastDisplayInverted = !_lastDisplayInverted;
      invertDisplay(displayHandle, _lastDisplayInverted);
#if defined(USE_TASK_FOR_ALARM_SOUND)
#else
      playAlarmBeep();
#endif      
    }
#if defined(AUTO_ACK_ALARM_MINUTES) 
    long diffMillis = millis() - _alarmDueMillis;
    if (diffMillis > (AUTO_ACK_ALARM_MINUTES * 60 * 1000)) {
      Serial.println("!!! alarm auto ack !!!");
      ackAlarmDue();
    }
#endif    
    return true;
  }
  CurrTime currTime = getCurrTime();
  if (false) {
    if (_checkedNextAlarmIdxDay == -1 && _lastAlarmDueTime.year == -1) {
      _forceSetDebugAlarms();
    }
  }
  bool needCheckAlarmDueIdx = _checkedNextAlarmIdxDay == -1 || _checkedNextAlarmIdxDay != currTime.day;
  if (needCheckAlarmDueIdx) {
    _nextAlarmDueIdx = checkNextAlarmDueIdx(currTime);
    _checkedNextAlarmIdxDay = currTime.day;
  }
  if (_nextAlarmDueIdx != -1) {
    Alarm& alarm = _alarms[_nextAlarmDueIdx];
    bool alarmDue = alarm.hour == currTime.hour && alarm.minute == currTime.minute;
    if (alarmDue) {
      _dueAlarmIdx = _nextAlarmDueIdx;
      _alarmDueMillis = millis();
      _lastAlarmDueTime = currTime;
      _lastAlarmBeepMillis = -1;
      if (alarm.weekDayMask == 0) {
        alarm.enabled = false; // one-time alarm
        eeprompt_saveAlarm(_dueAlarmIdx/*_nextAlarmDueIdx*/);
      }
      Alarm& alarm = _alarms[_dueAlarmIdx/*_nextAlarmDueIdx*/];
      Serial.print("$$$ alarm [");
      Serial.print(_dueAlarmIdx/*_nextAlarmDueIdx*/);
      Serial.print("] due -- ");
      Serial.print(alarm.hour);
      Serial.print(":");
      Serial.print(alarm.minute);
      Serial.print(" @ ");
      Serial.print(currTime.year);
      Serial.print("-");
      Serial.print(currTime.month);
      Serial.print("-");
      Serial.print(currTime.day);
      Serial.print(" ");
      Serial.print(currTime.hour);
      Serial.print(":");
      Serial.print(currTime.minute);
      Serial.print(":");
      Serial.print(currTime.second);
      Serial.print(", weekDay: ");
      Serial.println(currTime.weekDay);
#if defined(USE_TASK_FOR_ALARM_SOUND)
      setSoundingAlarmSoundIdx(alarm.alarmSoundIdx);
#endif
    }
  }
  return _alarmDueMillis != -1;
}
