#ifndef CONFIG_H
#define CONFIG_H


// #####
// # you will need to either
// # . enable and complete the following "secret" block
// # . or create and complete the "secret" block in the file `_secret.h`
// #####
#if false

  // ----------------------
  // !!! "secret" block !!!
  // ----------------------
  //
  // *****
  // * you can setup WIFI_SSID / WIFI_PASSWORD; for ESP32, if WIFI_SSID not defined, will use WiFiManager to get WiFi SSID and password
  // * you will need to setup OPEN_WEATHER_MAP_APP_ID
  // * you can optionally setup UNSPLASH_CLIENT_ID
  // *****

  #define WIFI_SSID               "<wifi ssid>"
  #define WIFI_PASSWORD           "<wifi password>"

  // you MUST get APP_ID from https://home.openweathermap.org/users/sign_up
  #define OPEN_WEATHER_MAP_APP_ID "<app id>"

  // optionally, sign up and create an app to get Access Key from https://unsplash.com/developers
  // comment out UNSPLASH_CLIENT_ID if you do not want to use unsplash.com
  #define UNSPLASH_CLIENT_ID      "<client id>"

#else
  #include "_secret.h"
#endif  


#include <Arduino.h>
#include "sys_config.h"

// TIMEZONE (in hours); note that NTP timezone will be gotten from weather api, hence, this is just the initial TIMEZONE
#define INIT_TIMEZONE                       8

// In order to properly setup the openweathermap.org the endpoint
// * please head to http://api.openweathermap.org to create an account and get an APP ID 
// * the country (location) for which to retrieve whether is defined with OPEN_WEATHER_API_LOCATION
//   - please refer to https://openweathermap.org/api/geocoding-api
//   - Please use ISO 3166 country codes -- https://en.wikipedia.org/wiki/List_of_ISO_3166_country_codes
// * below DEF_SYNC_WEATHER_LOCATION_WITH_GPS ==> got location from GPS when connected to DD
#define DEF_OPEN_WEATHER_API_LOCATION       "Hong Kong"

#define DEF_SYNC_WEATHER_LOCATION_WITH_GPS  true /* got location from GPS when connected to DD */
#define DEF_SLIDE_SHOW_IDLE_DELAY_MINS      2    /* <= 0 means slide show not enabled */
#define DEF_SLIDE_DURATION_SECS             5
#define DEF_UPDATE_WEATHER_INT_MINS         30

#define NUM_ALARMS                          5
#define AUTO_ACK_ALARM_MINUTES              10

#define AUTOCONNECT_AP_NAME                 "AWClock"
#define USE_NTP                             true
#define MAX_FEELS_LIKE_CELSIUS              50
#define FORECAST_WEATHER                    true

#define DD_GET_GPS_INTERVAL_S               10
#define DD_GPS_LOC_CHANGE_KM                2


#endif

