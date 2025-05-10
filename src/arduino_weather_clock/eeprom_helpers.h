#ifndef EEPROM_HELPERS_H
#define EEPROM_HELPERS_H

void eeprom_initialization();

void eeprompt_saveGlobalSettings(const char* reason);
void eeprompt_saveAlarm(int alarmIdx);



#endif