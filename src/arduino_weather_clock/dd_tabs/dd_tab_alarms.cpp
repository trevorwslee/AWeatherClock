#include <Arduino.h>
#include "dd_tab_helpers.h"
#include <arduino_weather_clock/alarms_helpers.h>


#define ALARM_COUNT 5

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
    syncCurrEditingAlarmHourSelection();
    syncCurrEditingAlarmMinuteSelection();
    return true;
  }

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

    GraphicalDDLayerHandle setAlarmBackgroundHandle = dumbdisplay.createGraphicalLayerHandle(120, 100);
    GraphicalDDLayer setAlarmBackground(setAlarmBackgroundHandle);
    setAlarmBackground.border(3, "darkgreen", "round", 1);
    setAlarmBackground.margin(1);
    setAlarmBackground.noBackgroundColor();

    autoPinConfig = DDAutoPinConfig('H', 8)
      .addLayer(editAlarmSelection)
      .beginGroup('S')
        .addLayer(setAlarmBackground)
        .beginPaddedGroup('V', 0, 10, 0, 10)
          .beginGroup('V')
            .addLayer(onRepeatedSelection)
            .addLayer(weekDaySelection)
          .endGroup()
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
  }

  dumbdisplay.pinAutoPinLayers(autoPinConfig, PF_TAB_LEFT, PF_TAB_TOP, PF_TAB_WIDTH, PF_TAB_HEIGHT, "T");
  selectEditingAlarm(0, true);
  currEditingAlarmDirty = false;
}

bool dd_alarms_loop() {
  const DDFeedback* alarmSelectionFeedback = editAlarmSelection->getFeedback();
  const DDFeedback* onOffSelectionFeedback = onRepeatedSelection->getFeedback();
  const DDFeedback* weekDaySelectionFeedback = weekDaySelection->getFeedback();
  const DDFeedback* hourUpButtonFeedback = hourUpButton->getFeedback();
  const DDFeedback* hourDownButtonFeedback = hourDownButton->getFeedback();
  const DDFeedback* minuteUpButtonFeedback = minuteUpButton->getFeedback();
  const DDFeedback* minuteDownButtonFeedback = minuteDownButton->getFeedback();
  const DDFeedback* hourSelectionFeedback = hourSelection->getFeedback();
  const DDFeedback* minuteSelectionFeedback = minuteSelection->getFeedback();
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
  return false;
}

void dd_alarms_done() {

  Serial.println("* DD 'alarm tab' done");
  ensureCurrEditingAlarmSet();
}


