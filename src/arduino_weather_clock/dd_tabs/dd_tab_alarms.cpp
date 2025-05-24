#include <Arduino.h>
#include "dd_tab_helpers.h"
#include "arduino_weather_clock/sys_config.h"
#include "arduino_weather_clock/alarms_helpers.h"
#include <arduino_weather_clock/melody_helpers.h>
#include <arduino_weather_clock/sound_helpers.h>
#include <arduino_weather_clock/global.h>


#define ALARM_COUNT 5

#if defined(USE_TASK_FOR_ALARM_SOUND)
  #define SUPPORT_ALARM_SOUNDS
  // #define ALARM_SOUND_MELODY_START_IDX 1
  // #if defined(ES8311_PA)
  //   #define MUSIC_AS_ALARM_SOUND
  // #endif  
#endif


namespace {


  SelectionDDLayer* editAlarmSelection;
  SelectionDDLayer* onRepeatedSelection;
  SelectionDDLayer* weekDaySelection;
  SevenSegmentRowDDLayer* hourSelection;
  SevenSegmentRowDDLayer* minuteSelection;
  LcdDDLayer* hourUpButton;
  LcdDDLayer* hourDownButton;
  LcdDDLayer* minuteUpButton;
  LcdDDLayer* minuteDownButton;

#if defined(SUPPORT_ALARM_SOUNDS)
  SelectionDDLayer* alarmSoundSelection;
#endif

#if defined(CAN_SET_VOLUME)
  JoystickDDLayer *audioVolumeSlider;
  SelectionDDLayer* audioButton;
#endif

  String autoPinConfig;

  Alarm editingAlarm[ALARM_COUNT];
  int currEditingAlarmIdx;
  bool currEditingAlarmDirty;


  SevenSegmentRowDDLayer* createTimeSelection() {
    SevenSegmentRowDDLayer* timeSelection = dumbdisplay.create7SegmentRowLayer(2);
    timeSelection->border(10, "darkblue", "round", 3);
    timeSelection->backgroundColor("lightgray");
    timeSelection->segmentColor("darkgreen");
    timeSelection->enableFeedback("fs:dbc>numkeys");
    return timeSelection;
  }
  LcdDDLayer* createUpDownButton(const char* label) {
    LcdDDLayer* upDownButton = dumbdisplay.createLcdLayer(8, 1);
    upDownButton->border(0.8, "darkblue", "round", 0.2);
    upDownButton->writeCenteredLine(label);
    upDownButton->enableFeedback(":rpt500");
    return upDownButton;
  }

  void syncEditingAlarmSelection(int alarmIdx, bool forOnOff = true, bool forTime = true) {
    Alarm& alarm = editingAlarm[alarmIdx];
    if (forOnOff) {
      String text1 = "#" + String(alarmIdx + 1) + ":";
      if (alarm.enabled) {
        text1.concat("🕰️");
      }
      editAlarmSelection->text(text1, 0, 0, alarmIdx);
    }
    if (forTime) {
      String text2;
      if (alarm.enabled) {
        text2 = String(100 + alarm.hour).substring(1) + ":" + String(100 +  alarm.minute).substring(1);
      } else {
        text2 = "--:--";
      }
      editAlarmSelection->text(text2, 1, 0, alarmIdx);
    }
  }


  void syncCurrEditingAlarmOnOffSelection() {
    Alarm& currEditingAlarm = editingAlarm[currEditingAlarmIdx];
    onRepeatedSelection->selected(currEditingAlarm.enabled, 0);
    onRepeatedSelection->selected(currEditingAlarm.weekDayMask != 0, 1);
    for (int i = 0; i < 7; i++) {
      int mask = 1 << i;
      bool on = (currEditingAlarm.weekDayMask & mask) != 0;
      weekDaySelection->selected(on, i);
    }
  }

#if defined(SUPPORT_ALARM_SOUNDS)
  void syncCurrEditingSoundSelection() {
    Alarm& currEditingAlarm = editingAlarm[currEditingAlarmIdx];
    int alarmSoundIdx = currEditingAlarm.alarmSoundIdx;
    if (alarmSoundIdx < 0 || alarmSoundIdx >= getAlarmSoundSelectCount()) {
      alarmSoundIdx = 0;
    }
    alarmSoundSelection->select(alarmSoundIdx);
  }
#endif

