#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>
#include "config.h"

class WeatherIcon {
  public:
    WeatherIcon(const String& iconId, int len) {
      this->iconId = iconId;
      this->len = len;
      this->buff = new uint8_t[len];
    }
    ~WeatherIcon() {
      if (buff != nullptr) {
        delete[] buff;
      }
    }
    const String& getIconId() const {
      return iconId;
    }
    uint8_t* getBuff() {
      return buff;
    }
    int getBufLen() const {
      return len;
    }
  private:
    String iconId;
    uint8_t* buff;
    int len;    
};

class WeatherInfo {
  public:
    WeatherInfo(
        const String& weatherMain, const String& weatherDescription,
        const String& weatherIconId,
        float temp, float feelsLike, float tempMin, float tempMax,
        int pressure, int humidity,
        const String& name,  const String country) {
      this->weatherMain = weatherMain;
      this->weatherDescription = weatherDescription;
      this->weatherIconId = weatherIconId;
      this->temp = temp;
      this->feelsLike = feelsLike;
      this->tempMin = tempMin;
      this->tempMax = tempMax;
      this->pressure = pressure;
      this->humidity = humidity;
      this->humidity = humidity;
      this->name = name;
      this->country = country;
    }
    const String& getWeatherMain() const {
      return weatherMain;
    }
    const String& getWeatherDescription() const {
      return weatherDescription;
    }
    const String& getWeatherIconId() const {
      return weatherIconId;
    }
    int getTempCelsius() const {
      return round(temp - 273.15);
      //return temp;
    }
    int getMinTempCelsius() const {
      return round(tempMin - 273.15);
      //return tempMin;
    }
    int getMaxTempCelsius() const {
      return round(tempMax - 273.15);
      //return tempMax;
    }
    int getFeelsLinkCelsius() const {
      return round(feelsLike - 273.15);
    }
    int getFeelsLikePercent() const {
      float feelsLikeCelsius = feelsLike - 273.15;
      int feelsLikePercent;
      if (feelsLikeCelsius <= 0) {
        feelsLikePercent = 0;
      } else if (feelsLikeCelsius >= MAX_FEELS_LIKE_CELSIUS) {
        feelsLikePercent = 100;
      } else {
        feelsLikePercent = round(100 * feelsLikeCelsius / MAX_FEELS_LIKE_CELSIUS);
      }
      if (feelsLikePercent < 0) {
        feelsLikePercent = 0;
      } else if (feelsLikePercent > 100) {
        feelsLikePercent = 100;
      }
      return feelsLikePercent;
    }
    int getHumidity() const {
      return humidity;
    }
    int getHumidityPercent() const {
      int humidityPercent = humidity;
      if (humidityPercent < 0) {
        humidityPercent = 0;
      } else if (humidityPercent > 100) {
        humidityPercent = 100;
      }
      return humidityPercent;
    }
    const String& getName() const {
      return name;
    }
    const String& getCountry() const {
      return country;
    }
  public:
    bool updateTempMinMax(float tempMin, float tempMax) {
      bool updated = false;
      if (tempMin < this->tempMin) {
        this->tempMin = tempMin;
        updated = true;
      }
      if (tempMax > this->tempMax) {
        this->tempMax = tempMax;
        updated = true;
      }
      return updated;
    }
  private:
    String weatherMain;
    String weatherDescription;
    String weatherIconId;
    float temp;
    float feelsLike;
    float tempMin;
    float tempMax;
    int pressure;
    int humidity;
    String name;
    String country;
};


struct KnowLocation {
  float latitude;
  float longitude;
  int version = -1;  // -1 means not valid
};


extern bool systemStartupSettingsInvalid;

extern int timeZoneInSecs;

extern bool syncWeatherLocationWithGPS;
extern int slideShowIdleDelayMins;
extern int slideDurationSecs;
extern int updateWeatherIntervalMins;

extern WeatherIcon* currWeatherIcon;
extern WeatherInfo* currWeatherInfo;

extern bool internationalTimeFormat;
extern bool showTimeOnSlide;

extern bool ddExclusiveMode;
extern bool forceRefreshWeather;

extern KnowLocation knownLocation;


void onGlobalSettingsChanged(const char* reason);

#endif
