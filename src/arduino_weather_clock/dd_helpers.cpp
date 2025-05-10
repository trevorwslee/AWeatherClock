#include <Arduino.h>

#include "global.h"
#include "config.h"
#include "dd_helpers.h"
#include "dd_tabs/dd_tab_helpers.h"

#include "wifidumbdisplay.h"
DumbDisplay dumbdisplay(new DDWiFiServerIO());
DDMasterResetPassiveConnectionHelper pdd(dumbdisplay, true);



#define LOG_DD_GPS_LOCATION false


struct Tab {
  String name;
  bool createdLayers;
  void (*setup)(bool recreateLayers);
  bool (*loop)();
  void (*done)();
};


#if true
static Tab Tabs[] = {
  { "General", false, dd_general_setup, dd_general_loop, dd_general_done },
  { "Alarms", false, dd_alarms_setup, dd_alarms_loop, dd_alarms_done },
  { "Slides", false, dd_slides_setup, dd_slides_loop, dd_slides_done }
};
#else
static Tab Tabs[] = {
  { "General", false, dd_general_setup, dd_general_loop, dd_general_done },
  { "Alarms", false, dd_alarms_setup, dd_alarms_loop, dd_alarms_done },
  { "Slides", false, dd_slides_setup, dd_slides_loop, dd_slides_done },
  { "Blink", false, dd_blink_setup, dd_blink_loop, nullptr }
};
#endif
static int const NumTabs = sizeof(Tabs) / sizeof(Tabs[0]);
int currentTabIdx;

bool forceRedrawScreen;

GraphicalDDLayer* root;
SelectionDDLayer* tabSelection;
GpsServiceDDTunnel* gpsTunnel;

DDTimedChangeStateHelper timedChangeStatHelper;

DDLocation lastKnownLocation;
int lastKnownLocationVersion = -1;

void selectCurrTab(int tabIdx) {
  if (currentTabIdx == tabIdx) {
    return;
  }

  if (currentTabIdx != -1) {
    if (Tabs[currentTabIdx].done != nullptr) {
      Tabs[currentTabIdx].done();
    }
  }

  currentTabIdx = tabIdx;  // tabIdx == -1 only if disconnectedDD()

  if (currentTabIdx != -1) {
    tabSelection->select(currentTabIdx, 0);
    tabSelection->flashArea(currentTabIdx, 0);
    Tab& currTab = Tabs[currentTabIdx];

    dumbdisplay.resetPinLayers();
    dumbdisplay.pinLayer(tabSelection, 0, 0, 100, 10, "LB");
    
    currTab.setup(!currTab.createdLayers);
    currTab.createdLayers = true;
  }
}

void initializeDD() {
  dumbdisplay.configPinFrame(100, 120, true);

  root = dumbdisplay.setRootLayer(100, 120, "T");
  root->backgroundColor("beige");
  root->drawImageFileFit("dumbdisplay.png");
  root->opacity(7);
  root->padding(1);
  root->border(1, "blue", "round", 0.5);
  root->addLevel("M", 100, 20, true);
  root->setLevelAnchor(0, 100);

  tabSelection = dumbdisplay.createSelectionLayer(10, 1, NumTabs, 1);
  tabSelection->backgroundColor("blue");
  tabSelection->highlightBorder(true, "purple");
  tabSelection->pixelColor("cyan", true);
  tabSelection->pixelColor("yellow", false);
  for (int i = 0; i < NumTabs; i++) {
    Tab& tab = Tabs[i];
    tabSelection->textCentered(tab.name, 0, i, 0);
    tab.createdLayers = false;
  }
 
  gpsTunnel = nullptr;
  currentTabIdx = -1;
  selectCurrTab(0);
  forceRedrawScreen = false;

  timedChangeStatHelper.initialize(0);
}

