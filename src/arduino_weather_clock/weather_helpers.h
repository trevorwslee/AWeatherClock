#ifndef WEATHER_HELPERS_H
#define WEATHER_HELPERS_H

#include <Arduino.h>

#include "config.h"

struct FetchedWeather {
  WeatherInfo* weatherInfo;
  WeatherIcon* weatherIcon;
  long dt;
  int timezone;
};

FetchedWeather fetchWeather(const WeatherIcon* currWeatherIcon, bool forecast);

#endif