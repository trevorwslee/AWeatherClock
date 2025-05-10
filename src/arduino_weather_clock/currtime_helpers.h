#ifndef CURRTIME_HELPERS_H
#define CURRTIME_HELPERS_H

#include <Arduino.h>


struct CurrTime {
  int16_t year = -1;
  int16_t month;  // 1-12
  int16_t day;    
  int16_t hour;   // 0-23
  int16_t minute; // 0-59
  int16_t second; // 0-59
  int16_t weekDay; // 0-6: Sun-Sat
};

struct ClockCurrTime {
  String HH = "--";
  String mm = "--";
  String ss = "--";
  String apm = "";  // "" if 24 hour clock (international); "a" or "p" for AM/PM
  String yyyy = "";  // "" if no date
  String MM;
  String dd;
  String EEE;
};


void setDateTime(long dt, int dtTimezone);
CurrTime getCurrTime();
ClockCurrTime getClockCurrTime(bool international);  // international or AM/PM -- https://en.wikipedia.org/wiki/12-hour_clock

#endif