#include <Arduino.h>
//#include <EEPROM.h> 

#include "config.h"
#include "global.h"
#include "png_helpers.h"
#include "weather_helpers.h"
#include "currtime_helpers.h"
#include "screen_helpers.h"
#include "eeprom_helpers.h"
#include "trigger_helpers.h"
#include "dd_helpers.h"
#include "slides_helpers.h"
#include "name_helpers.h"
#include "imgs/temp.h"
#include "imgs/humi.h"
#include "imgs/alarm.h"


#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#else
  #include <WiFi.h>
#endif

#ifdef ESP32
  #include <WiFiManager.h>  // https://github.com/tzapu/WiFiManager
#endif
#include "alarms_helpers.h"
#include "sound_helpers.h"


#ifdef TEST_WIFI_MANAGER
  #undef WIFI_SSID
#endif

enum Showing { CLOCK, CLOCK_WITH_IP, SLIDES };
enum ExclusiveState { NONE, DD_EXCLUSIVE, ALARM_EXCLUSIVE };


IPAddress localIP;
Showing showing = Showing::CLOCK;
ExclusiveState exclusiveState = ExclusiveState::NONE;


long lastUpdateWeatherMillis = 0;
long lastUpdateTimeMillis = 0;
unsigned long ddJustIdleMillisForShowSlides = 0;



void configNTP(long gmtOffset_sec) {
  configTime(gmtOffset_sec, 0, "pool.ntp.org", "time.nist.gov");
}

void drawCurrWeatherIcon() {
  if (currWeatherIcon == nullptr) {
    return;
  }
  drawPNG(TFT_X_OFF + 145, 5, currWeatherIcon->getBuff(), currWeatherIcon->getBufLen());
}

void drawCurrTime(void* displayHandle) {
  if (currWeatherInfo  == nullptr) {
    return;
  }
  ClockCurrTime currTime = getClockCurrTime(internationalTimeFormat);
  int timeXOff = internationalTimeFormat ? 0 : -7;
  drawText(displayHandle, currTime.HH + ":" + currTime.mm + ":" + currTime.ss, TFT_X_OFF + timeXOff + 22, 100, COLOR_CYAN, 4);
  if (!internationalTimeFormat) {
    String am = "AM";
    String pm = "PM";
    if (currTime.apm == "a") {
      pm = "  ";
    } else if (currTime.apm == "p") {
      am = "  ";
    }
    drawText(displayHandle, am, TFT_X_OFF + 212, 102, COLOR_WHITE, 1);
    drawText(displayHandle, pm, TFT_X_OFF + 212, 122, COLOR_WHITE, 1);
  }
  if (currTime.yyyy != "") {
    drawText(displayHandle, currTime.yyyy != "" ? currTime.yyyy + "-" + currTime.MM + "-" + currTime.dd : "", TFT_X_OFF + 18, 148, COLOR_WHITE, 2);
    drawText(displayHandle, currTime.EEE, TFT_X_OFF + 160, 143, COLOR_YELLOW, 3);
  }
}


void drawPercentageBar(void* displayHandle, int x, int y, int percentage, uint16_t barColor) {
  int h = 8;
  float scale = 0.6;
  int scaledPercentage = round(percentage * scale);
  int scaledBarWidth = round(100 * scale);
  drawRect(displayHandle, x, y, scaledBarWidth + 4, h + 4, COLOR_WHITE);
  drawRect(displayHandle, x + 1, y + 1, scaledBarWidth + 2, h + 2, COLOR_WHITE);
  fillRect(displayHandle, x + 2, y + 2, scaledPercentage, h, barColor);
}

