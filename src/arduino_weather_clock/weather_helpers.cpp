#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "config.h"
#include "global.h"
#include "weather_helpers.h"

#define OPEN_WEATHER_API_WEATHER_END_POINT  "http://api.openweathermap.org/data/2.5/weather"
#define OPEN_WEATHER_API_FORECAST_END_POINT "http://api.openweathermap.org/data/2.5/forecast"



FetchedWeather fetchWeather(const WeatherIcon* currWeatherIcon, bool forecast) {
  HTTPClient http;
  int httpCode = HTTP_CODE_UNAUTHORIZED;
#ifdef OPEN_WEATHER_MAP_APP_ID
  String url = String(OPEN_WEATHER_API_WEATHER_END_POINT) + "?appid=" + OPEN_WEATHER_MAP_APP_ID;
  if (syncWeatherLocationWithGPS &&  knownLocation.version != -1) {
    url = url + "&lat=" + knownLocation.latitude + "&lon=" + knownLocation.longitude;
  } else {
    String q = String(DEF_OPEN_WEATHER_API_LOCATION);
    q.replace(" ", "%20");
      url = url + "&q=" + q;
  }
  Serial.printf("... fetching weather info with url %s ...\n", url.c_str());
  http.begin(url); 
  httpCode = http.GET();
#else  
  Serial.println("... OPEN_WEATHER_API_LOCATION not set for fetching weather info ...");
#endif  
  //String gotWeatherIconId = "";
  WeatherInfo* weatherInfo = nullptr;
  long dt = 0;
  int timezone = 0;
  if (httpCode == HTTP_CODE_OK) {
    Serial.printf("... got weather info from URL: %s\n", url.c_str());
    String payload = http.getString();
    //Serial.println(payload);
    JsonDocument doc;
    deserializeJson(doc, payload);
    String weatherMain = doc["weather"][0]["main"].as<String>();
    String weatherDescription = doc["weather"][0]["description"].as<String>();
    String weatherIconId = doc["weather"][0]["icon"].as<String>();
    float temp = doc["main"]["temp"].as<float>();
    float feelsLike = doc["main"]["feels_like"].as<float>();
    float tempMin = doc["main"]["temp_min"].as<float>();
    float tempMax = doc["main"]["temp_max"].as<float>();
    int pressure = doc["main"]["pressure"].as<int>();
    int humidity = doc["main"]["humidity"].as<int>();
    dt = doc["dt"].as<long>();
    timezone = doc["timezone"].as<int>();
    String name = doc["name"].as<String>();
    String country = doc["sys"]["country"].as<String>();
    Serial.printf("... weather main: '%s'\n", weatherMain.c_str());
    Serial.printf("... weather description: '%s'\n", weatherDescription.c_str());
    Serial.printf("... weather icon id: '%s'\n", weatherIconId.c_str());  // https://openweathermap.org/img/wn/10d@2x.png for 10d
    Serial.printf("... temp: %.2f\n", temp);
    Serial.printf("... feels like: %.2f\n", feelsLike);
    Serial.printf("... temp min: %.2f\n", tempMin);
    Serial.printf("... temp max: %.2f\n", tempMax);
    Serial.printf("... pressure: %d\n", pressure);
    Serial.printf("... humidity: %d\n", humidity);
    Serial.printf("... dt: %ld\n", dt);
    Serial.printf("... timezone: %d\n", timezone);
    Serial.printf("... name: '%s'\n", name.c_str());
    Serial.printf("... country: '%s'\n", country.c_str());
    weatherInfo = new WeatherInfo(
      weatherMain, weatherDescription,
      weatherIconId,
      temp, feelsLike, tempMin, tempMax,
      pressure, humidity,
      name, country);
  } else {
    Serial.printf("... HTTP failed code: %d\n", httpCode);
  }
  if (weatherInfo != nullptr && forecast) {
    String url = String(OPEN_WEATHER_API_FORECAST_END_POINT) + "?appid=" + OPEN_WEATHER_MAP_APP_ID;
    if (syncWeatherLocationWithGPS &&  knownLocation.version != -1) {
      url = url + "&lat=" + knownLocation.latitude + "&lon=" + knownLocation.longitude;
    } else {
      String q = String(DEF_OPEN_WEATHER_API_LOCATION);
      q.replace(" ", "%20");
      url = url + "&q=" + q;
    }
    Serial.printf("... fetching weather forecast info with url %s ...\n", url.c_str());
    http.begin(url); 
    httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
      Serial.printf("... got weather forecast info from URL: %s\n", url.c_str());
      String payload = http.getString();
      JsonDocument doc;
      deserializeJson(doc, payload);
      JsonDocument list = doc["list"];
      int listSize = list.size();
      for (int i = 0; i < listSize; i++) {
        float tempMin = list[i]["main"]["temp_min"].as<float>();
        float tempMax = list[i]["main"]["temp_max"].as<float>();
        // Serial.printf("... forecast ... temp min: %.2f\n", tempMin);
        // Serial.printf("... forecast ... temp max: %.2f\n", tempMax);
        bool updated = weatherInfo->updateTempMinMax(tempMin, tempMax);
        if (updated) {
          // Serial.printf("... forecast ... temp min: %.2f\n", tempMin);
          // Serial.printf("... forecast ... temp max: %.2f\n", tempMax);
        }
      }
    }  
  }
  if (weatherInfo != nullptr) {
    const String& gotWeatherIconId = weatherInfo->getWeatherIconId();
    WeatherIcon* weatherIcon = nullptr;
    if (currWeatherIcon == nullptr || gotWeatherIconId != currWeatherIcon->getIconId()) {  
      String weatherIconUrl = "http://openweathermap.org/img/wn/" + gotWeatherIconId + "@2x.png";
      Serial.printf("... get weather icon URL: %s\n", weatherIconUrl.c_str());
      HTTPClient http;
      //http.setTimeout(5000);
      http.begin(weatherIconUrl); 
      int httpCode = http.GET();
      if (httpCode == HTTP_CODE_OK) {
        Serial.printf("... get weather icon from URL: %s\n", weatherIconUrl.c_str());
        int len = http.getSize();
        weatherIcon = new WeatherIcon(gotWeatherIconId, len);
        uint8_t* buff = weatherIcon->getBuff();
        WiFiClient *stream = http.getStreamPtr();
        int readLen = 0;
        while (http.connected() && (len > 0 || len == -1)) {
          size_t size = stream->available();
          if (size) {
            int c = stream->readBytes(buff + readLen, (size > len) ? len : size);
            readLen += c;
            Serial.printf("... ... just read %d bytes ... so far %d of %d\n", c, readLen, len);
            if (readLen >= len) {
              break;
            }
          }
          delay(1);
        }
        //showWeatherIcon(buff, len);   
      } else {
        Serial.printf("... get weather icon HTTP failed code: %d, ... error: %s\n", httpCode, http.errorToString(httpCode).c_str());
      }
    }
    return FetchedWeather{weatherInfo, weatherIcon, dt, timezone};
    //updateWeather(weatherInfo, weatherIcon);
  }
  return FetchedWeather{0};
}
