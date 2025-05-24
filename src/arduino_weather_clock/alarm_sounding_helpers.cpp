#include "alarm_sounding_helpers.h"
#include "sound_helpers.h"
#include "melody_helpers.h"



#if defined(USE_TASK_FOR_ALARM_SOUND)


volatile int _alarmSoundIdx = -1;

void _soundAlarmTaskFunc(void* param) {
  while(true) {
    if (_alarmSoundIdx == -1) {
      vTaskDelay(100 / portTICK_PERIOD_MS);
      continue;
    }
    AlarmPreferredType preferAlarmType = AlarmPreferredType::Beeps;
    int alarmParam = -1;
    if (_alarmSoundIdx > 0) {
      int melodyIdx = _alarmSoundIdx - 1;
      if (melodyIdx >= 0 && melodyIdx < NumMelodies) {
        preferAlarmType = AlarmPreferredType::Melody;
        alarmParam = melodyIdx;
      }
  #if defined(ES8311_PA)
      else {
        preferAlarmType = AlarmPreferredType::Music;
        alarmParam = 0;
      }
  #endif  
    }
    soundAlarm([](void* param) {
      int soundingAlarmSoundIdx = (int) param;
      return soundingAlarmSoundIdx != _alarmSoundIdx;
    }, (void*) _alarmSoundIdx, preferAlarmType, alarmParam);
  }
  vTaskDelete(NULL);
}


bool isSoundingAlarmSound() {
  return _alarmSoundIdx != -1;
}
void setSoundingAlarmSoundIdx(int alarmSoundIdx) {
  _alarmSoundIdx = alarmSoundIdx;
}


void alarmSoundingSetup() {
  xTaskCreate(
      _soundAlarmTaskFunc,         
      "SoundAlarmTask",    
      10240,                
      NULL,                 
      configMAX_PRIORITIES - 1,
      NULL                  
  );
} 

#endif



