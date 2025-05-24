#include "global.h"
#include "eeprom_helpers.h"
//#include "search.h"


bool systemStartupSettingsInvalid = true;  // will be set properly in eeprom_initialization()

int timeZoneInSecs = INIT_TIMEZONE * 60 * 60;

bool syncWeatherLocationWithGPS = DEF_SYNC_WEATHER_LOCATION_WITH_GPS;

int slideShowIdleDelayMins = DEF_SLIDE_SHOW_IDLE_DELAY_MINS;
int slideDurationSecs = DEF_SLIDE_DURATION_SECS;
int updateWeatherIntervalMins = DEF_UPDATE_WEATHER_INTERVAL_MINS;

bool internationalTimeFormat = false;
bool showTimeOnSlide = true;

bool ddExclusiveMode = false;
bool forceRefreshWeather = false;

WeatherIcon* currWeatherIcon = nullptr;
WeatherInfo* currWeatherInfo = nullptr;

KnowLocation knownLocation;

#if defined(ES8311_VOLUME)
  int audioVolume = ES8311_VOLUME;
#else
  int audioVolume = -1;
#endif


void onGlobalSettingsChanged(const char* reason) {
  eeprompt_saveGlobalSettings(reason);
}