  void syncCurrEditingAlarmHourSelection() {
    Alarm& currEditingAlarm = editingAlarm[currEditingAlarmIdx];
    hourSelection->showFormatted(String(100 + currEditingAlarm.hour).substring(1));
  }

  void syncCurrEditingAlarmMinuteSelection() {
    Alarm& currEditingAlarm = editingAlarm[currEditingAlarmIdx];
    minuteSelection->showFormatted(String(100 + currEditingAlarm.minute).substring(1));
  }

  void setCurrEditingAlarmTime(int deltaHour, int deltaMinute) {
    Alarm& currEditingAlarm = editingAlarm[currEditingAlarmIdx];
    currEditingAlarm.hour += deltaHour;
    if (currEditingAlarm.hour > 23) {
      currEditingAlarm.hour = 0;
    } else if (currEditingAlarm.hour < 0) {
      currEditingAlarm.hour = 23;
    }
    currEditingAlarm.minute += deltaMinute;
    if (currEditingAlarm.minute > 59) {
      currEditingAlarm.minute = 0;
    } else if (currEditingAlarm.minute < 0) {
      currEditingAlarm.minute = 59;
    }
    if (deltaHour != 0) {
      syncCurrEditingAlarmHourSelection();
    }
    if (deltaMinute != 0) {
      syncCurrEditingAlarmMinuteSelection();
    }
    syncEditingAlarmSelection(currEditingAlarmIdx, false, true);    
    currEditingAlarmDirty = true;
  }
  void setCurrEditingAlarmHourMinute(int hour, int minute) {
    if (hour == -1 && minute >= 100) {
      hour = minute / 100;
      minute = minute % 100;
    } else if (minute == -1 && hour >= 100) {
      minute = hour % 100;
      hour = hour / 100;
    }
    Alarm& currEditingAlarm = editingAlarm[currEditingAlarmIdx];
    if (hour >= 0) {
      currEditingAlarm.hour = hour % 24;
      syncCurrEditingAlarmHourSelection();
    }
    if (minute >= 0) {
      currEditingAlarm.minute = minute % 60;
      syncCurrEditingAlarmMinuteSelection();
    }
    syncEditingAlarmSelection(currEditingAlarmIdx, false, true);    
    currEditingAlarmDirty = true;
  }

  void setCurrEditingAlarmOnOff(int onRepeatedSelectionIdx, int weekDaySelectionIdx) {
    Alarm& currEditingAlarm = editingAlarm[currEditingAlarmIdx];
    if (onRepeatedSelectionIdx == 0) {
      currEditingAlarm.enabled = !currEditingAlarm.enabled;
    } else if (onRepeatedSelectionIdx == 1) {
      if (currEditingAlarm.weekDayMask == 0) {
          currEditingAlarm.weekDayMask = 0b1111111;
      } else {
        currEditingAlarm.weekDayMask = 0;
      }
    }
    currEditingAlarm.weekDayMask ^= 1 << weekDaySelectionIdx;
    syncCurrEditingAlarmOnOffSelection();      
    syncEditingAlarmSelection(currEditingAlarmIdx, true, true);    
    currEditingAlarmDirty = true;
  }
 
#if defined(SUPPORT_ALARM_SOUNDS)
  void setCurrEditingAlarmSound(int alarmSoundIdx) {
    if (alarmSoundIdx < 0 || alarmSoundIdx >= getAlarmSoundSelectCount()) {
      return;
    }
    // Serial.print("***** setCurrEditingAlarmSound ... currEditingAlarmIdx: ");
    // Serial.println(currEditingAlarmIdx);
    Alarm& currEditingAlarm = editingAlarm[currEditingAlarmIdx];
    currEditingAlarm.alarmSoundIdx = alarmSoundIdx;
    syncCurrEditingSoundSelection();
  }
#endif

