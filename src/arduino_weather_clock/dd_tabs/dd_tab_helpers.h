#ifndef DD_TAB_HELPERS_H
#define DD_TAB_HELPERS_H

#include "../sys_config.h"
#include "dumbdisplay.h"

#define PF_TAB_LEFT  (0 + 1) 
#define PF_TAB_TOP   (10 + 3)

#if defined(USE_TASK_FOR_ALARM_SOUND)
  #define PF_TAB_WIDTH (100 - 2 * 1)
  #define PF_TAB_HEIGHT (120 - 2 * 3)
#else
  #define PF_TAB_WIDTH (100 - 2 * 1)
  #define PF_TAB_HEIGHT (90 - 2 * 3)
#endif


extern DumbDisplay dumbdisplay;


void dd_general_setup(bool recreateLayers);
bool dd_general_loop();
void dd_general_done();

void dd_slides_setup(bool recreateLayers);
bool dd_slides_loop();
void dd_slides_done();


void dd_alarms_setup(bool recreateLayers);
bool dd_alarms_loop();
void dd_alarms_done();


void dd_blink_setup(bool recreateLayers);
bool dd_blink_loop();

#endif