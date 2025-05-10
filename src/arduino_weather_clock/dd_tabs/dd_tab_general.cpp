#include <Arduino.h>
#include "dd_tab_helpers.h"

#include "../global.h"


namespace {

  static int SlideShowIdleChoices[] { 
    0, 1, 2, 5, 10, 15
  };
  #define SSI_SELECTION_HORI_SELECTION_COUNT 3
  #define SSI_SELECTION_VERT_SELECTION_COUNT 2
  #define SSI_SELECTION_HORI_IDX(i) (i % SSI_SELECTION_HORI_SELECTION_COUNT)
  #define SSI_SELECTION_VERT_IDX(i) (i / SSI_SELECTION_HORI_SELECTION_COUNT)
  static int SlideShowIdleChoicesCount = sizeof(SlideShowIdleChoices) / sizeof(SlideShowIdleChoices[0]);

  static int SlideDurationChoices[] { 
    5, 10, 15, 20, 30, 60
  };
  #define SD_SELECTION_HORI_SELECTION_COUNT 3
  #define SD_SELECTION_VERT_SELECTION_COUNT 2
  #define SD_SELECTION_HORI_IDX(i) (i % SD_SELECTION_HORI_SELECTION_COUNT)
  #define SD_SELECTION_VERT_IDX(i) (i / SD_SELECTION_HORI_SELECTION_COUNT)
  static int SlideDurationChoicesCount = sizeof(SlideDurationChoices) / sizeof(SlideDurationChoices[0]);

  static int UpdateWeatherIntervalChoices[] { 
    30, 60, 90
  };
  #define UWI_SELECTION_HORI_SELECTION_COUNT 3
  #define UWI_SELECTION_VERT_SELECTION_COUNT 1
  #define UWI_SELECTION_HORI_IDX(i) (i % UWI_SELECTION_HORI_SELECTION_COUNT)
  #define UWI_SELECTION_VERT_IDX(i) (i / UWI_SELECTION_HORI_SELECTION_COUNT)
  static int UpdateWeatherIntervalChoicesCount = sizeof(UpdateWeatherIntervalChoices) / sizeof(UpdateWeatherIntervalChoices[0]);


  SelectionDDLayer* forceRefreshWeatherButton;
  SelectionDDLayer* syncWeatherLocationWithGPSSelection;
  SelectionDDLayer* timeFormatSelection;

  SelectionDDLayer* slideShowIdleSelection;
  SelectionDDLayer* slideDurationSelection;
  SelectionDDLayer* updateWeatherIntervalSelection;

  String autoPinConfig;


  int getCurrentSlideShowIdleSelectionIdx() {
    // assume the first choice is 0
    if (slideShowIdleDelayMins > 0) {
      for (int i = 1; i < SlideShowIdleChoicesCount; i++) {
        if (slideShowIdleDelayMins <= SlideShowIdleChoices[i]) {
          return i;
        }
      }
    }
    return 0;  
  }

  int getCurrentSlideDurationSelectionIdx() {
    for (int i = 0; i < SlideDurationChoicesCount; i++) {
      if (slideDurationSecs <= SlideDurationChoices[i]) {
        return i;
      }
    }
    return 0;  
  }

  int getCurrentUpdateWeatherIntervalSelectionIdx() {
    for (int i = 0; i < UpdateWeatherIntervalChoicesCount; i++) {
      if (updateWeatherIntervalMins <= UpdateWeatherIntervalChoices[i]) {
        return i;
      }
    }
    return 0;  
  }

  void syncSyncWeatherLocationWithGPSSelection() {
    if (syncWeatherLocationWithGPS) {
      syncWeatherLocationWithGPSSelection->select();
    } else {
      syncWeatherLocationWithGPSSelection->deselect();
    }
  }
  void syncTimeFormatSelection() {
    timeFormatSelection->select(internationalTimeFormat ? 1 : 0, 0);
  }
  void syncSlideShowIdleSelection() {
    int idx = getCurrentSlideShowIdleSelectionIdx();
    slideShowIdleSelection->select(idx % SSI_SELECTION_HORI_SELECTION_COUNT, idx / SSI_SELECTION_HORI_SELECTION_COUNT);
  }
  void syncSlideDurationSelection() {
    int idx = getCurrentSlideDurationSelectionIdx();
    slideDurationSelection->select(idx % SD_SELECTION_HORI_SELECTION_COUNT, idx / SD_SELECTION_HORI_SELECTION_COUNT);
  }
  void syncUpdateWeatherIntervalSelection() {
    int idx = getCurrentUpdateWeatherIntervalSelectionIdx();
    updateWeatherIntervalSelection->select(idx % UWI_SELECTION_HORI_SELECTION_COUNT, idx / UWI_SELECTION_HORI_SELECTION_COUNT);
  }

}