  void ensureCurrEditingAlarmSet() {
    if (currEditingAlarmDirty) {
      setAlarm(currEditingAlarmIdx, editingAlarm[currEditingAlarmIdx], "* DD set alarm -- ", true);
      currEditingAlarmDirty = false;
    }
  }

  bool selectEditingAlarm(int alarmIdx, bool forInit = false) {
    if (!forInit && alarmIdx == currEditingAlarmIdx) {
      return false;
    }
    Alarm& alarm = editingAlarm[alarmIdx];
    editAlarmSelection->selected(true, 0, alarmIdx, true);
    if (!forInit) {
      ensureCurrEditingAlarmSet();
    }
    currEditingAlarmIdx = alarmIdx;
    syncCurrEditingAlarmOnOffSelection();      
  #if defined(SUPPORT_ALARM_SOUNDS)
    syncCurrEditingSoundSelection();
  #endif
    syncCurrEditingAlarmHourSelection();
    syncCurrEditingAlarmMinuteSelection();
    return true;
  }

#if defined(CAN_SET_VOLUME)
  void syncAudioVolume() {
    int volume = audioVolume;
    if (volume < 0) {
      volume = 0;
    } else if (volume > 100) {
      volume = 100;
    }
    audioVolumeSlider->moveToPos(volume, 0);
  }
  void toggleAdhocSoundingMelody() {
    if (isSoundingAlarm()) {
      stopAlarmSound();
      audioButton->selected(false);
    } else {
      adhocStartAlarmSound(3);
      audioButton->selected(true);
    }    
  }
  void ensureAdhocSoundingMelody(bool ensureStarted = true) {
    bool isSounding = isSoundingAlarm();
    bool toggle = ensureStarted ? !isSounding : isSounding; 
    if (toggle) {
      toggleAdhocSoundingMelody();
    }
  }
#endif
}