void updateDD(bool isFirstUpdate) {
  // A friendly reminder!
  // * Jesus Saves!   *
  // * Amazing Grace! * 
  // You can ignore the messages and comment out the following lines without breaking the code.
  // However, before you ignore the messages, think about where you will be in 100 years.
  // Jesus saves you from the wrath of God and gives you eternal life.
  // Or else, you will be condemned to hell!
  timedChangeStatHelper.checkStatChange([](int currentState) {
    int nextState;
    root->clear();
    if (currentState == 0) {
      root->drawStr(5, 0, "Jesus Saves!", "purple", "", 12);
      nextState = 1;
    } else {
      root->drawStr(0, 0, "Amazing Grace!", "purple", "", 12);
      nextState = 0;
    }
    return DDChangeStateInfo{ nextState, 2000 };
  });

  if (Tabs[currentTabIdx].loop()) {
    forceRedrawScreen = true;
  }

  const DDFeedback* feedback = tabSelection->getFeedback();
  if (feedback != nullptr) {
    selectCurrTab(feedback->x);
  }

  if (syncWeatherLocationWithGPS) {
    if (gpsTunnel == nullptr) {
      gpsTunnel = dumbdisplay.createGpsServiceTunnel();
      gpsTunnel->reconnectForLocation(DD_GET_GPS_INTERVAL_S);
      dumbdisplay.logToSerial("* created gpsTunnel");
    }
  } else {
    if (gpsTunnel != nullptr) {
      dumbdisplay.deleteTunnel(gpsTunnel);
      gpsTunnel = nullptr;
      lastKnownLocationVersion = -999;
      dumbdisplay.logToSerial("* deleted gpsTunnel");
    }
  }

  if (gpsTunnel != nullptr) {
    DDLocation location;
    if (gpsTunnel->readLocation(location)) {
      // got GPS location feedback
      if (LOG_DD_GPS_LOCATION) {
        dumbdisplay.logToSerial("* got location ...");
        dumbdisplay.logToSerial("  - latitude = " + String(location.latitude, 4));
        dumbdisplay.logToSerial("  - longitude = " + String(location.longitude, 4));
      }
      bool updateLastKnownLocation = false;
      if (lastKnownLocationVersion >= 0/*lastKnownLocationVersion != -1*/) {
        float R = 6371; // Radius of the earth in km
        float distance;
        if (true) {
          // Haversine formula
          float lat1 = lastKnownLocation.latitude * PI / 180.0;
          float lat2 = location.latitude * PI / 180.0;
          float lon1 = lastKnownLocation.longitude * PI / 180.0;
          float lon2 = location.longitude * PI / 180.0;
          float dela_lat = lat1 - lat2;
          float dela_lon = lon1 - lon2;
          float a = pow(sin(dela_lat / 2), 2) + cos(lat1) * cos(lat2) * pow(sin(dela_lon / 2), 2);
          float c = 2 * atan2(sqrt(a), sqrt(1 - a));
          distance = R * c;
        } else {
          distance = acos(sin(lastKnownLocation.latitude) * sin(location.latitude) + cos(lastKnownLocation.latitude) * cos(location.latitude) * cos(location.longitude - lastKnownLocation.longitude)) * R;
        }
        if (LOG_DD_GPS_LOCATION) {
          dumbdisplay.logToSerial("  - distance from last known = " + String(distance, 2) + " km");
        }
        if (distance >= DD_GPS_LOC_CHANGE_KM) {
          // keep it small, since the sync GPS location will only be done when connected to DD
          updateLastKnownLocation = true;
        }
      } else {
        updateLastKnownLocation = true;
      }
      if (updateLastKnownLocation) {
        lastKnownLocation = location;
        if (true) {
          lastKnownLocationVersion = knownLocation.version + 1;
        } else { 
          lastKnownLocationVersion += 1;
        }
        if (true) {
          dumbdisplay.log("* updated location ...");
          dumbdisplay.log("  - latitude = " + String(lastKnownLocation.latitude, 4));
          dumbdisplay.log("  - longitude = " + String(lastKnownLocation.longitude, 4));
        }
      }
    }
  }
}
void disconnectedDD() {
  selectCurrTab(-1);
}



bool checkDDRenderedRedrawScreen() {
  if (forceRedrawScreen) {
    forceRedrawScreen = false;
    return true;
  }
  return false;
}
bool updateKnownLocationFromDD() {
  if (lastKnownLocationVersion == -999 && knownLocation.version != -1) {  // reset last know location
    knownLocation.latitude = 0;
    knownLocation.longitude = 0;
    knownLocation.version = -1;
    onGlobalSettingsChanged("RESET knownLocation");
    return true;
  } else if (lastKnownLocationVersion >= 0 && knownLocation.version != lastKnownLocationVersion) {
    knownLocation.latitude = lastKnownLocation.latitude;
    knownLocation.longitude = lastKnownLocation.longitude;
    knownLocation.version = lastKnownLocationVersion;
    onGlobalSettingsChanged("knownLocation");
    return true;
  }
  return false;
}

void ddSetup() {
}

bool ddLoop() {
 pdd.loop([](){
    // **********
    // *** initializeCallback ***
    // **********
    initializeDD();
  }, [](){
    // **********
    // *** updateCallback ***
    // **********
    updateDD(!pdd.firstUpdated());
  }, [](){
    // **********
    // *** disconnectedCallback ***
    // **********
    disconnectedDD();
  });
  return pdd.isIdle();
}
