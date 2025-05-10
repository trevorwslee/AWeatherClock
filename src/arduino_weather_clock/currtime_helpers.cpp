
#include <Arduino.h>

#include "config.h"
#include "currtime_helpers.h"


int _dateTimeAdjust = 0;
bool _dateTimeSet = false;

void setDateTime(long dt, int dtTimezone) {
  if (!USE_NTP || !_dateTimeSet) {
    struct timeval tv;
    timezone tz;
#if defined(ESP32)
    if (true) {
      _dateTimeAdjust = 0;
      tv.tv_sec = dt;
      tz.tz_minuteswest = dtTimezone / 60;
    } else {
      _dateTimeAdjust = 0;
      tv.tv_sec = dt + dtTimezone;  // hmm ... check why not add timezone to tz_minuteswest (with or without NTP) ???
      tz.tz_minuteswest = 0;
    }
  #ifdef DEBUG_IT
    Serial.print("=== !!! dtTimezone: ");
    Serial.println(dtTimezone);
  #endif   
#else
    _dateTimeAdjust = dtTimezone;  // appears that need to "add" timezone when getting time
    tv.tv_sec = dt;
    tz.tz_minuteswest = 0;
#endif
    tv.tv_usec = 0;
    tz.tz_dsttime = 0;
    settimeofday(&tv, &tz);  
    _dateTimeSet = true;
  }
}

CurrTime getCurrTime() {
  struct timeval tv;
  struct timezone tz;
  gettimeofday(&tv, &tz);
#ifdef DEBUG_IT
  Serial.print("*** tz_minuteswest: ");
  Serial.println(tz.tz_minuteswest);
  Serial.print("*** tz_dsttime: ");
  Serial.println(tz.tz_dsttime);
  struct tm info;
  getLocalTime(&info, 0);
  Serial.print("*** tm_hour: ");
  Serial.println(info.tm_hour);
#endif
  time_t now = tv.tv_sec + _dateTimeAdjust;
  struct tm timeinfo;
  localtime_r(&now, &timeinfo);
  int year = timeinfo.tm_year + 1900;
  int month = timeinfo.tm_mon + 1;
  int day = timeinfo.tm_mday;
  int hour = timeinfo.tm_hour;  // 0-23
  int minute = timeinfo.tm_min;
  int second = timeinfo.tm_sec;
  int wday = timeinfo.tm_wday;
  CurrTime currTime;
  currTime.hour = hour;
  currTime.minute = minute;
  currTime.second = second;
  currTime.year = year;
  currTime.month = month;
  currTime.day = day;
  currTime.weekDay = wday;
  return currTime;
}


ClockCurrTime getClockCurrTime(bool international) {
  CurrTime currTime = getCurrTime();
  ClockCurrTime clockCurrTime;
  if (international) { 
    clockCurrTime.HH = String(100 + currTime.hour).substring(1);
  } else {
    if (currTime.hour == 0 || currTime.hour == 12) {
      clockCurrTime.HH = String(12);
    } else {
      clockCurrTime.HH = String(100 + (currTime.hour % 12)).substring(1);
    }
  }
  clockCurrTime.mm = String(100 + currTime.minute).substring(1);
  clockCurrTime.ss = String(100 + currTime.second).substring(1);
  if (international) {
    clockCurrTime.apm = "";
  } else {
    if (currTime.hour < 12) {
      clockCurrTime.apm = "a";
    } else {
      clockCurrTime.apm = "p";
    }
  }
  clockCurrTime.yyyy = String(currTime.year);
  clockCurrTime.MM = String(100 + currTime.month).substring(1);
  clockCurrTime.dd = String(100 + currTime.day).substring(1);
  const char* EEE;
  switch (currTime.weekDay) {
    case 0:
      EEE = "Sun";
      break;
    case 1:
      EEE = "Mon";
      break;
    case 2:
      EEE = "Tue";
      break;
    case 3:
      EEE = "Wed";
      break;
    case 4:
      EEE = "Thu";
      break;
    case 5:
      EEE = "Fri";
      break;
    case 6:
      EEE = "Sat";
      break;
    default:
      EEE = "???";
      break;
  }
  clockCurrTime.EEE = EEE;
  return clockCurrTime;
}