void dd_alarms_setup(bool recreateLayers) {
  if (recreateLayers) {
    editAlarmSelection = dumbdisplay.createSelectionLayer(5, 2, 1, ALARM_COUNT);
    editAlarmSelection->enableFeedback();
    for (int i = 0; i < ALARM_COUNT; i++) {
      editingAlarm[i] = getAlarm(i);
      syncEditingAlarmSelection(i);
    }

    onRepeatedSelection = dumbdisplay.createSelectionLayer(8, 1, 2, 1);
    onRepeatedSelection->textCentered("⏰", 0, 0);
    onRepeatedSelection->textCentered("🔄", 0, 1);
    onRepeatedSelection->enableFeedback();

    weekDaySelection = dumbdisplay.createSelectionLayer(2, 1, 7, 1);
    weekDaySelection->text("Su", 0, 0);
    weekDaySelection->text("Mo", 0, 1);
    weekDaySelection->text("Tu", 0, 2);
    weekDaySelection->text("We", 0, 3);
    weekDaySelection->text("Th", 0, 4);
    weekDaySelection->text("Fr", 0, 5);
    weekDaySelection->text("Sa", 0, 6);
    weekDaySelection->enableFeedback();

    hourSelection = createTimeSelection();
    hourUpButton = createUpDownButton("🔼");
    hourDownButton = createUpDownButton("🔽");

    minuteSelection = createTimeSelection();
    minuteUpButton = createUpDownButton("🔼");
    minuteDownButton = createUpDownButton("🔽");

    LedGridDDLayerHandle colonLabelHandle = dumbdisplay.createLedGridLayerHandle(1, 7);
    LedGridDDLayer colonLabel(colonLabelHandle);
    colonLabel.onColor("darkgreen");
    colonLabel.turnOn(0, 2);
    colonLabel.turnOn(0, 4);

    //GraphicalDDLayerHandle setAlarmBackgroundHandle = dumbdisplay.createGraphicalLayerHandle(120, 100);
#if defined(SUPPORT_ALARM_SOUNDS)
    int bgWidth = 100;
    int begHeight = 120;
#else
    int bgWidth = 120;
    int begHeight = 100;
#endif    
    GraphicalDDLayerHandle setAlarmBackgroundHandle = dumbdisplay.createGraphicalLayerHandle(bgWidth, begHeight);
    GraphicalDDLayer setAlarmBackground(setAlarmBackgroundHandle);
    setAlarmBackground.border(3, "darkgreen", "round", 1);
    setAlarmBackground.margin(1);
    setAlarmBackground.noBackgroundColor();

#if defined(SUPPORT_ALARM_SOUNDS)
    alarmSoundSelection = dumbdisplay.createSelectionLayer(13, 1, 2, 2);
    int alarmSoundSelectCount = getAlarmSoundSelectCount();
    for (int i = 0; i < alarmSoundSelectCount; i++) {
      String text = getAlarmSoundSelectText(i);
      alarmSoundSelection->textCentered(text, 0, i);
    }
    alarmSoundSelection->enableFeedback();
#endif

#if defined(CAN_SET_VOLUME)
    audioVolumeSlider = dumbdisplay.createJoystickLayer(100, "lr", 0.6);
    audioVolumeSlider->valueRange(0, 100);
    audioVolumeSlider->snappy(true);
    audioVolumeSlider->showValue(true, "white");

    // LcdDDLayerHandle volumeLabelHandle = dumbdisplay.createLcdLayerHandle(2, 1);
    // LcdDDLayer volumeLabel(volumeLabelHandle);
    // volumeLabel.border(1, "blue", "hair");
    audioButton = dumbdisplay.createSelectionLayer(4, 1);
    //audioButton->selected(true);
    audioButton->highlightBorder(true, "darkblue");
    if (false) {
      audioButton->textCentered("🔊");
    } else {
      // animate text on volumeLabel
      for (int i = 0; i < 3; i++) {
        String text;
        switch (i) {
          case 0: text = "🔈"; break;
          case 1: text = "🔉"; break;
          case 2: text = "🔊"; break;
        }
        audioButton->textCentered(text);    
        audioButton->exportAsBackgroundImage(false);
      }
      audioButton->clear();
      audioButton->animateBackgroundImage(2);
    }
#endif

    autoPinConfig = DDAutoPinConfig('H', 8)
      .addLayer(editAlarmSelection)
      .beginGroup('S')
        .addLayer(setAlarmBackground)
        .beginPaddedGroup('V', 0, 10, 0, 10)
          .beginGroup('V')
            .addLayer(onRepeatedSelection)
            .addLayer(weekDaySelection)
          .endGroup()
#if defined(SUPPORT_ALARM_SOUNDS)
          .addLayer(alarmSoundSelection)
#endif
          .beginPaddedGroup('H', 0, 10, 0, 2)
            .beginGroup('V')
              .addLayer(hourUpButton)
              .addLayer(hourSelection)
              .addLayer(hourDownButton)
            .endGroup()
            .addLayer(colonLabel)
            .beginGroup('V')
              .addLayer(minuteUpButton)
              .addLayer(minuteSelection)
              .addLayer(minuteDownButton)
            .endGroup()
          .endPaddedGroup()
        .endPaddedGroup()
      .endGroup()
    .build();

#if defined(CAN_SET_VOLUME)
    autoPinConfig = DDAutoPinConfig('V')
      .addAutoPinConfig(autoPinConfig)
      .beginGroup('H')
        .addLayer(audioButton)
        .addLayer(audioVolumeSlider)
      .endGroup()
      .build();
#endif
  }

  dumbdisplay.pinAutoPinLayers(autoPinConfig, PF_TAB_LEFT, PF_TAB_TOP, PF_TAB_WIDTH, PF_TAB_HEIGHT, "T");
#if defined(CAN_SET_VOLUME)
  syncAudioVolume();
#endif
  selectEditingAlarm(0, true);
  currEditingAlarmDirty = false;
}