void dd_general_setup(bool recreateLayers) {
  if (recreateLayers) {
    forceRefreshWeatherButton = dumbdisplay.createSelectionLayer(4, 1);
    forceRefreshWeatherButton->selected(true);
    forceRefreshWeatherButton->highlightBorder(true, "darkblue");
    if (false) {
      forceRefreshWeatherButton->textCentered("🌤️");    
    } else {
      // animate text on forceRefreshWeatherButton
      for (int i = 0; i < 6; i++) {
        String text;
        switch (i) {
          case 0: text = "☀️"; break;
          case 1: text = "🌤️"; break;
          case 2: text = "☁️"; break;
          case 3: text = "🌧️"; break;
          case 4: text = "🌦️"; break;
          case 5: text = "🌤️"; break;
        }
        forceRefreshWeatherButton->textCentered(text);    
        forceRefreshWeatherButton->exportAsBackgroundImage(false);
      }
      forceRefreshWeatherButton->clear();
      forceRefreshWeatherButton->animateBackgroundImage(2);
    }

    syncWeatherLocationWithGPSSelection = dumbdisplay.createSelectionLayer(4, 1);
    syncWeatherLocationWithGPSSelection->textCentered("📡");    

    timeFormatSelection = dumbdisplay.createSelectionLayer(7, 1, 2, 1);
    timeFormatSelection->textCentered("12 Hour", 0, 0);
    timeFormatSelection->textCentered("24 Hour", 0, 1);

    LcdDDLayer slideShowIdleSelectionLabel(dumbdisplay.createLcdLayerHandle(20, 1));
    slideShowIdleSelectionLabel.margin(0, 5, 0, 0);
    slideShowIdleSelectionLabel.writeLine("Slide Show Idle:");
    slideShowIdleSelectionLabel.noBackgroundColor();
    slideShowIdleSelectionLabel.pixelColor("darkblue");
    slideShowIdleSelection = dumbdisplay.createSelectionLayer(7, 1, SSI_SELECTION_HORI_SELECTION_COUNT, SSI_SELECTION_VERT_SELECTION_COUNT);
    for (int i = 0; i < SlideShowIdleChoicesCount; i++) {
      int mins = SlideShowIdleChoices[i];
      String text = mins == 0 ? String("🚫") : String(mins) + " mins";
      slideShowIdleSelection->textCentered(text, 0, SSI_SELECTION_HORI_IDX(i), SSI_SELECTION_VERT_IDX(i));
    }

    LcdDDLayer slideDurationSelectionLabel(dumbdisplay.createLcdLayerHandle(20, 1));
    slideDurationSelectionLabel.margin(0, 5, 0, 0);
    slideDurationSelectionLabel.writeLine("Slide Duration:");
    slideDurationSelectionLabel.noBackgroundColor();
    slideDurationSelectionLabel.pixelColor("darkblue");
    slideDurationSelection = dumbdisplay.createSelectionLayer(7, 1, SD_SELECTION_HORI_SELECTION_COUNT, SD_SELECTION_VERT_SELECTION_COUNT);
    for (int i = 0; i < SlideDurationChoicesCount; i++) {
      int secs = SlideDurationChoices[i];
      String text = String(secs) + " secs";
      slideDurationSelection->textCentered(text, 0, SD_SELECTION_HORI_IDX(i), SD_SELECTION_VERT_IDX(i));
    }

    LcdDDLayer updateWeatherIntervalSelectionLabel(dumbdisplay.createLcdLayerHandle(20, 1));
    updateWeatherIntervalSelectionLabel.margin(0, 5, 0, 0);
    updateWeatherIntervalSelectionLabel.writeLine("Update Weather:");
    updateWeatherIntervalSelectionLabel.noBackgroundColor();
    updateWeatherIntervalSelectionLabel.pixelColor("darkblue");
    updateWeatherIntervalSelection = dumbdisplay.createSelectionLayer(7, 1, UWI_SELECTION_HORI_SELECTION_COUNT, UWI_SELECTION_VERT_SELECTION_COUNT);
    for (int i = 0; i < UpdateWeatherIntervalChoicesCount; i++) {
      int mins = UpdateWeatherIntervalChoices[i];
      String text = String(mins) + " mins";
      updateWeatherIntervalSelection->textCentered(text, 0, UWI_SELECTION_HORI_IDX(i), UWI_SELECTION_VERT_IDX(i));
    }

    autoPinConfig = DDAutoPinConfig('V')
      .beginGroup('H')
      .addLayer(forceRefreshWeatherButton)
      .addLayer(timeFormatSelection)
        .addLayer(syncWeatherLocationWithGPSSelection)
      .endGroup()
      .addPaddedLayer(slideShowIdleSelectionLabel, 0, 0, 50, 0)
      .addLayer(slideShowIdleSelection)
      .addPaddedLayer(slideDurationSelectionLabel, 0, 0, 50, 0)
      .addLayer(slideDurationSelection)
      .addPaddedLayer(updateWeatherIntervalSelectionLabel, 0, 0, 50, 0)
      .addLayer(updateWeatherIntervalSelection)
      .build();
  }
  
  dumbdisplay.pinAutoPinLayers(autoPinConfig, PF_TAB_LEFT, PF_TAB_TOP, PF_TAB_WIDTH, PF_TAB_HEIGHT, "T");
  syncSyncWeatherLocationWithGPSSelection();   
  syncTimeFormatSelection();   
  syncSlideShowIdleSelection();
  syncSlideDurationSelection();
  syncUpdateWeatherIntervalSelection();
}


