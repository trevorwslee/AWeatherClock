#include <Arduino.h>

#include "config.h"
#include "trigger_helpers.h"

#define CLICK_THRESHOLD_MILLIS 200
#define DOUBLE_CLICK_PRESS_AGAIN_MILLIS 500

#if defined(CST_TP_BUS_NUM)
  #include <Wire.h>
  #include "cst816t.h"
  TwoWire tpWire(CST_TP_BUS_NUM);
  cst816t touchpad(tpWire, CST_TP_RST, CST_TP_INT);
#elif defined(FT_TP_SCL)
  #include <Wire.h>
  #include "FT6336U.h"
  FT6336U touchpad(/*FT_TP_SDA, FT_TP_SCL, */FT_TP_RST, FT_TP_INT);
  //volatile bool checkTouchpadStatus = false;
#elif defined(GT911_TP_SCL)  
  #include "TAMC_GT911.h"
  TAMC_GT911 touchpad = TAMC_GT911(GT911_TP_SDA, GT911_TP_SCL, GT911_TP_INT, GT911_TP_RST, GT911_TP_WIDTH, GT911_TP_HEIGHT);
#elif defined(FOR_TWATCH)
  #include <LilyGoWatch.h>
  //#define KEEP_PRESSED_MILLIS 1000
#endif

// . BUTTON_PIN / TOUCH_PIN / FOR_TWATCH: double click
// . CST_TP_BUS_NUM: double click or long press

//volatile bool _inSpecialState = false;
volatile TriggeredState _triggeredState;
volatile unsigned long _lastTriggerMillis = 0;
volatile unsigned long _canLastTriggerMillis = 0;

// #if defined(FOR_ESP_SPARKBOT)
//   #define TOUCH_PIN            3 
//   #define TOUCH_THRESHOLD      90
//   #define MIN_TOUCH_VALUE      33000
// #endif
#if defined(BUTTON_PIN) || defined(TOUCH_PIN) || defined(FOR_TWATCH) || defined(FT_TP_SCL) || defined(GT911_TP_SCL)
void  _triggered() {
  #if defined(BUTTON_PIN)
    int buttonState = digitalRead(BUTTON_PIN);
    // if (true) {
    //   Serial.printf("??? clicked ... val=%d\n", buttonState);
    // }
    if (buttonState != 0) {
      return;
    }
 
  #elif defined(TOUCH_PIN)
    touch_value_t val = touchRead(TOUCH_PIN);
    if (false) {  // TODO: for sparkboot check touch
      Serial.printf("????????????????????????????????????????? touched ... val=%d\n", val);
    }
    if (val < MIN_TOUCH_VALUE) {
      // if (false) {
      //   Serial.printf("??? not touched ... val=%d\n", val);
      // }
      return;
    }
  #elif defined(FT_TP_SCL)
    if (false) {
      Serial.print("### FT6336U TD Status: ");
      Serial.println(touchpad.read_td_status());
      // Serial.print("FT6336U Touch Event/ID 1: (");
      // Serial.print(touchpad.read_touch1_event()); Serial.print(" / "); Serial.print(touchpad.read_touch1_id()); Serial.println(")");
      // Serial.print("FT6336U Touch Position 1: (");
      // Serial.print(touchpad.read_touch1_x()); Serial.print(" , "); Serial.print(touchpad.read_touch1_y()); Serial.println(")");
      // Serial.print("FT6336U Touch Weight/MISC 1: (");
      // Serial.print(touchpad.read_touch1_weight()); Serial.print(" / "); Serial.print(touchpad.read_touch1_misc()); Serial.println(")");
      // Serial.print("FT6336U Touch Event/ID 2: (");
      // Serial.print(touchpad.read_touch2_event()); Serial.print(" / "); Serial.print(touchpad.read_touch2_id()); Serial.println(")");
      // Serial.print("FT6336U Touch Position 2: (");
      // Serial.print(touchpad.read_touch2_x()); Serial.print(" , "); Serial.print(touchpad.read_touch2_y()); Serial.println(")");
      // Serial.print("FT6336U Touch Weight/MISC 2: (");
      // Serial.print(touchpad.read_touch2_weight()); Serial.print(" / "); Serial.print(touchpad.read_touch2_misc()); Serial.println(")");
    }
    uint8_t status = touchpad.read_td_status();
    if (true) {
      Serial.print("### FT6336U TD Status: ");
      Serial.println(status);
    }
    if (status != 0) {
      return;
    }
  #endif
  #if defined(BUTTON_PIN) || defined(GT911_TP_SCL) || defined(FOR_TWATCH)
    // debounce
    unsigned long diffMillis = millis() - _lastTriggerMillis;
    if (_lastTriggerMillis != 0 && diffMillis < CLICK_THRESHOLD_MILLIS) {
      return;  // not too fast
    }
    _lastTriggerMillis = millis();
  #endif  
  unsigned long canDiffMillis = millis() - _canLastTriggerMillis;
  if (true) {
    Serial.printf("=== triggered ... %lu ... canDiffMillis=%lu\n", millis(), canDiffMillis);
  }
  if (_triggeredState != TS_NONE) {
    if (canDiffMillis > DOUBLE_CLICK_PRESS_AGAIN_MILLIS) {
      _triggeredState = TS_NONE;
      _canLastTriggerMillis = 0;
    }
  } else {
    // like detect double click
    //unsigned long diffMillis = millis() - lastTriggerMillis;
    if (_canLastTriggerMillis == 0 || canDiffMillis > DOUBLE_CLICK_PRESS_AGAIN_MILLIS) {
      _canLastTriggerMillis = millis();
    } else {
      _triggeredState = TS_IP;
      //lastTriggerMillis = 0;
    }
  }
}
#endif

