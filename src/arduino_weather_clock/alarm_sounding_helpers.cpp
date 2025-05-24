#include "alarm_sounding_helpers.h"
#include "sound_helpers.h"
#include "melody_helpers.h"



#if defined(USE_TASK_FOR_ALARM_SOUND)


// //volatile bool _alarmSoundingWithTask = false;
// volatile int _alarmSoundIdx = -1;
// volatile int _soundingAlarmSoundIdx = -1;

// void _soundAlarmTaskFunc(void* param) {
//    if (true) {
//     //Alarm& alarm = _alarms[_dueAlarmIdx];
//     AlarmPreferredType preferAlarmType = AlarmPreferredType::Beeps;
//     int alarmParam = -1;
//     if (_alarmSoundIdx > 0) {
//       int melodyIdx = _alarmSoundIdx - 1;
//       if (melodyIdx >= 0 && melodyIdx < NumMelodies) {
//         preferAlarmType = AlarmPreferredType::Melody;
//         alarmParam = melodyIdx;
//       }
//   #if defined(ES8311_PA)
//       else {
//         preferAlarmType = AlarmPreferredType::Music;
//         alarmParam = 0;
//       }
//   #endif  
//     }
//     soundAlarm([](){ return _soundingAlarmSoundIdx != _alarmSoundIdx; }, preferAlarmType, alarmParam);
//   }
//   vTaskDelete(NULL);
// }


void alarmSoundingSetup() {

} 

#endif