void drawCurrWeather(void* displayHandle, bool showIP, bool alarmDue/*AlarmDueMode alarmDueMode*/) {
  if (currWeatherInfo  == nullptr) {
    return;
  }

  const char* _countryName = countryCodeToCountryName(currWeatherInfo->getCountry());
  String countryName;
  if (_countryName != nullptr) {
    countryName = _countryName;
  } else {
    countryName = currWeatherInfo->getCountry();
  }
  if (countryName.length() > 12) {
    int idx = countryName.indexOf(" ");
    if (idx != -1) {
      countryName = countryName.substring(0, idx);
    }
    if (countryName.endsWith(",")) {
      countryName = countryName.substring(0, countryName.length() - 1);
    }
  }
  String name = currWeatherInfo->getName();
  if (name.length() > 14) {
    name = name.substring(0, 14);
  }
  drawText(displayHandle, countryName, TFT_X_OFF + 10, 20, COLOR_YELLOW, 2);
  drawText(displayHandle, name, TFT_X_OFF + 10, 46, COLOR_WHITE, 1);
  drawCurrWeatherIcon();   
  drawBitmap(displayHandle, TFT_X_OFF + 121, 47, temp565, TEMP565_WIDTH, TEMP565_HEIGHT);
  drawText(displayHandle, String(currWeatherInfo->getMaxTempCelsius()), TFT_X_OFF + 147, 45, COLOR_RED, 1);
  drawText(displayHandle, String(currWeatherInfo->getMinTempCelsius()), TFT_X_OFF + 147, 70, COLOR_CYAN, 1);
  drawText(displayHandle, String(currWeatherInfo->getTempCelsius()), TFT_X_OFF + 94, 51, COLOR_WHITE, 3);

  drawCurrTime(displayHandle);

  if (showIP/*showing == Showing::CLOCK_WITH_IP*/) {
    String ip = localIP.toString();
    drawText(displayHandle, ip, TFT_X_OFF + 2, 218, COLOR_RED, 2);
  } else {
    drawBitmap(displayHandle, TFT_X_OFF + 10, 180, temp565, TEMP565_WIDTH, TEMP565_HEIGHT);
    drawBitmap(displayHandle, TFT_X_OFF + 10, 210, humi565, HUMI565_WIDTH, HUMI565_HEIGHT);

    drawPercentageBar(displayHandle, TFT_X_OFF + 48, 189, currWeatherInfo->getFeelsLikePercent(), COLOR_ORANGE);
    drawText(displayHandle, String(currWeatherInfo->getFeelsLinkCelsius()), TFT_X_OFF + 123, 187, COLOR_ORANGE, 2);

    drawPercentageBar(displayHandle, TFT_X_OFF + 48, 219, currWeatherInfo->getHumidityPercent(), COLOR_GREEN);
    drawText(displayHandle, String(currWeatherInfo->getHumidity()), TFT_X_OFF + 123, 217, COLOR_GREEN, 2);
    
    const String& weatherMain = currWeatherInfo->getWeatherMain();
    int sepIdx = weatherMain.indexOf(" ");
    if (sepIdx == -1) {
      drawText(displayHandle, weatherMain, TFT_X_OFF + 156, 202, COLOR_WHITE, 2);
    } else {
      drawText(displayHandle, weatherMain.substring(0, sepIdx), TFT_X_OFF + 156, 187, COLOR_WHITE, 2);
      drawText(displayHandle, weatherMain.substring(sepIdx + 1), TFT_X_OFF + 156, 217, COLOR_WHITE, 2);
    }

    if (alarmDue) {
      drawBitmap(displayHandle, TFT_X_OFF + 10, 33, alarm565, ALARM565_WIDTH, ALARM565_HEIGHT);
      drawBitmap(displayHandle, TFT_X_OFF + 170, 33, alarm565, ALARM565_WIDTH, ALARM565_HEIGHT);
    }
    if (alarmDue) {
      drawBitmap(displayHandle, TFT_X_OFF + 10, 175, alarm565, ALARM565_WIDTH, ALARM565_HEIGHT);
      drawBitmap(displayHandle, TFT_X_OFF + 170, 175, alarm565, ALARM565_WIDTH, ALARM565_HEIGHT);
    }
  }

  Serial.println("!!! updated weather");
}


bool fetchAndUpdateWeather() {
  FetchedWeather weather = fetchWeather(currWeatherIcon, FORECAST_WEATHER);
  WeatherInfo* weatherInfo = weather.weatherInfo;
  WeatherIcon* weatherIcon = weather.weatherIcon;
  if (weatherInfo != nullptr) {
    long dt = weather.dt;
    int timezone = weather.timezone;
    setDateTime(dt, timezone);
    if (USE_NTP) {
      configNTP(timezone);
      Serial.printf("!!! updated NTP timezone: %d\n", timezone);
      if (timeZoneInSecs != timezone) {
        timeZoneInSecs = timezone;
        onGlobalSettingsChanged("TIMEZONE");
      }      
    } else {
      Serial.printf("!!! updated local time of timezone: %d\n", timezone);
    }
    if (currWeatherInfo != nullptr) {
      delete currWeatherInfo;
    }
    currWeatherInfo = weatherInfo;
    if (weatherIcon != nullptr) {
      if (currWeatherIcon != nullptr) {
        delete currWeatherIcon;
      }
      currWeatherIcon = weatherIcon;
    }
    return true;
  }
  return false;
}

bool initialized = false;

#ifdef DELAY_INITIALIZE_FOR_SECONDS
  int initializeCounter = DELAY_INITIALIZE_FOR_SECONDS; 
#else
  int initializeCounter = 0;
#endif

