#include <Arduino.h>
#include "dd_tab_helpers.h"

LedGridDDLayer* led;
DDTimedChangeStateHelper blinkStateChangeHelper;

void dd_blink_setup(bool recreateLayers) {
  if (recreateLayers) {
    led = dumbdisplay.createLedGridLayer();
    led->enableFeedback("fl");
  }

  dumbdisplay.pinLayer(led, PF_TAB_LEFT, PF_TAB_TOP, PF_TAB_WIDTH, PF_TAB_HEIGHT);

  blinkStateChangeHelper.initialize(0);
}

bool dd_blink_loop() {
  blinkStateChangeHelper.checkStatChange([](int currentState) {
    led->toggle();
    return DDChangeStateInfo{ currentState, 1000 };
  });
  return false;
}