bool dd_general_loop() {
  const DDFeedback* feedback;
  bool redrawScreen = false;
 
  if (forceRefreshWeatherButton->getFeedback() != nullptr) {
    forceRefreshWeather = true;
  }

  if (syncWeatherLocationWithGPSSelection->getFeedback() != nullptr) {
    syncWeatherLocationWithGPS = !syncWeatherLocationWithGPS;
    onGlobalSettingsChanged("syncWeatherLocationWithGPS");
    syncSyncWeatherLocationWithGPSSelection();
  }
  
  feedback = timeFormatSelection->getFeedback();
  if (feedback != nullptr) {
    int x = feedback->x;
    if (x == 0) {
      internationalTimeFormat = false;
    } else {
      internationalTimeFormat = true;
    }
    onGlobalSettingsChanged("internationalTimeFormat");
    syncTimeFormatSelection();
    redrawScreen = true;
  }

  feedback = slideShowIdleSelection->getFeedback();
  if (feedback != nullptr) {
    int x = feedback->x;
    int y = feedback->y;
    int idx = y * SSI_SELECTION_HORI_SELECTION_COUNT + x;
    slideShowIdleDelayMins = SlideShowIdleChoices[idx];
    onGlobalSettingsChanged("slideShowIdleDelayMins");
    syncSlideShowIdleSelection();
  }

  feedback = slideDurationSelection->getFeedback();
  if (feedback != nullptr) {
    int x = feedback->x;
    int y = feedback->y;
    int idx = y * SD_SELECTION_HORI_SELECTION_COUNT + x;
    slideDurationSecs = SlideDurationChoices[idx];
    onGlobalSettingsChanged("slideDurationSecs");
    syncSlideDurationSelection();
  }

  feedback = updateWeatherIntervalSelection->getFeedback();
  if (feedback != nullptr) {
    int x = feedback->x;
    int y = feedback->y;
    int idx = y * UWI_SELECTION_HORI_SELECTION_COUNT + x;
    updateWeatherIntervalMins = UpdateWeatherIntervalChoices[idx];
    onGlobalSettingsChanged("updateWeatherIntervalMins");
    syncUpdateWeatherIntervalSelection();
  }

  return redrawScreen;
}

void dd_general_done() {
  dumbdisplay.logToSerial("=== dd_general_done ===");
  dumbdisplay.logToSerial(". syncWeatherLocationWithGPS=" + String(syncWeatherLocationWithGPS));
  dumbdisplay.logToSerial(". internationalTimeFormat=" + String(internationalTimeFormat));
  dumbdisplay.logToSerial(". slideShowIdleDelayMins=" + String(slideShowIdleDelayMins));
  dumbdisplay.logToSerial(". slideDurationSecs=" + String(slideDurationSecs));
  dumbdisplay.logToSerial(". updateWeatherIntervalMins=" + String(updateWeatherIntervalMins));
}