void showArduinoWeatherClockInfo() {
  Serial.println();
  Serial.println("****************************************");
  Serial.println("*** ARDUINO WEATHER CLOCK (v" + String(ARDUINO_WEATHER_CLOCK_VERSION) + ")");
#if !defined(WIFI_SSID)
  Serial.println("*** - AUTOCONNECT_AP_NAME           : " + String(AUTOCONNECT_AP_NAME));
#endif    
  Serial.println("*** - timezone (seconds) : " + String(timeZoneInSecs));
  if (knownLocation.version != -1) {
  Serial.println("*** - GPS location       : lat=" + String(knownLocation.latitude) + " / long=" + String(knownLocation.longitude));
  }
  if (syncWeatherLocationWithGPS && knownLocation.version != -1) {
  } else {
  Serial.println("*** - weather location   : " + String(DEF_OPEN_WEATHER_API_LOCATION));
  }
#if defined(ESP32)
  Serial.println("*** - IP                 : " + localIP.toString());
  Serial.println("*** - ESP32 line of MCU  :");
  Serial.println("***   . Sketch - " + String(ESP.getSketchSize() / 1024.0) + "KB" + " / Free: " + String(ESP.getFreeSketchSpace() / 1024.0) + "KB");
  Serial.println("***   . Heap   - " + String(ESP.getHeapSize() / 1024.0) + "KB" + " / Free: " + String(ESP.getFreeHeap() / 1024.0) + "KB");
  Serial.println("***   . PSRAM  - " + String(ESP.getPsramSize() / 1024.0) + "KB" + " / Free: " + String(ESP.getFreePsram() / 1024.0) + "KB");
#endif    
  Serial.println("****************************************");
  Serial.println();
}

void setup() {
  Serial.begin(115200);
}

void _initialize() {
  eeprom_initialization();
  triggerSetup();
  soundSetup();
  screenSetup();

  void* displayHandle = getDisplayHandle();

  clearScreen(displayHandle);
  //tft.fillScreen(BACKGROUND_COLOR);

  ddSetup();
  alarmsSetup();
  slidesSetup();

#ifndef WIFI_SSID
  #ifdef ESP32
  drawText(displayHandle, String("WiFi AP -- ") + AUTOCONNECT_AP_NAME, 10, 60, CONTROL_FONT_COLOR, CONTROL_FONT_SIZE);
  WiFiManager wm;
  #ifdef TEST_WIFI_MANAGER
  if (true) {
    // reset settings - for testing
    wm.resetSettings();
    wm.erase();
  }
  #endif
  if (!wm.autoConnect(AUTOCONNECT_AP_NAME)) {
    Serial.println("failed to auto connect");
    clearScreen(displayHandle);
    drawText(displayHandle, "AP connect failed!", 10, 60, CONTROL_FONT_COLOR, CONTROL_FONT_SIZE);
    delay(5000);
    // reset and try again, or maybe put it to deep sleep
    ESP.restart();
    return;
  }
  #else
    #error for non-ESP32, please define WIFI_SSID and WIFI_PASSWORD in _secret.h
  #endif
#else
  drawText(displayHandle, "Connecting WiFi...", 10, 60, CONTROL_FONT_COLOR, CONTROL_FONT_SIZE);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
#endif 
  while (true) {
    uint8_t status = WiFi.status();
    if (status == WL_CONNECTED) {
      localIP = WiFi.localIP();
      break;
    }
    Serial.println("... connecting to WiFi ...");
    delay(1000);
  }
  Serial.println("*** connected to WiFi ***");
 
  if (USE_NTP) {
    configNTP(timeZoneInSecs/*TIMEZONE * 60 * 60*/);
  }

  showArduinoWeatherClockInfo();

  // the first thing is get weather ... so, show it here
  clearScreen(displayHandle);
  drawText(displayHandle, "Getting Weather...", 10, 60, CONTROL_FONT_COLOR, CONTROL_FONT_SIZE);
}