// bool checkInSpecialState() {
//   return _inSpecialState;
// }
// void setInSpecialState(bool inSpecialState) {
//   _inSpecialState = inSpecialState;
//   if (false) {
//     canLastTriggerMillis = 0;
//   }
// }
TriggeredState getTriggeredState() {
  return _triggeredState;
}
void setTriggeredState(TriggeredState triggeredState) {
  _triggeredState = triggeredState;

}



// #if defined(FT_TP_SCL)
// void _tpIntHandle(void) {
//   checkTouchpadStatus = true;
// }
// #endif

void triggerSetup() {
#ifdef BUTTON_PIN
  pinMode(BUTTON_PIN, INPUT_PULLUP);  // assume INPUT_PULLUP
  //attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), _triggered, CHANGE);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), _triggered, FALLING);
#elif defined(TOUCH_PIN)
  //pinMode(TOUCH_PIN, INPUT_PULLDOWN);  // assume INPUT_PULLUP
  touchAttachInterrupt(TOUCH_PIN, _triggered, TOUCH_THRESHOLD);
  //touchAttachInterrupt(1, _touched, TOUCH_THRESHOLD);
  //touchAttachInterrupt(TOUCH_PAD_NUM2, _touched, TOUCH_THRESHOLD);
  //touchAttachInterrupt(3, _touched, 60);
#elif defined(CST_TP_BUS_NUM)
  tpWire.begin(CST_TP_SDA, CST_TP_SCL);
  touchpad.begin(mode_motion);
#elif defined(FT_TP_SCL)
  //pinMode(FT_TP_INT, INPUT_PULLUP);
  Wire.setSDA(FT_TP_SDA);  // FT6336U will use Wire
  Wire.setSCL(FT_TP_SCL);
  touchpad.begin();
  attachInterrupt(digitalPinToInterrupt(FT_TP_INT), _triggered, CHANGE);
#elif defined(GT911_TP_SCL)  
  touchpad.begin(GT911_ADDR2);  // sometimes, need to use the backup GT911_ADDR2
  touchpad.setRotation(ROTATION_NORMAL);
#endif
}

