
#include <Arduino.h>
#include "config.h"
#include "eeprom_helpers.h"

//#define PERSIST_KNOWN_LOCATION true


#define READ_BACK_WRITTEN_EEPROM_VALUES

const uint8_t EEPROM_VERSION = 1;

#define O_V0_EEPROM_SETTINGS_SIZE            (4 + 32)
#define O_V0_EACH_ALARM_NEEDED_EEPROM_SIZE   8
#define O_V0_ALARMS_NEEDED_EEPROM_SIZE       (4 + NUM_ALARMS * O_V0_EACH_ALARM_NEEDED_EEPROM_SIZE)
#define O_V0_EEPROM_TOTAL_SIZE               (V0_EEPROM_SETTINGS_SIZE + O_V0_ALARMS_NEEDED_EEPROM_SIZE)
#define O_V0_EEPROM_SETTINGS_START           4
#define O_V0_EEPROM_ALARMS_START             O_V0_EEPROM_SETTINGS_SIZE
#define O_V0_EEPROM_ALARM_0_START            (4 + O_V0_EEPROM_ALARMS_START)

#define V0_EEPROM_HEADER_SIZE              4
#define V0_EEPROM_SETTINGS_SIZE            32
#define V0_EACH_ALARM_NEEDED_EEPROM_SIZE   8
#define V0_EEPROM_ALARM_HEADER_SIZE        4
#define V0_ALARMS_NEEDED_EEPROM_SIZE       (V0_EEPROM_ALARM_HEADER_SIZE + NUM_ALARMS * V0_EACH_ALARM_NEEDED_EEPROM_SIZE)
#define V0_EEPROM_TOTAL_SIZE               (V0_EEPROM_HEADER_SIZE + V0_EEPROM_SETTINGS_SIZE + V0_ALARMS_NEEDED_EEPROM_SIZE)
#define V0_EEPROM_SETTINGS_START           V0_EEPROM_HEADER_SIZE
#define V0_EEPROM_ALARMS_START             V0_EEPROM_SETTINGS_START + V0_EEPROM_SETTINGS_SIZE
#define V0_EEPROM_ALARM_0_START            (V0_EEPROM_ALARM_HEADER_SIZE + V0_EEPROM_ALARMS_START)

#define VC_EEPROM_HEADER_SIZE              4
#define VC_EEPROM_SETTINGS_SIZE            32
#define VC_EACH_ALARM_NEEDED_EEPROM_SIZE   10
#define VC_EEPROM_ALARM_HEADER_SIZE        4
#define VC_ALARMS_NEEDED_EEPROM_SIZE       (VC_EEPROM_ALARM_HEADER_SIZE + NUM_ALARMS * VC_EACH_ALARM_NEEDED_EEPROM_SIZE)
#define VC_EEPROM_TOTAL_SIZE               (VC_EEPROM_HEADER_SIZE + VC_EEPROM_SETTINGS_SIZE + VC_ALARMS_NEEDED_EEPROM_SIZE)
#define VC_EEPROM_SETTINGS_START           VC_EEPROM_HEADER_SIZE
#define VC_EEPROM_ALARMS_START             VC_EEPROM_SETTINGS_START + VC_EEPROM_SETTINGS_SIZE
#define VC_EEPROM_ALARM_0_START            (VC_EEPROM_ALARM_HEADER_SIZE + VC_EEPROM_ALARMS_START)

// #define VC_EEPROM_HEADER_SIZE              4
// #define VC_EEPROM_SETTINGS_SIZE            35
// #define VC_EACH_ALARM_NEEDED_EEPROM_SIZE   9
// #define VC_EEPROM_ALARM_HEADER_SIZE        4
// #define VC_ALARMS_NEEDED_EEPROM_SIZE       (VC_EEPROM_ALARM_HEADER_SIZE + NUM_ALARMS * VC_EACH_ALARM_NEEDED_EEPROM_SIZE)
// #define VC_EEPROM_TOTAL_SIZE               (VC_EEPROM_HEADER_SIZE + VC_EEPROM_SETTINGS_SIZE + VC_ALARMS_NEEDED_EEPROM_SIZE)
// #define VC_EEPROM_SETTINGS_START           VC_EEPROM_HEADER_SIZE
// #define VC_EEPROM_ALARMS_START             VC_EEPROM_SETTINGS_START + VC_EEPROM_SETTINGS_SIZE
// #define VC_EEPROM_ALARM_0_START            (VC_EEPROM_ALARM_HEADER_SIZE + VC_EEPROM_ALARMS_START)

