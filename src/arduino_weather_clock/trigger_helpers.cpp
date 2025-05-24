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
#elif defined(CST816S_SDA)
  #include <CST816S.h>
  CST816S touch(CST816S_SDA, CST816S_SCL, CST816S_RST, CST816S_INT);	// sda, scl, rst, irq
#elif defined(FT_TP_SCL)
  #include <Wire.h>
  #include "FT6336U.h"
  #if defined(ESP32)
    FT6336U touchpad(FT_TP_SDA, FT_TP_SCL, FT_TP_RST, FT_TP_INT);
  #else
    FT6336U touchpad(/*FT_TP_SDA, FT_TP_SCL, */FT_TP_RST, FT_TP_INT);
  #endif  
#elif defined(GT911_TP_SCL)  
  #include "TAMC_GT911.h"
  TAMC_GT911 touchpad = TAMC_GT911(GT911_TP_SDA, GT911_TP_SCL, GT911_TP_INT, GT911_TP_RST, GT911_TP_WIDTH, GT911_TP_HEIGHT);
#elif defined(XPT2046_MOSI)
  #include <XPT2046_Bitbang.h>
  XPT2046_Bitbang touchscreen(XPT2046_MOSI, XPT2046_MISO, XPT2046_CLK, XPT2046_CS);
  //#include <XPT2046_Touchscreen.h>
  //XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
  // #include <TFT_Touch.h>
  // TFT_Touch touch = TFT_Touch(XPT2046_CS, XPT2046_CLK,XPT2046_DIN, XPT2046_DOUT);
#elif defined(XPT2046_IRQ)
  // #include <XPT2046_Bitbang.h>
  // XPT2046_Bitbang touchscreen(XPT2046_MOSI, XPT2046_MISO, XPT2046_CLK, XPT2046_CS);
  #include <XPT2046_Touchscreen.h>
  SPIClass ts_spi(HSPI);
  XPT2046_Touchscreen ts(XPT2046_CS/*, XPT2046_IRQ*/);
  //#include <TFT_Touch.h>
  // TFT_Touch touch = TFT_Touch(XPT2046_CS, XPT2046_CLK,XPT2046_DIN, XPT2046_DOUT);
#elif defined(XPT2046_CS)
  // #include <XPT2046_Bitbang.h>
  // XPT2046_Bitbang touchscreen(XPT2046_MOSI, XPT2046_MISO, XPT2046_CLK, XPT2046_CS);
  //#include <XPT2046_Touchscreen.h>
  #include <TFT_Touch.h>
  TFT_Touch touch = TFT_Touch(XPT2046_CS, XPT2046_CLK,XPT2046_DIN, XPT2046_DOUT);
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
#if !defined(TW3)
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
    if (false) {  // TODO: for sparkbot check touch
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
  if (false) {
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



void triggerSetup() {
#ifdef BUTTON_PIN
  #if defined(BUTTON_PULLDOWN)
    pinMode(BUTTON_PIN, INPUT_PULLDOWN);
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), _triggered, FALLING);
  #else
    pinMode(BUTTON_PIN, INPUT_PULLUP);  // assume INPUT_PULLUP
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), _triggered, FALLING);
  #endif
#elif defined(TOUCH_PIN)
  //pinMode(TOUCH_PIN, INPUT_PULLDOWN);  // assume INPUT_PULLUP
  touchAttachInterrupt(TOUCH_PIN, _triggered, TOUCH_THRESHOLD);
  //touchAttachInterrupt(1, _touched, TOUCH_THRESHOLD);
  //touchAttachInterrupt(TOUCH_PAD_NUM2, _touched, TOUCH_THRESHOLD);
  //touchAttachInterrupt(3, _touched, 60);
#elif defined(CST_TP_BUS_NUM)
  tpWire.begin(CST_TP_SDA, CST_TP_SCL);
  touchpad.begin(mode_motion);
  if (true) {
    uint8_t chip_id = touchpad.chip_id;
    Serial.print("***** Touch Chip ID: ");
    Serial.println(chip_id);
    String version = touchpad.version();
    Serial.print("***** Touch Firmware Version: ");
    Serial.println(version);
  }