//unsigned long lastTriggerMillis = 0;
// #ifdef KEEP_PRESSED_MILLIS
//   unsigned long switchingToShowIPMillis = 0;
// #endif
void triggerLoop() {
#if defined(CST_TP_BUS_NUM)
  if (touchpad.available()) {
    bool singleClick = touchpad.gesture_id == GESTURE_SINGLE_CLICK;
    bool doubleClick = touchpad.gesture_id == GESTURE_DOUBLE_CLICK;
    bool longPress = touchpad.gesture_id == GESTURE_LONG_PRESS;
    //Serial.printf("* id=%d ... 1=%d / 2=%d / long=%d\n", touchpad.gesture_id, singleClick, doubleClick, longPress);
    if (_triggeredState != TS_NONE) {
      _triggeredState = TS_NONE;
    } else if (doubleClick || longPress) {
      _triggeredState = TS_IP;
    } 
  }
// #elif defined(FT_TP_SCL)
//   if (checkTouchpadStatus) {
//     if (true) {
//       Serial.print("FT6336U TD Status: ");
//       Serial.println(touchpad.read_td_status());
//       // Serial.print("FT6336U Touch Event/ID 1: (");
//       // Serial.print(touchpad.read_touch1_event()); Serial.print(" / "); Serial.print(touchpad.read_touch1_id()); Serial.println(")");
//       // Serial.print("FT6336U Touch Position 1: (");
//       // Serial.print(touchpad.read_touch1_x()); Serial.print(" , "); Serial.print(touchpad.read_touch1_y()); Serial.println(")");
//       // Serial.print("FT6336U Touch Weight/MISC 1: (");
//       // Serial.print(touchpad.read_touch1_weight()); Serial.print(" / "); Serial.print(touchpad.read_touch1_misc()); Serial.println(")");
//       // Serial.print("FT6336U Touch Event/ID 2: (");
//       // Serial.print(touchpad.read_touch2_event()); Serial.print(" / "); Serial.print(touchpad.read_touch2_id()); Serial.println(")");
//       // Serial.print("FT6336U Touch Position 2: (");
//       // Serial.print(touchpad.read_touch2_x()); Serial.print(" , "); Serial.print(touchpad.read_touch2_y()); Serial.println(")");
//       // Serial.print("FT6336U Touch Weight/MISC 2: (");
//       // Serial.print(touchpad.read_touch2_weight()); Serial.print(" / "); Serial.print(touchpad.read_touch2_misc()); Serial.println(")");
//     }
//     uint8_t status = touchpad.read_td_status();
//     if (status != 0) {
//       _triggered();
//     }
//     // if (status == 0) {
//     //   _showIP = false;
//     // } else {
//     //   _showIP = true;
//     // }
//     checkTouchpadStatus = false;
//   }
#elif defined(GT911_TP_SCL)  
  touchpad.read();
  if (touchpad.isTouched) {
    if (false) {
      for (int i=0; i<touchpad.touches; i++){
        Serial.print("Touch ");Serial.print(i+1);Serial.print(": ");;
        Serial.print("  x: ");Serial.print(touchpad.points[i].x);
        Serial.print("  y: ");Serial.print(touchpad.points[i].y);
        Serial.print("  size: ");Serial.println(touchpad.points[i].size);
        Serial.println(' ');
      }
    }
    _triggered();
  }
#elif defined(FOR_TWATCH)
  TTGOClass *ttgo = TTGOClass::getWatch();
  int16_t x, y;
  bool gotPoint = ttgo->getTouch(x, y);
  if (gotPoint) {
    _triggered();
  }
  // #ifdef KEEP_PRESSED_MILLIS
  // if (gotPoint) {
  //   unsigned long now = millis();
  //   if (lastTriggerMillis == 0) {
  //     if (_inSpecialState) {
  //       _inSpecialState = false;
  //     } else { 
  //       lastTriggerMillis = now;
  //     }
  //   } else {
  //     unsigned long diffMillis = now - lastTriggerMillis;
  //     if (diffMillis >= KEEP_PRESSED_MILLIS) {
  //       _inSpecialState = true;
  //     }
  //   }
  // } else {
  //   lastTriggerMillis = 0;
  // }  
  // #else
  // if (gotPoint) {
  //   unsigned long now = millis();
  //   unsigned long diffMillis = now - lastTriggerMillis;
  //   lastTriggerMillis = now;
  //   if (diffMillis >= 200) {
  //     _showIP = !_showIP;
  //   }
  // }
  // #endif
#endif
}