#define EEPROM_TOTAL_SIZE               VC_EEPROM_TOTAL_SIZE
#define EEPROM_SETTINGS_SIZE            VC_EEPROM_SETTINGS_SIZE
#define EEPROM_SETTINGS_START           VC_EEPROM_SETTINGS_START
#define EEPROM_ALARMS_START             VC_EEPROM_ALARMS_START
#define EEPROM_ALARM_0_START            VC_EEPROM_ALARM_0_START
#define EACH_ALARM_NEEDED_EEPROM_SIZE   VC_EACH_ALARM_NEEDED_EEPROM_SIZE


int _get_read_settings_start(uint8_t version) {
  if (version == 0) {
    return V0_EEPROM_SETTINGS_START;
  } else {
    return VC_EEPROM_SETTINGS_START;
  }
}
int _get_read_alarms_start(uint8_t version) {
  if (version == 0) {
    return V0_EEPROM_ALARMS_START;
  } else {
    return VC_EEPROM_ALARMS_START;
  }
}
int _get_read_alarm_0_strat(uint8_t version) {
  if (version == 0) {
    return V0_EEPROM_ALARM_0_START;
  } else {
    return VC_EEPROM_ALARM_0_START;
  }
}
int _get_read_each_alarm_needed_size(uint8_t version) {
  if (version == 0) {
    return V0_EACH_ALARM_NEEDED_EEPROM_SIZE;
  } else {
    return VC_EACH_ALARM_NEEDED_EEPROM_SIZE;
  }
}



#include <EEPROM.h> 
#include "global.h"
#include "alarms_helpers.h"