#elif defined(CST816S_SDA)
  touch.begin();
  //touch.disable_auto_sleep();
  if (true) {
    // Print version information
    Serial.print("Touch Firmware Version: ");
    Serial.print(touch.data.version);
    Serial.print("\t");
    Serial.print(touch.data.versionInfo[0]);
    Serial.print("-");
    Serial.print(touch.data.versionInfo[1]);
    Serial.print("-");
    Serial.println(touch.data.versionInfo[2]);
  // // Disable auto sleep to keep the device active (useful during debugging or testing)
  // touch.disable_auto_sleep();
  // Serial.println("Auto sleep disabled");    
  }
#elif defined(FT_TP_SCL)
  //pinMode(FT_TP_INT, INPUT_PULLUP);
  #if !defined(ESP32)
    Wire.setSDA(FT_TP_SDA);  // FT6336U will use Wire
    Wire.setSCL(FT_TP_SCL);
  #endif  
  touchpad.begin();
  attachInterrupt(digitalPinToInterrupt(FT_TP_INT), _triggered, CHANGE);
#elif defined(GT911_TP_SCL)  
  touchpad.begin(GT911_ADDR1);  // sometimes, need to use the backup GT911_ADDR2
  touchpad.setRotation(ROTATION_NORMAL);
#elif defined(XPT2046_MOSI)
  touchscreen.begin();
  //ts.begin(SPI);
  //touch.setCal(481, 3395, 755, 3487, 320, 240, 1);
  // touch.setCal(0, 4095, 0, 4095, 320, 240, 1);
  // touch.setRotation(1);
  // ts.begin();
  // ts.setRotation(1);
#elif defined(XPT2046_IRQ)
  //touch.setCal(481, 3395, 755, 3487, 320, 240, 1);
  // touch.setCal(0, 4095, 0, 4095, 320, 240, 1);
  // touch.setRotation(1);

  // TODO: not working
  #define TP_CLK   25
  #define TP_MOSI  32
  #define TP_MISO  39
  ts_spi.setFrequency(2500000);
  //ts_spi.begin(TP_CLK, 39/*TP_MISO*/, -1/*TP_MOSI*/, XPT2046_CS);
  ts.begin(ts_spi);

  //ts.begin();
  ts.setRotation(1);
#elif defined(XPT2046_CS)
  //touch.setCal(481, 3395, 755, 3487, 320, 240, 1);
  //touch.setCal(0, 4095, 0, 4095, 320, 240, 1);
  touch.setRotation(1);
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
#elif defined(CST816S_SDA)
  if (touch.available()) {
    if (true) {
      Serial.print(touch.gesture());
      Serial.print("\t");
      Serial.print(touch.data.points);
      Serial.print("\t");
      Serial.print(touch.data.event);
      Serial.print("\t");
      Serial.print(touch.data.x);
      Serial.print("\t");
      Serial.println(touch.data.y);      
    }
    _triggered();
  }
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
#elif defined(XPT2046_MOSI)
  TouchPoint touch = touchscreen.getTouch();
  // Display touches that have a pressure value (Z)
  if (touch.zRaw != 0 && (touch.x != 0 || touch.y != 0)) {
    Serial.print("Touch at X: ");
    Serial.print(touch.x);
    Serial.print(", Y: ");
    Serial.println(touch.y);
  }
#elif defined(XPT2046_IRQ)
  // if (touch.Pressed()) // Note this function updates coordinates stored within library variables
  // {
  //   // Read the current X and Y axis as co-ordinates at the last touch time
  //   // The values were captured when Pressed() was called!
  //   int X_Coord = touch.X();
  //   int Y_Coord = touch.Y();

  //   Serial.print(X_Coord); Serial.print(","); Serial.println(Y_Coord);
  // }
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    Serial.print("Pressure = ");
    Serial.print(p.z);
    Serial.print(", x = ");
    Serial.print(p.x);
    Serial.print(", y = ");
    Serial.print(p.y);
    Serial.println();
  }  
#elif defined(XPT2046_CS)
  if (touch.Pressed()) // Note this function updates coordinates stored within library variables
  {
    // Read the current X and Y axis as co-ordinates at the last touch time
    // The values were captured when Pressed() was called!
    int X_Coord = touch.X();
    int Y_Coord = touch.Y();

    Serial.print(X_Coord); Serial.print(","); Serial.println(Y_Coord);
  }
  // if (ts.touched()) {
  //   TS_Point p = ts.getPoint();
  //   Serial.print("Pressure = ");
  //   Serial.print(p.z);
  //   Serial.print(", x = ");
  //   Serial.print(p.x);
  //   Serial.print(", y = ");
  //   Serial.print(p.y);
  //   Serial.println();
 //}  
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