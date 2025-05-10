
#include <Arduino.h>
#include "config.h"
#include "eeprom_helpers.h"

#define PERSIST_KNOWN_LOCATION true


#define READ_BACK_WRITTEN_EEPROM_VALUES


#define EEPROM_SETTINGS_SIZE            (4 + 32)
#define EACH_ALARM_NEEDED_EEPROM_SIZE   8
#define ALARMS_NEEDED_EEPROM_SIZE       (4 + NUM_ALARMS * EACH_ALARM_NEEDED_EEPROM_SIZE)
#define EEPROM_TOTAL_SIZE               (EEPROM_SETTINGS_SIZE + ALARMS_NEEDED_EEPROM_SIZE)
#define EEPROM_SETTINGS_START           4
#define EEPROM_ALARMS_START             EEPROM_SETTINGS_SIZE
#define EEPROM_ALARM_0_START            (4 + EEPROM_ALARMS_START)



#include <EEPROM.h> 
#include "global.h"
#include "alarms_helpers.h"


void _readGlobalSettings() {
  // targets:
  // . syncWeatherLocationWithGPS
  // . slideShowIdleDelayMins
  // . slideDurationSecs
  // . updateWeatherIntervalMins
  // . internationalTimeFormat
  // . showTimeOnSlide
  // . knownLocation
  int address = EEPROM_SETTINGS_START;
  EEPROM.get(address, timeZoneInSecs);  address += sizeof(timeZoneInSecs);
  EEPROM.get(address, syncWeatherLocationWithGPS);  address += sizeof(syncWeatherLocationWithGPS);
  EEPROM.get(address, slideShowIdleDelayMins);  address += sizeof(slideShowIdleDelayMins);
  EEPROM.get(address, slideDurationSecs);  address += sizeof(slideDurationSecs);
  EEPROM.get(address, updateWeatherIntervalMins);  address += sizeof(updateWeatherIntervalMins);
  EEPROM.get(address, internationalTimeFormat);  address += sizeof(internationalTimeFormat);
  EEPROM.get(address, showTimeOnSlide);  address += sizeof(showTimeOnSlide);
  if (PERSIST_KNOWN_LOCATION) {
    EEPROM.get(address, knownLocation);  address += sizeof(knownLocation);
  }
  Serial.println("EEPROM read setting: total size " + String(address) + " vs allocated: " + String(EEPROM_SETTINGS_SIZE));
  Serial.println("- timeZoneInSecs = " + String(timeZoneInSecs));
  Serial.println("- syncWeatherLocationWithGPS = " + String(syncWeatherLocationWithGPS));
  Serial.println("- slideShowIdleDelayMins = " + String(slideShowIdleDelayMins));
  Serial.println("- slideDurationSecs = " + String(slideDurationSecs));
  Serial.println("- updateWeatherIntervalMins = " + String(updateWeatherIntervalMins));
  Serial.println("- internationalTimeFormat = " + String(internationalTimeFormat));
  Serial.println("- showTimeOnSlide = " + String(showTimeOnSlide));
  if (PERSIST_KNOWN_LOCATION) {
    Serial.println("- knownLocation = lat:" + String(knownLocation.latitude) + " / long:" + String(knownLocation.longitude) + " / version:" + String(knownLocation.version));
  }
}
void _readAlarm(int alarmIdx) {

} 
int _readAlarm(int alarmIdx, const char* printPrefix) {
  int address = EEPROM_ALARM_0_START + (alarmIdx * EACH_ALARM_NEEDED_EEPROM_SIZE);
  Alarm alarm;
  EEPROM.get(address, alarm);  address += sizeof(alarm);
  setAlarm(alarmIdx, alarm, printPrefix);
  return address;
}
void _readAlarms() {
    // alarms
    Serial.println("EEPROM read alarms:");
    int address = EEPROM_ALARMS_START;
    int alarmCount;
    EEPROM.get(address, alarmCount);  address += sizeof(alarmCount);
    if (alarmCount == NUM_ALARMS) {
      for (int i = 0; i < NUM_ALARMS; i++) {
        address = _readAlarm(i, "- ");
        // Alarm alarm;
        // EEPROM.get(address, alarm);  address += sizeof(alarm);
        // setAlarm(i, alarm, "- ");
      }
    } else {
      Serial.println("EEPROM alarm count mismatch -- " + String(alarmCount) + " != " + String(NUM_ALARMS) + " ==> not read alarms");
    }
} 
void _writeGlobalSettings(const char* reason) {
  int address = EEPROM_SETTINGS_START;
  EEPROM.put(address, timeZoneInSecs); address += sizeof(timeZoneInSecs);
  EEPROM.put(address, syncWeatherLocationWithGPS); address += sizeof(syncWeatherLocationWithGPS);
  EEPROM.put(address, slideShowIdleDelayMins); address += sizeof(slideShowIdleDelayMins);
  EEPROM.put(address, slideDurationSecs); address += sizeof(slideDurationSecs);
  EEPROM.put(address, updateWeatherIntervalMins); address += sizeof(updateWeatherIntervalMins);
  EEPROM.put(address, internationalTimeFormat); address += sizeof(internationalTimeFormat);
  EEPROM.put(address, showTimeOnSlide); address += sizeof(showTimeOnSlide);
  if (PERSIST_KNOWN_LOCATION) {
    KnowLocation lastKnownLocation = knownLocation;
    EEPROM.put(address, lastKnownLocation); address += sizeof(lastKnownLocation);
  }
  EEPROM.commit();
  Serial.print("EEPROMP [");
  Serial.print(reason);
  Serial.println("] written setting: settings size " + String(address) + " vs settings allocated: " + String(EEPROM_SETTINGS_SIZE));
#if defined(READ_BACK_WRITTEN_EEPROM_VALUES)
    _readGlobalSettings();
#endif
}
int _writeAlarm(int alarmIdx) {
  int address = EEPROM_ALARM_0_START + (alarmIdx * EACH_ALARM_NEEDED_EEPROM_SIZE);
  Alarm alarm = getAlarm(alarmIdx);
  EEPROM.put(address, alarm); address += sizeof(alarm);
  EEPROM.commit();
  Serial.println("EEPROM written alarm [" + String(alarmIdx) + "]");
  return address;
}
void _writeAlarms() {
  int address = EEPROM_ALARMS_START;
  int alarmCount = NUM_ALARMS;
  EEPROM.put(address, alarmCount); address += sizeof(alarmCount);
  for (int i = 0; i < NUM_ALARMS; i++) {
    address = _writeAlarm(i);
    //Alarm alarm = getAlarm(i);
    //EEPROM.put(address, alarm); address += sizeof(alarm);
  }
  Serial.println("EEPROM written alarms: total size " + String(address) + " vs total allocated: " + String(EEPROM_TOTAL_SIZE));
}


void eeprom_initialization() {
  EEPROM.begin(EEPROM_TOTAL_SIZE);
  int32_t header;
  EEPROM.get(0, header);
  if (header == EEPROM_HEADER) {
    Serial.println("EEPROM header `" + String(header) + "` matched");
    _readGlobalSettings();
    _readAlarms();
    systemStartupSettingsInvalid = false;
  } else {
    Serial.println("EEPROM header mismatch -- `" + String(header) + "` != `" + String(EEPROM_HEADER) + "` ==> reset things");
    EEPROM.put(0, EEPROM_HEADER);
    EEPROM.commit();
    _writeGlobalSettings("INIT");
    _writeAlarms();  // will at least need to the number of alarms be written
    systemStartupSettingsInvalid = true;
  }
}


void eeprompt_saveGlobalSettings(const char* reason) {
  _writeGlobalSettings(reason);
}
void eeprompt_saveAlarm(int alarmIdx) {
  _writeAlarm(alarmIdx);
}