void _readGlobalSettings(uint8_t version) {
  // targets:
  // . syncWeatherLocationWithGPS
  // . slideShowIdleDelayMins
  // . slideDurationSecs
  // . updateWeatherIntervalMins
  // . internationalTimeFormat
  // . showTimeOnSlide
  // . knownLocation
  int address = _get_read_settings_start(version);//EEPROM_SETTINGS_START;
  EEPROM.get(address, timeZoneInSecs);  address += sizeof(timeZoneInSecs);
  EEPROM.get(address, syncWeatherLocationWithGPS);  address += sizeof(syncWeatherLocationWithGPS);
  EEPROM.get(address, slideShowIdleDelayMins);  address += sizeof(slideShowIdleDelayMins);
  EEPROM.get(address, slideDurationSecs);  address += sizeof(slideDurationSecs);
  EEPROM.get(address, updateWeatherIntervalMins);  address += sizeof(updateWeatherIntervalMins);
  EEPROM.get(address, internationalTimeFormat);  address += sizeof(internationalTimeFormat);
  EEPROM.get(address, showTimeOnSlide);  address += sizeof(showTimeOnSlide);
  EEPROM.get(address, knownLocation);  address += sizeof(knownLocation);
  int size = address - _get_read_settings_start(version);//EEPROM_SETTINGS_START;
  Serial.println("EEPROM read setting: total settings size " + String(size) + " vs allocated: " + String(EEPROM_SETTINGS_SIZE));
  Serial.println("- timeZoneInSecs = " + String(timeZoneInSecs));
  Serial.println("- syncWeatherLocationWithGPS = " + String(syncWeatherLocationWithGPS));
  Serial.println("- slideShowIdleDelayMins = " + String(slideShowIdleDelayMins));
  Serial.println("- slideDurationSecs = " + String(slideDurationSecs));
  Serial.println("- updateWeatherIntervalMins = " + String(updateWeatherIntervalMins));
  Serial.println("- internationalTimeFormat = " + String(internationalTimeFormat));
  Serial.println("- showTimeOnSlide = " + String(showTimeOnSlide));
  Serial.println("- knownLocation = lat:" + String(knownLocation.latitude) + " / long:" + String(knownLocation.longitude) + " / version:" + String(knownLocation.version));
}
// void _readAlarm(int alarmIdx) {
// } 
int _readAlarm(uint8_t version, int alarmIdx, const char* printPrefix) {
  int address = _get_read_alarm_0_strat(version)/*EEPROM_ALARM_0_START*/ + (alarmIdx * _get_read_each_alarm_needed_size(version)/*EACH_ALARM_NEEDED_EEPROM_SIZE*/);
  if (version <= 0) {
    AlarmV0 alarm_v0;
    EEPROM.get(address, alarm_v0);  address += sizeof(alarm_v0);
    Alarm alarm;
    alarm.enabled = alarm_v0.enabled;
    alarm.hour = alarm_v0.hour;
    alarm.minute = alarm_v0.minute;
    alarm.weekDayMask = alarm_v0.weekDayMask;
    setAlarm(alarmIdx, alarm, printPrefix);
    return address;
  } else {
    Alarm alarm;
    EEPROM.get(address, alarm);  address += sizeof(alarm);
    setAlarm(alarmIdx, alarm, printPrefix);
    return address;
  }
}
void _readAlarms(uint8_t version) {
    // alarms
    Serial.println("EEPROM read alarms:");
    int address = _get_read_alarms_start(version);//EEPROM_ALARMS_START;
    int alarmCount;
    EEPROM.get(address, alarmCount);  address += sizeof(alarmCount);
    if (alarmCount == NUM_ALARMS) {
      for (int i = 0; i < NUM_ALARMS; i++) {
        address = _readAlarm(version, i, "- ");
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
  KnowLocation lastKnownLocation = knownLocation;
  EEPROM.put(address, lastKnownLocation); address += sizeof(lastKnownLocation);
  EEPROM.commit();
  Serial.print("EEPROM [");
  Serial.print(reason);
  int size = address - EEPROM_SETTINGS_START; 
  Serial.println("] written setting: settings size " + String(size) + " vs settings allocated: " + String(EEPROM_SETTINGS_SIZE));
#if defined(READ_BACK_WRITTEN_EEPROM_VALUES)
    _readGlobalSettings(EEPROM_VERSION);
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
void _writeAll(const char* reason) {
   uint32_t header = EEPROM_HEADER | (((uint32_t) EEPROM_VERSION) << 28);
   EEPROM.put(0, header);
  _writeGlobalSettings(reason);
  _writeAlarms();
  EEPROM.commit();
}


void eeprom_initialization() {
  EEPROM.begin(EEPROM_TOTAL_SIZE);
  uint32_t header;
  EEPROM.get(0, header);
  uint8_t version = 0;
  if (true) {
    version = (header >> 28) & 0xf;
    header = 0x0fffffff & header;
  }
  if (header == EEPROM_HEADER) {
    Serial.println("EEPROM header `" + String(header) + "` matched with version `" + String(version) + "`");
    _readGlobalSettings(version);
    _readAlarms(version);
    if (version != EEPROM_VERSION) {
      Serial.println("EEPROM upgrade from version  `" + String(version) + "` to version `" + String(EEPROM_VERSION) + "`");
      _writeAll("UPGRADE");
      // _writeGlobalSettings("UPGRADE");
      // _writeAlarms();
    }
    systemStartupSettingsInvalid = false;
  } else {
    Serial.println("EEPROM header mismatch -- `" + String(header) + "` != `" + String(EEPROM_HEADER) + "` ==> reset things");
    _writeAll("INIT");
    // EEPROM.put(0, EEPROM_HEADER);
    // _writeGlobalSettings("INIT");
    // _writeAlarms();
    //EEPROM.commit();
    systemStartupSettingsInvalid = true;
  }
}


void eeprompt_saveGlobalSettings(const char* reason) {
  _writeGlobalSettings(reason);
}
void eeprompt_saveAlarm(int alarmIdx) {
  _writeAlarm(alarmIdx);
}