void loopProcessing(void* displayHandle, bool ddIdle, bool alarmDue, bool resetForStateChange) {
  if (resetForStateChange) {
    if (alarmDue) {
      setTriggeredState(TS_ALARM); 
    } else {
      setTriggeredState(TS_NONE); 
    }
    ddJustIdleMillisForShowSlides = 0;
    showing = Showing::CLOCK;
  }

  long currMillis = millis();
  bool readyToShowSlides = checkReadyToShowSlides() && !alarmDue;
  TriggeredState triggeredState = getTriggeredState();

  bool showIP = false;
  if (alarmDue) {
    if (triggeredState != TS_ALARM) {
      ackAlarmDue();
    }
  } else if (showing == Showing::SLIDES) {
    if (triggeredState != TS_SLIDES) {
      ddJustIdleMillisForShowSlides = 0;
    }
  } else {
    showIP = triggeredState == TS_IP;
  } 
  
  bool ddConnected = false;
  if (lastUpdateWeatherMillis > 0) {
    if (false) {
      if (resetForStateChange) {
        setTriggeredState(TS_NONE);
      }
    }
    ddConnected = !ddIdle;
    if (!ddIdle) {
      // DD connected ==> show normally
      showIP = false;
    }
    if (ddIdle && readyToShowSlides/* && !showIP*/) {
      // considered idle for slide show
      if (ddJustIdleMillisForShowSlides == 0) {
        ddJustIdleMillisForShowSlides = currMillis;
      }
    } else {
      // not considered idle for slide show
      ddJustIdleMillisForShowSlides = 0;
    }
  }

  bool showSlides = false;
  if (ddJustIdleMillisForShowSlides > 0 && slideShowIdleDelayMins > 0) {
    showSlides =  (currMillis - ddJustIdleMillisForShowSlides) >= (1000 * 60 * slideShowIdleDelayMins);
  }
  bool updateWeather;
  if (updateKnownLocationFromDD()) {
    updateWeather = true;
  } else {
    updateWeather = lastUpdateWeatherMillis == 0 || (currMillis - lastUpdateWeatherMillis) >= (1000 * 60 * updateWeatherIntervalMins);
  }
  bool weatherUpdated = false;
  if (forceRefreshWeather) {
    updateWeather = true;
    forceRefreshWeather = false;
  }
  if (updateWeather) {
    lastUpdateWeatherMillis = currMillis;
    fetchAndUpdateWeather();
    weatherUpdated = true;
  }
  bool updateTime = lastUpdateTimeMillis == 0 || (currMillis - lastUpdateTimeMillis) >= 1000;  
  bool timeUpdated = false;
  if (updateTime) {
    lastUpdateTimeMillis = currMillis;
    timeUpdated = true;
  }  
  if (showSlides) {
    bool showingSlides = showing == Showing::SLIDES;
    if (!showingSlides) {
      Serial.println("!!! start showing slides");
      setTriggeredState(TS_SLIDES);  // showing slide is a special state
      clearScreen(displayHandle);
    }
    slidesLoop(!showingSlides);
    showing = Showing::SLIDES;
  } else {
    bool redrawScreen = false;
    if (showing == Showing::SLIDES) {
      Serial.println("!!! end showing slides");
      redrawScreen = true;
    } 
    if (showIP) {
      redrawScreen = showing != CLOCK_WITH_IP;
      showing = Showing::CLOCK_WITH_IP;
    } else {
      redrawScreen = showing != Showing::CLOCK;
      showing = Showing::CLOCK;
    }
    if (ddConnected) {
      redrawScreen |= checkDDRenderedRedrawScreen();
    }
    if (redrawScreen || resetForStateChange) {
      clearScreen(displayHandle);
      weatherUpdated = true;
      timeUpdated = true;
    }
    if (weatherUpdated) {
      clearScreen(displayHandle);
      drawCurrWeather(displayHandle, showIP, alarmDue);
    }
    if (timeUpdated) {
      //Serial.printf("* %d\n", (currMillis - ddJustIdleMillisForShowSlides) / 1000);
      drawCurrTime(displayHandle);
    }  
  }
}

void loop() {
  if (!initialized) {
    if (initializeCounter > 0) {
      Serial.println("!!! delay initialize ... " + String(initializeCounter) + " ...");
      delay(1000);
      initializeCounter -= 1;
      return;
    }
    _initialize();
    initialized = true;
    return;
  }

  void* displayHandle = getDisplayHandle();  
  triggerLoop();
  bool alarmDue = alarmsLoop();
  bool ddIdle = ddLoop();
  if (ddExclusiveMode && alarmDue) {
    ddExclusiveMode = false;  // if alarm is due, disable dd exclusive mode    
  }
  if (ddExclusiveMode) {
    if (exclusiveState != ExclusiveState:: DD_EXCLUSIVE) {
      exclusiveState = ExclusiveState::DD_EXCLUSIVE;
      Serial.println("### enter DD exclusive mode");
      clearScreen(displayHandle);
    }
  } else {
    bool resetForStateChange = false;
    if (alarmDue) {
      if (exclusiveState != ExclusiveState::ALARM_EXCLUSIVE) {
        exclusiveState = ExclusiveState::ALARM_EXCLUSIVE;
        resetForStateChange = true;
        Serial.println("### enter ALARM exclusive mode");
      }
    } else {
      if (exclusiveState != ExclusiveState::NONE) {
        exclusiveState = ExclusiveState::NONE;
        resetForStateChange = true;
        Serial.println("### exit exclusive mode");
      }
    }
    loopProcessing(displayHandle, ddIdle, alarmDue, resetForStateChange);
  }
}