bool dd_alarms_loop() {
#if defined(CAN_SET_VOLUME)
  if (audioButton->getFeedback() != nullptr) {
    toggleAdhocSoundingMelody();
  }
  const DDFeedback* audioVolumeFeedback = audioVolumeSlider->getFeedback();
  if (audioVolumeFeedback != nullptr) {
    int audioVolumeSetTo = audioVolumeFeedback->x;
    if (audioVolumeSetTo >= 0 && audioVolumeSetTo <= 100) {
      //Serial.println("***** set audio volume: " + String(audioVolumeSetTo));
      ensureAdhocSoundingMelody();
      audioVolume = audioVolumeSetTo;
      //playAlarmBeep();
      onGlobalSettingsChanged("audioVolume");
    }
  }
#endif
  const DDFeedback* alarmSelectionFeedback = editAlarmSelection->getFeedback();
  const DDFeedback* onOffSelectionFeedback = onRepeatedSelection->getFeedback();
  const DDFeedback* weekDaySelectionFeedback = weekDaySelection->getFeedback();
  const DDFeedback* hourUpButtonFeedback = hourUpButton->getFeedback();
  const DDFeedback* hourDownButtonFeedback = hourDownButton->getFeedback();
  const DDFeedback* minuteUpButtonFeedback = minuteUpButton->getFeedback();
  const DDFeedback* minuteDownButtonFeedback = minuteDownButton->getFeedback();
  const DDFeedback* hourSelectionFeedback = hourSelection->getFeedback();
  const DDFeedback* minuteSelectionFeedback = minuteSelection->getFeedback();
#if defined(SUPPORT_ALARM_SOUNDS)
  const DDFeedback* alarmSoundSelectionFeedback = alarmSoundSelection->getFeedback();  
#endif
  if (alarmSelectionFeedback != nullptr) {
    int alarmIdx = alarmSelectionFeedback->y;
    editAlarmSelection->flashArea(0, alarmIdx);
    selectEditingAlarm(alarmIdx);
  }
  if (onOffSelectionFeedback != nullptr) {
    int onRepeatedSelectionIdx = onOffSelectionFeedback->x;
    onRepeatedSelection->flashArea(onRepeatedSelectionIdx, 0);
    setCurrEditingAlarmOnOff(onRepeatedSelectionIdx, -1);
  }
  if (weekDaySelectionFeedback != nullptr) {
    int weekDaySelectionIdx = weekDaySelectionFeedback->x;
    weekDaySelection->flashArea(weekDaySelectionIdx, 0);
    setCurrEditingAlarmOnOff(-1, weekDaySelectionIdx);
  }
  if (hourUpButtonFeedback != nullptr) {
    hourUpButton->flash();
    setCurrEditingAlarmTime(-1, 0);
  }
  if (hourDownButtonFeedback != nullptr) {
    hourDownButton->flash();
    setCurrEditingAlarmTime(1, 0);
  }
  if (minuteUpButtonFeedback != nullptr) {
    minuteUpButton->flash();
    setCurrEditingAlarmTime(0, -1);
  }
  if (minuteDownButtonFeedback != nullptr) {
    minuteDownButton->flash();
    setCurrEditingAlarmTime(0, 1);
  }
  if (hourSelectionFeedback != nullptr) {
    int hour = hourSelectionFeedback->text.toInt();
    setCurrEditingAlarmHourMinute(hour, -1);
  }
  if (minuteSelectionFeedback != nullptr) {
    int minute = minuteSelectionFeedback->text.toInt();
    setCurrEditingAlarmHourMinute(-1, minute);
  }
#if defined(SUPPORT_ALARM_SOUNDS)
  if (alarmSoundSelectionFeedback != nullptr) {
    int x = alarmSoundSelectionFeedback->x;
    int y = alarmSoundSelectionFeedback->y;
    int alarmSoundIdx = x + 2 * y;
    if (alarmSoundIdx >= 0 && alarmSoundIdx < getAlarmSoundSelectCount()) {
      // Serial.print("***** setCurrEditingAlarmSound ... alarmSoundIdx: ");
      // Serial.println(alarmSoundIdx);
      alarmSoundSelection->flashArea(x, y);
      setCurrEditingAlarmSound(alarmSoundIdx);
    }
  }
#endif
  return false;
}

void dd_alarms_done() {
  Serial.println("* DD 'alarm tab' done");
#if defined(CAN_SET_VOLUME)
  ensureAdhocSoundingMelody(false);
#endif
  ensureCurrEditingAlarmSet();
}


